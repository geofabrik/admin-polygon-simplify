/*
 * vector3d.hpp
 *
 *  Created on:  2017-02-08
 *      Author: Michael Reichert
 */

#ifndef SRC_VECTOR3D_HPP_
#define SRC_VECTOR3D_HPP_

#include <array>
#include <osmium/osm/location.hpp>
#include <osmium/geom/haversine.hpp>

using VectorArray = std::array<double, 3>;

/**
 * \brief Helper class representing a 3-dimensional vector
 */
class Vector3D : public VectorArray {
    static constexpr double EARTH_RADIUS = osmium::geom::haversine::EARTH_RADIUS_IN_METERS;

public:
    Vector3D(double x, double y, double z);

    /**
     * factory method
     */
    static Vector3D latlon_to_3d(const osmium::Location& from);

    /**
     * subtraction
     */
    Vector3D operator-(const Vector3D& other);

    /**
     * \brief multiply with reciprocal of a scalar
     *
     * This operator is useful if you want to normalize a vector.
     */
    Vector3D& operator/=(const double other);

    /**
     * \brief multiply with a scalar
     *
     * This operator is useful if you want to normalize a vector.
     */
    Vector3D& operator*=(const double other);

    /**
     * cross product
     */
    Vector3D cross_product(const Vector3D& other);

    /**
     * \brief length of a vector
     */
    double vector_length();

    osmium::Location location();

    void print();
};


#endif /* SRC_VECTOR3D_HPP_ */
