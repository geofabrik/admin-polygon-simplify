/*
 * osm_adminfilter.cpp
 *
 *  Created on:  2017-02-07
 *      Author: Michael Reichert
 */

#include <getopt.h>
#include <osmium/io/reader.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/util/progress_bar.hpp>
#include "admin_rel_handlers.hpp"
#include "way_admin_level_index.hpp"

void print_help(char* argv[]) {
    std::cerr <<
            "Transfer admin_level=* values to member ways of admin relations. All member\n" \
            "ways get the admin_level of the relation with the lowest \n" \
            "Usage: " << argv[0] << " [ARGS] INPUT_FILE OUTPUT_FILE\n" \
            "Arguments:\n" \
            "  -M NUM, --max-level=NUM  Process levels 2 to N only (default 11). Ways with\n" \
            "                           higher levels will not get modified if they are not\n" \
            "                           used by a relation of interest.\n" \
            "  -v, --verbose            Enable verbose mode (show progress bar)\n";
    exit(1);
}

int main(int argc, char* argv[]) {
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"max-level", required_argument, 0, 'M'},
        {"verbose", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };
    int max_level = 11;
    bool verbose = false;
    std::string input_filename;
    std::string output_filename;
    while (true) {
        int c = getopt_long(argc, argv, "hM:v", long_options, 0);
        if (c == -1) {
            break;
        }

        char** ptr = nullptr;
        switch (c) {
        case 'h':
            print_help(argv);
            break;
        case 'M':
            max_level = static_cast<int>(strtol(optarg, ptr, 10));
            if (ptr || max_level < 2 || max_level > 11) {
                std::cerr << "ERROR: Invalid argument for option --max-level\n";
                exit(1);
            }
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
        std::cerr << "Error: Too few or too much arguments.\n";
        print_help(argv);
    }
    input_filename =  argv[optind];
    output_filename = argv[optind + 1];

    WayAdminLevelIndex way_level_idx;
    osmium::io::File input_file(input_filename);
    std::cerr << "Reading relations\n";
    {
        AdminRelHandler1 handler1 {way_level_idx, max_level};
        osmium::io::Reader reader1{input_file, osmium::osm_entity_bits::relation};
        osmium::ProgressBar progress_bar{reader1.file_size(), osmium::util::isatty(2) && verbose};
        while (osmium::memory::Buffer buffer = reader1.read()) {
            progress_bar.update(reader1.offset());
            osmium::apply(buffer, handler1);
        }
        reader1.close();
    }
    way_level_idx.prepare_for_query();
    osmium::io::Header header;
    header.set("generator", "osm_admin_level_rels2ways");
    header.set("copyright", "OpenStreetMap and contributors");
    header.set("attribution", "http://www.openstreetmap.org/copyright");
    header.set("license", "http://opendatacommons.org/licenses/odbl/1-0/");
    osmium::io::File output_file {output_filename};
    std::cerr << "Writing to output file\n";
    AdminRelHandler2 handler2 {way_level_idx, output_file, header, max_level};
    {
        osmium::io::Reader reader2(input_file);
        osmium::ProgressBar progress_bar{reader2.file_size(), osmium::util::isatty(2) && verbose};
        while (osmium::memory::Buffer buffer = reader2.read()) {
            progress_bar.update(reader2.offset());
            osmium::apply(buffer, handler2);
        }
        reader2.close();
        progress_bar.done();
        if (verbose) {
            std::cerr << way_level_idx.size() << " ways are used by admin boundary relations.\n";
        }
    }
}
