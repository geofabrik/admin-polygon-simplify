/*
 * osm_adminfilter.cpp
 *
 *  Created on:  2017-02-07
 *      Author: Michael Reichert
 */

#include <getopt.h>
#include <stdlib.h>
#include <osmium/io/reader.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/osm/tag.hpp>
#include <osmium/util/progress_bar.hpp>
#include <osmium/index/map/sparse_mmap_array.hpp>
#include <osmium/index/map/dense_mmap_array.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>
#include "boundary_filter_collector.hpp"

using index_type = osmium::index::map::Map<osmium::unsigned_object_id_type, osmium::Location>;
using location_handler_type = osmium::handler::NodeLocationsForWays<index_type>;

static constexpr int max_level_max = 11;
static int max_level = max_level_max;

void print_help(char* argv[]) {
    std::cerr <<
            "Error: Too few or too much arguments.\n" \
            "Usage: " << argv[0] << " INPUT_FILE OUTPUT_FILE\n" \
            "Arguments:\n" \
            "-a, --adminbounds       select all administrative boundaries\n" \
            "-C, --no-changeset      don't write changeset IDs to the output\n" \
            "-i INDEX, --index=INDEX use INDEX (default: sparse_mmap_array)\n" \
            "-L, --no-lastchange     don't write last_modified to the output\n" \
            "                        file\n" \
            "-M=N, --max-level=N     only ouput levels 2 to N\n" \
            "-n, --add-nodes         add nodes to the output file\n" \
            "-p, --postalcodes       select all postal code boundaries\n" \
            "-r, --add-relations     add relations to the output file\n" \
            "-V, --no-version        don't write version to the output file\n";
    exit(1);
}

bool is_adminbound(const osmium::TagList& tags) {
    const char* type = tags.get_value_by_key("type", "");
    const char* boundary = tags.get_value_by_key("boundary", "");
    if (!strcmp(type, "boundary") || !strcmp(type, "multipolygon")) {
        if (!strcmp(boundary, "administrative")) {
            const char* admin_level = tags.get_value_by_key("admin_level");
            if (admin_level && max_level != max_level_max) {
                char** ptr = nullptr;
                int parsed = static_cast<int>(strtol(admin_level, ptr, 10));
                return (!ptr && parsed <= max_level && parsed >= 2);
            } else if (!admin_level && max_level != max_level_max) {
                return false;
            } else {
                return true;
            }
        }
    }
    return false;
}

bool is_postalcode_bound(const osmium::TagList& tags) {
    const char* type = tags.get_value_by_key("type", "");
    const char* boundary = tags.get_value_by_key("boundary", "");
    if (!strcmp(type, "boundary")) {
        const char* postal_code = tags.get_value_by_key("postal_code");
        if (!strcmp(boundary, "postal_code") || postal_code) {
            return true;
        }
    }
    return false;
}

int main(int argc, char* argv[]) {

    static struct option long_options[] = {
        {"adminbounds", no_argument, 0, 'a'},
        {"no-changeset", required_argument, 0, 'C'},
        {"index", required_argument, 0, 'i'},
        {"no-lastchange", no_argument, 0, 'L'},
        {"max-level", required_argument, 0, 'M'},
        {"add-nodes", no_argument, 0, 'n'},
        {"postalcodes", no_argument, 0, 'p'},
        {"add-relations", no_argument, 0, 'r'},
        {"no-version", no_argument, 0, 'V'},
        {0, 0, 0, 0}
    };
    std::string location_index_type = "sparse_mmap_array";
    std::string input_filename;
    std::string output_filename;
    bool adminbounds = false;
    bool postal_codes = false;
    bool changeset = true;
    bool lastchange = true;
    bool version = true;
    bool add_nodes = false;
    bool add_relations = false;
    while (true) {
        int c = getopt_long(argc, argv, "aCi:LM:nprV", long_options, 0);
        if (c == -1) {
            break;
        }

        char** ptr = nullptr;
        switch (c) {
        case 'a':
            adminbounds = true;
            break;
        case 'C':
            changeset = false;
            break;
        case 'i':
            location_index_type = optarg;
            break;
        case 'L':
            lastchange = false;
            break;
        case 'M':
            max_level = static_cast<int>(strtol(optarg, ptr, 10));
            if (ptr || max_level < 2 || max_level > 11) {
                std::cerr << "ERROR: Invalid argument for option --max-level\n";
                exit(1);
            }
            break;
        case 'n':
            add_nodes = true;
            break;
        case 'p':
            postal_codes = true;
            break;
        case 'r':
            add_relations = true;
            break;
        case 'V':
            version = false;
            break;
        default:
            exit(1);
        }
    }
    int remaining_args = argc - optind;
    if (remaining_args != 2) {
        print_help(argv);
    }
    if (!adminbounds && !postal_codes) {
        std::cerr << "ERROR: No filter selected. Use at least -a or -p.\n";
        print_help(argv);
    }
    input_filename =  argv[optind];
    output_filename = argv[optind+1];

    std::vector<std::function<bool (const osmium::TagList &)>> check_functions;
    if (adminbounds) {
        check_functions.push_back(is_adminbound);
    }
    if (postal_codes) {
        check_functions.push_back(is_postalcode_bound);
    }

    BoundaryFilterCollector collector(output_filename, check_functions, changeset, lastchange, version, add_nodes, add_relations);
    osmium::io::File input_file(input_filename);
    osmium::io::Reader reader1{input_file, osmium::osm_entity_bits::relation};
    collector.read_relations(reader1);
    reader1.close();
    osmium::io::Reader reader2(input_file);
    const auto& map_factory = osmium::index::MapFactory<osmium::unsigned_object_id_type, osmium::Location>::instance();
    auto location_index = map_factory.create_map(location_index_type);
    location_handler_type location_handler(*location_index);
    osmium::ProgressBar progress_bar{reader2.file_size(), osmium::util::isatty(2)};
    while (osmium::memory::Buffer buffer = reader2.read()) {
        progress_bar.update(reader2.offset());
        osmium::apply(buffer, location_handler, collector.handler());
    }
    progress_bar.done();
    collector.write_to_file();
    reader2.close();
}
