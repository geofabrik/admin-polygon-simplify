/*
 * test_intersection.cpp
 *
 *  Created on:  2017-05-09
 *      Author: Michael Reichert <michael.reichert@geofabrik.de>
 */

#include "catch.hpp"

#include <unordered_set>

#include <intermediate_simplifier.hpp>

TEST_CASE("Intersection is start point of one segment") {
    ErrorsMap errors;
    KeepNodesMap nodes_to_be_kept;
    std::vector<BoundarySegment> segments;
    std::unordered_set<osmium::object_id_type> treat_as_rings_way;
    osmium::util::VerboseOutput vout(true);

    osmium::Location start1 (-57.0092500, -36.8587500);
    osmium::Location end1 (-57.0075000, -36.8607500);
    osmium::Location start2 (-57.0065000, -36.8625000);
    osmium::Location end2 (-57.0085000, -36.8590000);
    segments.emplace_back(start1, end1, 194711843, 60, 62);
    segments.emplace_back(start2, end2, 194711843, 68, 77);

    IntermediateSimplifier interm_simplifier (100, errors, segments, nodes_to_be_kept, vout);

    REQUIRE(interm_simplifier.intersection(segments.at(0), segments.at(1)) != osmium::Location());
}

TEST_CASE("One line is vertical") {
    ErrorsMap errors;
    KeepNodesMap nodes_to_be_kept;
    std::vector<BoundarySegment> segments;
    std::unordered_set<osmium::object_id_type> treat_as_rings_way;
    osmium::util::VerboseOutput vout(true);

    osmium::Location start1 (-57.007, -36.8);
    osmium::Location end1 (-57.007, -39.0);
    osmium::Location start2 (-57.0065000, -36.8625000);
    osmium::Location end2 (-57.0085000, -36.8590000);
    segments.emplace_back(start1, end1, 194711843, 60, 62);
    segments.emplace_back(start2, end2, 194711843, 68, 77);

    IntermediateSimplifier interm_simplifier (100, errors, segments, nodes_to_be_kept, vout);

    REQUIRE(interm_simplifier.intersection(segments.at(0), segments.at(1)) != osmium::Location());
}

TEST_CASE("Simple test") {
    ErrorsMap errors;
    KeepNodesMap nodes_to_be_kept;
    std::vector<BoundarySegment> segments;
    std::unordered_set<osmium::object_id_type> treat_as_rings_way;
    osmium::util::VerboseOutput vout(true);

    osmium::Location start1 (1.0, 1.0);
    osmium::Location end1 (2.0, 2.0);
    osmium::Location start2 (1.0, 2.0);
    osmium::Location end2 (2.0, 1.0);
    segments.emplace_back(start1, end1, 194711843, 60, 62);
    segments.emplace_back(start2, end2, 194711843, 68, 77);

    IntermediateSimplifier interm_simplifier (100, errors, segments, nodes_to_be_kept, vout);

    REQUIRE(interm_simplifier.intersection(segments.at(0), segments.at(1)) != osmium::Location());
}

TEST_CASE("parallel lines not intersecting") {
    ErrorsMap errors;
    KeepNodesMap nodes_to_be_kept;
    std::vector<BoundarySegment> segments;
    std::unordered_set<osmium::object_id_type> treat_as_rings_way;
    osmium::util::VerboseOutput vout(true);

    osmium::Location start1 (1.0, 2.0);
    osmium::Location end1 (3.0, 4.0);
    osmium::Location start2 (1.0, 2.1);
    osmium::Location end2 (2.0, 3.1);
    segments.emplace_back(start1, end1, 194711843, 60, 62);
    segments.emplace_back(start2, end2, 194711843, 68, 77);

    IntermediateSimplifier interm_simplifier (100, errors, segments, nodes_to_be_kept, vout);

    REQUIRE(interm_simplifier.intersection(segments.at(0), segments.at(1)) == osmium::Location());
}

TEST_CASE("parallel lines overlapping") {
    ErrorsMap errors;
    KeepNodesMap nodes_to_be_kept;
    std::vector<BoundarySegment> segments;
    std::unordered_set<osmium::object_id_type> treat_as_rings_way;
    osmium::util::VerboseOutput vout(true);

    osmium::Location start1 (1.0, 2.0);
    osmium::Location end1 (3.0, 4.0);
    osmium::Location start2 (0.8, 1.8);
    osmium::Location end2 (2.0, 3.0);
    segments.emplace_back(start1, end1, 194711843, 60, 62);
    segments.emplace_back(start2, end2, 194711843, 68, 77);

    IntermediateSimplifier interm_simplifier (100, errors, segments, nodes_to_be_kept, vout);

    REQUIRE(interm_simplifier.intersection(segments.at(0), segments.at(1)) != osmium::Location());
}

TEST_CASE("1234parallel lines overlapping") {
    ErrorsMap errors;
    KeepNodesMap nodes_to_be_kept;
    std::vector<BoundarySegment> segments;
    std::unordered_set<osmium::object_id_type> treat_as_rings_way;
    osmium::util::VerboseOutput vout(true);

    osmium::Location start1 (108.9491373,10.5007859);
    osmium::Location end1 (108.9500329,10.5038746);
    osmium::Location start2 (108.9491941,10.5008231);
    osmium::Location end2 (108.9501124,10.502694);
    segments.emplace_back(start1, end1, 194711843, 60, 62);
    segments.emplace_back(start2, end2, 194711843, 68, 77);

    IntermediateSimplifier interm_simplifier (100, errors, segments, nodes_to_be_kept, vout);

    REQUIRE(interm_simplifier.intersection(segments.at(0), segments.at(1)) == osmium::Location());
}
