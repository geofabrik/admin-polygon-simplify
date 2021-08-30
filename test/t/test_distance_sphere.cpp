/*
 * test_distance_sphere.cpp
 *
 *  Created on:  2017-02-09
 *      Author: Michael Reichert
 */

#include "catch.hpp"
#include <distance_sphere_plain.hpp>

/**
 * \brief Test if distance calcuation works.
 */
TEST_CASE("Distances between two points") {
//    SECTION("(0, 0)--(0.3, 0) |- (0.15, 0.1)") {
//        DistanceSpherePlain dist_object;
//        double dist = dist_object.distance_from_line(osmium::Location(0.0, 0.0), osmium::Location(0.3, 0.0), osmium::Location(0.15, 0.1));
//        std::cerr << "This is the distance: " << dist << "\n";
//        REQUIRE(dist == Approx(11119).epsilon(5));
//    }
//
//    SECTION("(0, 0)--(0.3, 0) |- (0.15, 0.002)") {
//        DistanceSpherePlain dist_object;
//        double dist = dist_object.distance_from_line(osmium::Location(0.0, 0.0), osmium::Location(0.3, 0.0), osmium::Location(0.15, 0.002));
//        std::cerr << "This is the distance: " << dist << "\n";
//        REQUIRE(dist == Approx(222).epsilon(3));
//    }
//
//    SECTION("(0, 0)--(0, 0.3) |- (0.002, 0.15)") {
//        DistanceSpherePlain dist_object;
//        double dist = dist_object.distance_from_line(osmium::Location(0.0, 0.0), osmium::Location(0.0, 0.3), osmium::Location(0.002, 0.15));
//        std::cerr << "This is the distance: " << dist << "\n";
//        REQUIRE(dist == Approx(222).epsilon(3));
//    }
//
//    SECTION("(-95.1534, 49.3845)--(-95.1531, 49.0) |- (-95.1517, 49.2274)") {
//        DistanceSpherePlain dist_object;
//        double dist = dist_object.distance_from_line(osmium::Location(-95.1534, 49.3845), osmium::Location(-95.1531, 49.0), osmium::Location(-95.1517, 49.2274));
//        std::cerr << "This is the distance: " << dist << "\n";
//        std::cerr << "#######################\n";
//        REQUIRE(dist == Approx(114).epsilon(0.0001));
//    }



    SECTION("(0, 0)--(0.3, 0) |- (0.15, 0.1)") {
        DistanceSpherePlain dist_object;
        double dist = dist_object.distance_from_line_sphere(osmium::Location(0.0, 0.0), osmium::Location(0.3, 0.0), osmium::Location(0.15, 0.1));
        REQUIRE(dist == Approx(11119).epsilon(5));
    }

    SECTION("(0, 0)--(0.3, 0) |- (0.15, 0.002)") {
        DistanceSpherePlain dist_object;
        double dist = dist_object.distance_from_line_sphere(osmium::Location(0.0, 0.0), osmium::Location(0.3, 0.0), osmium::Location(0.15, 0.002));
        REQUIRE(dist == Approx(222).epsilon(3));
    }

    SECTION("(0, 0)--(0, 0.3) |- (0.002, 0.15)") {
        DistanceSpherePlain dist_object;
        double dist = dist_object.distance_from_line_sphere(osmium::Location(0.0, 0.0), osmium::Location(0.0, 0.3), osmium::Location(0.002, 0.15));
        REQUIRE(dist == Approx(222).epsilon(3));
    }

    SECTION("(-95.1534, 49.3845)--(-95.1531, 49.0) |- (-95.1517, 49.2274)") {
        DistanceSpherePlain dist_object;
        double dist = dist_object.distance_from_line_sphere(osmium::Location(-95.1534, 49.3845), osmium::Location(-95.1531, 49.0), osmium::Location(-95.1517, 49.2274));
        REQUIRE(dist == Approx(114).epsilon(3));
    }


//    SECTION("(0, 0)--(0.3, 0) |- (0.15, 0.1)") {
//        DistanceSpherePlain dist_object;
//        double dist = dist_object.distance_from_line_less_trig(osmium::Location(0.0, 0.0), osmium::Location(0.3, 0.0), osmium::Location(0.15, 0.1));
//        std::cerr << "This is the distance: " << dist << "\n";
//        REQUIRE(dist == Approx(11119).epsilon(5));
//    }
//
//    SECTION("(0, 0)--(0.3, 0) |- (0.15, 0.002)") {
//        DistanceSpherePlain dist_object;
//        double dist = dist_object.distance_from_line_less_trig(osmium::Location(0.0, 0.0), osmium::Location(0.3, 0.0), osmium::Location(0.15, 0.002));
//        std::cerr << "This is the distance: " << dist << "\n";
//        REQUIRE(dist == Approx(222).epsilon(3));
//    }
//
//    SECTION("(0, 0)--(0, 0.3) |- (0.002, 0.15)") {
//        DistanceSpherePlain dist_object;
//        double dist = dist_object.distance_from_line_less_trig(osmium::Location(0.0, 0.0), osmium::Location(0.0, 0.3), osmium::Location(0.002, 0.15));
//        std::cerr << "This is the distance: " << dist << "\n";
//        REQUIRE(dist == Approx(222).epsilon(3));
//    }
//
//    SECTION("(-95.1534, 49.3845)--(-95.1531, 49.0) |- (-95.1517, 49.2274)") {
//        DistanceSpherePlain dist_object;
//        double dist = dist_object.distance_from_line_less_trig(osmium::Location(-95.1534, 49.3845), osmium::Location(-95.1531, 49.0), osmium::Location(-95.1517, 49.2274));
//        std::cerr << "This is the distance: " << dist << "\n";
//        REQUIRE(dist == Approx(114).epsilon(0.0001));
//    }
}


