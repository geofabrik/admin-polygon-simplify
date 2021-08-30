/*
 * test_douglas_peucker_closed.cpp
 *
 *  Created on:  2017-02-10
 *      Author: Michael Reichert
 */

#include "catch.hpp"
#include "util.hpp"
#include <distance_sphere_plain.hpp>

void run_simplification(const osmium::Way& way, std::vector<const osmium::NodeRef*>& kept_node_refs) {
    // We don't discard the first and last node
    kept_node_refs.front() = &(way.nodes().front());
    kept_node_refs.back() = &(way.nodes().back());

    // simplify the node list
    test_douglas_peucker::simplify_node_list_area(way.nodes(), kept_node_refs);
}

TEST_CASE("Line with 5 nodes") {
    SECTION("no inner node should be dropped") {
        osmium::memory::Buffer buffer (1024*1024, osmium::memory::Buffer::auto_grow::yes);
        // build way
        std::vector<osmium::object_id_type> node_refs {1, 2, 3, 4, 1};
        std::vector<osmium::Location> node_locations {
            osmium::Location(9.0, 50.0), osmium::Location(8.8, 50.11),osmium::Location(9.0, 50.3), osmium::Location(9.14, 50.18),
            osmium::Location(9.0, 50.0)
        };
        test_douglas_peucker::build_way(buffer, node_refs, node_locations);

        const osmium::Way& way = static_cast<const osmium::Way&>(*(buffer.cbegin()));
        std::vector<const osmium::NodeRef*> kept_node_refs {way.nodes().size(), nullptr};
        run_simplification(way, kept_node_refs);

        REQUIRE(test_utils::count_non_nullptr_elements(kept_node_refs) == kept_node_refs.size());
    }

    SECTION("the second node should be dropped") {
        osmium::memory::Buffer buffer (1024*1024, osmium::memory::Buffer::auto_grow::yes);
        // build way
        std::vector<osmium::object_id_type> node_refs {1, 2, 3, 4, 1};
        std::vector<osmium::Location> node_locations {
            osmium::Location(9.0, 50.0), osmium::Location(8.99951, 50.124029),osmium::Location(9.0, 50.3), osmium::Location(9.14, 50.18),
            osmium::Location(9.0, 50.0)
        };
        test_douglas_peucker::build_way(buffer, node_refs, node_locations);

        const osmium::Way& way = static_cast<const osmium::Way&>(*(buffer.cbegin()));
        std::vector<const osmium::NodeRef*> kept_node_refs {way.nodes().size(), nullptr};
        run_simplification(way, kept_node_refs);

        REQUIRE(test_utils::count_non_nullptr_elements(kept_node_refs) == 4);
        REQUIRE(kept_node_refs.at(1) == nullptr);
    }
}

TEST_CASE("Line with 7 nodes") {
    SECTION("the second node should be dropped") {
        osmium::memory::Buffer buffer (1024*1024, osmium::memory::Buffer::auto_grow::yes);
        // build way
        std::vector<osmium::object_id_type> node_refs {1, 2, 3, 10, 11, 4, 1};
        std::vector<osmium::Location> node_locations {
            osmium::Location(9.0, 50.0), osmium::Location(8.99951, 50.124029),osmium::Location(9.0, 50.3),
            osmium::Location(9.080332, 50.2756495), osmium::Location(9.0785133, 50.2308827),
            osmium::Location(9.14, 50.18), osmium::Location(9.0, 50.0)
        };
        test_douglas_peucker::build_way(buffer, node_refs, node_locations);

        const osmium::Way& way = static_cast<const osmium::Way&>(*(buffer.cbegin()));
        std::vector<const osmium::NodeRef*> kept_node_refs {way.nodes().size(), nullptr};
        run_simplification(way, kept_node_refs);

        REQUIRE(test_utils::count_non_nullptr_elements(kept_node_refs) == kept_node_refs.size() - 1);
        REQUIRE(kept_node_refs.at(1) == nullptr);
    }
}


