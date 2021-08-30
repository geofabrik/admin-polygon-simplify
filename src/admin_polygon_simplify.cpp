/*
  admin_polygon_simplify.cpp

   Created on:  2017-02-07
       Author: Michael Reichert

  Copyright 2012-2016 Jochen Topf <jochen@topf.org>.

  This file contains parts of OSMCoastline.

  OSMCoastline – and therefore this software – is free software: you
  can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OSMCoastline is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OSMCoastline.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <getopt.h>
#include <unordered_map>
#include <osmium/io/reader.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/osm/entity_bits.hpp>
#include <osmium/util/file.hpp>
#include <osmium/util/progress_bar.hpp>
#include "way_simplify_handler2.hpp"
#include "way_simplify_handler.hpp"
#include "boundary_segment.hpp"
#include "no_simplify_segment.hpp"
#include "intermediate_simplifier.hpp"
#include "boundary_relation_collector.hpp"

void print_help() {
    std::cerr << "Missing arguments, correct usage:\n" \
              << "admin_polygon_simplify [OPTIONS] INFILE OUTFILE\n" \
              << "Options:\n" \
              << "-e E, --epsilon=E    set maximum error to E (default: 75 m)\n" \
              << "-i I, --iterations=I set maximum of iterations to I (default: 6)\n" \
              << "-h, --help           show help, i.e. this message\n" \
              << "-v, --verbose        verbose output\n";
}

int main(int argc, char* argv[]) {
    static struct option long_options[] = {
        {"epsilon", required_argument, 0, 'e'},
        {"help", no_argument, 0, 'h'},
        {"iterations", required_argument, 0, 'i'},
        {"verbose", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };
    double max_error = 75;
    int iterations = 7;
    bool verbose = false;
    std::string input_filename;
    std::string output_filename;
    while (true) {
        int c = getopt_long(argc, argv, "e:hi:v", long_options, 0);
        if (c == -1) {
            break;
        }

        switch (c) {
        case 'e':
            max_error = std::atof(optarg);
            break;
        case 'h':
            print_help();
            exit(1);
        case 'i':
            iterations = std::atoi(optarg) + 1;
            break;
        case 'v':
            verbose = true;
            break;
        default:
            exit(1);
        }
    }
    int remaining_args = argc - optind;
    if (remaining_args != 2) {
        print_help();
        exit(1);
    } else if (max_error < 0) {
        std::cerr << "ERROR: maximum error must be a positive number.\n";
        print_help();
    } else if (iterations < 0) {
        std::cerr << "ERROR: The number of iterations must be a positive number.\n";
        print_help();
    }
    input_filename =  argv[optind];
    output_filename = argv[optind+1];

    if (argc != 3) {

    }
    osmium::io::File input_file(input_filename);
    osmium::util::VerboseOutput vout(verbose);

    std::unordered_set<osmium::object_id_type> treat_as_rings_way;
    {
        vout << "Pass 1 – read boundary relations\n";
        osmium::io::Reader reader{input_file, osmium::osm_entity_bits::relation};
        BoundaryRelationCollector br_collector(treat_as_rings_way);
        osmium::ProgressBar progress_bar{reader.file_size(), osmium::util::isatty(2)};
        while (osmium::memory::Buffer buffer = reader.read()) {
            progress_bar.update(reader.offset());
            br_collector.read_relations(buffer.begin(), buffer.end());
        }

        vout << "Pass 2 – read members of boundary relations\n";
        osmium::io::Reader reader2{input_file, osmium::osm_entity_bits::way | osmium::osm_entity_bits::relation};
        progress_bar.done();
        while (osmium::memory::Buffer buffer = reader2.read()) {
            progress_bar.update(reader2.offset());
            osmium::apply(buffer, br_collector.handler());
        }
    }

    ErrorsMap errors;
    KeepNodesMap nodes_to_be_kept;
    {
        std::vector<BoundarySegment> segments;
        vout << "Pass 3 – read ways\n";
        osmium::io::Reader reader{input_file};
        osmium::ProgressBar progress_bar{reader.file_size(), osmium::util::isatty(2)};
        WaySimplifyHandler simplify_handler {max_error, segments, treat_as_rings_way};
        while (osmium::memory::Buffer buffer = reader.read()) {
            progress_bar.update(reader.offset());
            osmium::apply(buffer, simplify_handler);
        }
        reader.close();
        progress_bar.done();

        IntermediateSimplifier interm_simplifier (max_error, errors, segments, nodes_to_be_kept, vout);
        vout << "Trying to eliminate intersections ...\n";
        int counter = 1;
        if (interm_simplifier.recheck_intersections()) {
            do {
                vout << "Trying to avoid intersections of the simplified geometry, iteration " << counter << "\n";
                osmium::io::Reader reader_i{input_file, osmium::osm_entity_bits::way};
                osmium::ProgressBar progress_bar_i{reader_i.file_size(), osmium::util::isatty(2)};
                while (osmium::memory::Buffer buffer = reader_i.read()) {
                    progress_bar_i.update(reader.offset());
                    osmium::apply(buffer, interm_simplifier);
                }
                reader_i.close();
                progress_bar_i.done();
                ++counter;
            } while (interm_simplifier.recheck_intersections() && counter < iterations);
        }
    }


    vout << "Last pass\n";
    osmium::io::Reader last_reader{input_file, osmium::osm_entity_bits::way | osmium::osm_entity_bits::relation};
    osmium::ProgressBar progress_bar2{last_reader.file_size(), osmium::util::isatty(2)};

    osmium::io::Header header;
    header.set("generator", "admin_polygon_simplify");
    header.set("copyright", "OpenStreetMap and contributors");
    header.set("attribution", "http://www.openstreetmap.org/copyright");
    header.set("license", "http://opendatacommons.org/licenses/odbl/1-0/");

    osmium::io::File output_file {output_filename};
    output_file.set("locations_on_ways", true);
    WaySimplifyHandler2 simplify_handler2 {output_file, max_error, header, errors, nodes_to_be_kept, treat_as_rings_way};
    while (osmium::memory::Buffer buffer = last_reader.read()) {
        progress_bar2.update(last_reader.offset());
        osmium::apply(buffer, simplify_handler2);
    }
    last_reader.close();
    progress_bar2.done();
}
