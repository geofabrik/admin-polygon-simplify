/*
 * osm_admin_level_relways_export.cpp
 *
 *  Created on:  2017-02-07
 *      Author: Michael Reichert
 */

#include <getopt.h>
#include <osmium/index/map/sparse_mmap_array.hpp>
#include <osmium/index/map/dense_mmap_array.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>
#include <osmium/io/reader.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/util/progress_bar.hpp>
#include "admin_rel_handlers.hpp"
#include "admin_shp_handler.hpp"
#include "way_admin_level_index.hpp"

using index_type = osmium::index::map::Map<osmium::unsigned_object_id_type, osmium::Location>;
using location_handler_type = osmium::handler::NodeLocationsForWays<index_type>;

void print_help(char* argv[]) {
    std::cerr <<
            "Export member ways of administrative boundary relations into a shape file. All member\n" \
            "ways get the admin_level of the relation with the lowest \n" \
            "Usage: " << argv[0] << " [ARGS] INPUT_FILE OUTPUT_FILE\n" \
            "Arguments:\n" \
            "  -i ARG, --index=ARG      Location index type (default: sparse_mmap_array, alternative:\n" \
			"                           dense_mmap_array)\n" \
            "  -M NUM, --max-level=NUM  Process levels 2 to N only (default 11).\n" \
            "  -v, --verbose            Enable verbose mode (show progress bar)\n";
    exit(1);
}

int main(int argc, char* argv[]) {
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"index", required_argument, 0, 'i'},
        {"max-level", required_argument, 0, 'M'},
        {"verbose", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };
    int max_level = 11;
    bool verbose = false;
    std::string index = "sparse_mmap_array";
    std::string input_filename;
    std::string output_filename;
    while (true) {
        int c = getopt_long(argc, argv, "hi:M:v", long_options, 0);
        if (c == -1) {
            break;
        }

        char** ptr = nullptr;
        switch (c) {
        case 'h':
            print_help(argv);
            break;
        case 'i':
        	if (!strcmp(optarg, "dense_mmap_array")) {
        		index = "dense_mmap_array";
        	} else if (strcmp(optarg, "sparse_mmap_array")) {
        		std::cerr << "ERROR: Unsupported index type\n";
        		exit(1);
        	}
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

    const auto& map_factory = osmium::index::MapFactory<osmium::unsigned_object_id_type, osmium::Location>::instance();
    auto location_index = map_factory.create_map(index);
    location_handler_type location_handler(*location_index);
    location_handler.ignore_errors(); // We will catch missing nodes by ourselves.

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
    std::cerr << "Writing to output file\n";
    AdminSHPHandler handler2 {way_level_idx, output_filename, max_level};
    {
        osmium::io::Reader reader2(input_file, osmium::osm_entity_bits::node | osmium::osm_entity_bits::way);
        osmium::ProgressBar progress_bar{reader2.file_size(), osmium::util::isatty(2) && verbose};
        while (osmium::memory::Buffer buffer = reader2.read()) {
            progress_bar.update(reader2.offset());
            osmium::apply(buffer, location_handler, handler2);
        }
        reader2.close();
        progress_bar.done();
        if (verbose) {
            std::cerr << way_level_idx.size() << " ways are used by admin boundary relations.\n";
        }
    }
}
