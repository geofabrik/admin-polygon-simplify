/*
 * test_douglas_peucker_nonclosed.cpp
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
    test_douglas_peucker::simplify_node_list(way.nodes(), kept_node_refs, 0, way.nodes().size() - 1);
}

TEST_CASE("Line with two nodes") {
    SECTION("Any line with two nodes") {
        osmium::memory::Buffer buffer (1024*1024, osmium::memory::Buffer::auto_grow::yes);
        // build way
        std::vector<osmium::object_id_type> node_refs {1, 2};
        std::vector<osmium::Location> node_locations {osmium::Location(9.0, 50.0), osmium::Location(9.0, 50.1)};
        test_douglas_peucker::build_way(buffer, node_refs, node_locations);

        const osmium::Way& way = static_cast<const osmium::Way&>(*(buffer.cbegin()));
        std::vector<const osmium::NodeRef*> kept_node_refs {way.nodes().size(), nullptr};
        run_simplification(way, kept_node_refs);

        REQUIRE(test_utils::count_non_nullptr_elements(kept_node_refs) == kept_node_refs.size());
    }
}

TEST_CASE("Line with three nodes") {
    SECTION("Any line with three nodes") {
        osmium::memory::Buffer buffer (1024*1024, osmium::memory::Buffer::auto_grow::yes);
        // build way
        std::vector<osmium::object_id_type> node_refs {1, 2, 3};
        std::vector<osmium::Location> node_locations {
            osmium::Location(9.0, 50.0), osmium::Location(9.0, 50.1), osmium::Location(9.0, 50.10001)
        };
        test_douglas_peucker::build_way(buffer, node_refs, node_locations);

        const osmium::Way& way = static_cast<const osmium::Way&>(*(buffer.cbegin()));
        std::vector<const osmium::NodeRef*> kept_node_refs {way.nodes().size(), nullptr};
        run_simplification(way, kept_node_refs);

        REQUIRE(kept_node_refs.at(1) == nullptr);
        REQUIRE(test_utils::count_non_nullptr_elements(kept_node_refs) == 2);
    }

    SECTION("Line with three nodes which can be simplified") {
        osmium::memory::Buffer buffer (1024*1024, osmium::memory::Buffer::auto_grow::yes);
        // build way
        std::vector<osmium::object_id_type> node_refs {1, 2, 3};
        std::vector<osmium::Location> node_locations {
            osmium::Location(9.0, 50.0), osmium::Location(9.0, 50.1), osmium::Location(9.3, 50.10001)
        };
        test_douglas_peucker::build_way(buffer, node_refs, node_locations);

        const osmium::Way& way = static_cast<const osmium::Way&>(*(buffer.cbegin()));
        std::vector<const osmium::NodeRef*> kept_node_refs {way.nodes().size(), nullptr};
        run_simplification(way, kept_node_refs);

        REQUIRE(test_utils::count_non_nullptr_elements(kept_node_refs) == 3);
    }
}

TEST_CASE("Line with four nodes") {
    SECTION("all inner nodes should be dropped") {
        osmium::memory::Buffer buffer (1024*1024, osmium::memory::Buffer::auto_grow::yes);
        // build way
        std::vector<osmium::object_id_type> node_refs {1, 2, 3, 4};
        std::vector<osmium::Location> node_locations {
            osmium::Location(9.0, 50.0), osmium::Location(9.0, 50.1), osmium::Location(9.0, 50.25), osmium::Location(9.0, 50.3)
        };
        test_douglas_peucker::build_way(buffer, node_refs, node_locations);

        const osmium::Way& way = static_cast<const osmium::Way&>(*(buffer.cbegin()));
        std::vector<const osmium::NodeRef*> kept_node_refs {way.nodes().size(), nullptr};
        run_simplification(way, kept_node_refs);

        REQUIRE(test_utils::count_non_nullptr_elements(kept_node_refs) == 2);
        REQUIRE(kept_node_refs.at(1) == nullptr);
        REQUIRE(kept_node_refs.at(2) == nullptr);
    }
    SECTION("the second node should be dropped – case 1") {
        osmium::memory::Buffer buffer (1024*1024, osmium::memory::Buffer::auto_grow::yes);
        // build way
        std::vector<osmium::object_id_type> node_refs {1, 2, 3, 4};
        std::vector<osmium::Location> node_locations {
            osmium::Location(9.0, 50.0), osmium::Location(9.000331, 50.00089), osmium::Location(9.1, 50.1), osmium::Location(9.0, 50.3)
        };
        test_douglas_peucker::build_way(buffer, node_refs, node_locations);

        const osmium::Way& way = static_cast<const osmium::Way&>(*(buffer.cbegin()));
        std::vector<const osmium::NodeRef*> kept_node_refs {way.nodes().size(), nullptr};
        run_simplification(way, kept_node_refs);

        REQUIRE(test_utils::count_non_nullptr_elements(kept_node_refs) == 3);
        REQUIRE(kept_node_refs.at(1) == nullptr);
    }
    SECTION("the second node should be dropped – case 2") {
        osmium::memory::Buffer buffer (1024*1024, osmium::memory::Buffer::auto_grow::yes);
        // build way
        std::vector<osmium::object_id_type> node_refs {1, 2, 3, 4};
        std::vector<osmium::Location> node_locations {
            osmium::Location(9.0, 50.0), osmium::Location(9.0013701, 50.0005847), osmium::Location(9.1, 50.1), osmium::Location(9.0, 50.3)
        };
        test_douglas_peucker::build_way(buffer, node_refs, node_locations);

        const osmium::Way& way = static_cast<const osmium::Way&>(*(buffer.cbegin()));
        std::vector<const osmium::NodeRef*> kept_node_refs {way.nodes().size(), nullptr};
        run_simplification(way, kept_node_refs);

        REQUIRE(test_utils::count_non_nullptr_elements(kept_node_refs) == 3);
        REQUIRE(kept_node_refs.at(1) == nullptr);
    }
    SECTION("no nodes should be dropped – case 1 (first inner node only beyond distance threshold in second iteration)") {
        osmium::memory::Buffer buffer (1024*1024, osmium::memory::Buffer::auto_grow::yes);
        // build way
        std::vector<osmium::object_id_type> node_refs {1, 2, 3, 4};
        std::vector<osmium::Location> node_locations {
            osmium::Location(9.0, 50.0), osmium::Location(8.999172, 50.0004465), osmium::Location(9.1, 50.1), osmium::Location(9.0, 50.3)
        };
        test_douglas_peucker::build_way(buffer, node_refs, node_locations);

        const osmium::Way& way = static_cast<const osmium::Way&>(*(buffer.cbegin()));
        std::vector<const osmium::NodeRef*> kept_node_refs {way.nodes().size(), nullptr};
        run_simplification(way, kept_node_refs);

        REQUIRE(test_utils::count_non_nullptr_elements(kept_node_refs) == 4);
    }
}
