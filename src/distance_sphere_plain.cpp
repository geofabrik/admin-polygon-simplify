/*
 * distance_sphere_plain.cpp
 *
 *  Created on:  2017-02-08
 *      Author: Michael Reichert
 */

#include <osmium/geom/util.hpp>
#include <iostream>
#include "distance_sphere_plain.hpp"

Vector3D DistanceSpherePlain::latlon_to_3d(const osmium::Location& point) {
    return Vector3D({
        EARTH_RADIUS * sin(osmium::geom::deg_to_rad(point.lon())),
        EARTH_RADIUS * cos(osmium::geom::deg_to_rad(point.lon())),
        EARTH_RADIUS * sin(osmium::geom::deg_to_rad(point.lat())),
    });
}

double DistanceSpherePlain::distance_from_line(const osmium::Location& start,
        const osmium::Location& end, const osmium::Location& point) {
    // If the coordinates of start and end node are equal (i.e. closed ring), the distance can be calculated
    // directly from the coordinates.
    if (start == end) {
        return osmium::geom::haversine::distance(start, point);
    }

    Vector3D start_vec = latlon_to_3d(start);
    Vector3D end_vec = latlon_to_3d(end);
    Vector3D point_vec = latlon_to_3d(point);

    Vector3D line = end_vec - start_vec;

    return ((point_vec - start_vec).cross_product(line)).vector_length() / line.vector_length();
}

double DistanceSpherePlain::distance_from_line_less_trig(const osmium::Location& start,
        const osmium::Location& end, const osmium::Location& point) {
    // If the coordinates of start and end node are equal (i.e. closed ring), the distance can be calculated
    // directly from the coordinates.
    if (start == end) {
        return osmium::geom::haversine::distance(start, point);
    }
    // https://stackoverflow.com/questions/1299567/how-to-calculate-distance-from-a-point-to-a-line-segment-on-a-sphere
    Vector3D start_vec = latlon_to_3d(start);
    Vector3D end_vec = latlon_to_3d(end);
    Vector3D point_vec = latlon_to_3d(point);

    Vector3D g = start_vec.cross_product(end_vec);
    Vector3D f = point_vec.cross_product(g);
    Vector3D t = g.cross_product(f);
//    std::cerr << "vector T: ";
//    t.print();
    t /= t.vector_length();
//    t.at(0) /= t.vector_length();
//    t.at(1) /= t.vector_length();
//    t.at(2) /= t.vector_length();
//    std::cerr << " vector T normalized: ";
//    t.print();
//    std::cerr << "\n";
    t *= EARTH_RADIUS;
//    std::cerr << "Location of T: " << t.location().lon_without_check() << ", " << t.location().lat_without_check() << "\n";
    return osmium::geom::haversine::distance(t.location(), point);
}

double DistanceSpherePlain::distance_from_line_sphere(const osmium::Location& start,
        const osmium::Location& end, const osmium::Location& point) {
    // http://www.movable-type.co.uk/scripts/latlong.html
    // If the coordinates of start and end node are equal (i.e. closed ring), the distance can be calculated
    // directly from the coordinates.
    if (start == end) {
        return osmium::geom::haversine::distance(start, point);
    }

    // bearing from start to end
    double bearing12 = bearing(start, end);
    double bearing13 = bearing(start, point);
    double angel13 = osmium::geom::haversine::distance(start, point) / EARTH_RADIUS;
    return std::abs(std::asin(sin(angel13) * sin(bearing13 - bearing12)) * EARTH_RADIUS);
}

double DistanceSpherePlain::bearing(const osmium::Location& start, const osmium::Location& end) {
    double d_lon = osmium::geom::deg_to_rad(end.lon() - start.lon());
    // lon and lat of start and end node
    double lat1 = osmium::geom::deg_to_rad(start.lat());
    double lat2 = osmium::geom::deg_to_rad(end.lat());
    return std::atan2(sin(d_lon) * cos(lat2), cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2 * cos(d_lon)));
}
