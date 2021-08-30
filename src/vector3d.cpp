/*
 * vector3d.cpp
 *
 *  Created on:  2017-02-08
 *      Author: Michael Reichert
 */

#include "vector3d.hpp"
#include <iostream>

Vector3D::Vector3D(double x, double y, double z) :
    VectorArray({x, y, z}) { }

Vector3D Vector3D::latlon_to_3d(const osmium::Location& point) {
    return Vector3D {
        EARTH_RADIUS * sin(osmium::geom::deg_to_rad(point.lon())),
        EARTH_RADIUS * cos(osmium::geom::deg_to_rad(point.lon())),
        EARTH_RADIUS * sin(osmium::geom::deg_to_rad(point.lat())),
    };
}

double Vector3D::vector_length() {
    return sqrt(at(0) * at(0) + at(1) * at(1) + at(2) * at(2));
}

Vector3D Vector3D::operator-(const Vector3D& other) {
    return Vector3D(at(0) - other.at(0), at(1) - other.at(1), at(2) - other.at(2));
}

Vector3D Vector3D::cross_product(const Vector3D& vector) {
    return Vector3D(
            at(1) * vector.at(2) - at(2) * vector.at(1),
            at(2) * vector.at(0) - at(0) * vector.at(2),
            at(0) * vector.at(1) - at(1) * vector.at(0)
            );
}

Vector3D& Vector3D::operator/=(const double other) {
    at(0) = at(0) / other;
    at(1) = at(1) / other;
    at(2) = at(2) / other;
    return *this;
}

Vector3D& Vector3D::operator*=(const double other) {
    at(0) = at(0) * other;
    at(1) = at(1) * other;
    at(2) = at(2) * other;
    return *this;
}

osmium::Location Vector3D::location() {
    return osmium::Location(
            osmium::geom::rad_to_deg(std::atan2(at(1), at(0))),
            osmium::geom::rad_to_deg(std::asin(at(2) / EARTH_RADIUS))
    );
}

void Vector3D::print() {
    std::cerr << "(" << at(0) << ", " << at(1) << ", " << at(2) << ")\n";
}
