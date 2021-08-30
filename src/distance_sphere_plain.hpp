/*
 * distance_sphere_plain.hpp
 *
 *  Created on:  2017-02-08
 *      Author: Michael Reichert
 */

#ifndef SRC_DISTANCE_SPHERE_PLAIN_HPP_
#define SRC_DISTANCE_SPHERE_PLAIN_HPP_

#include <osmium/osm/location.hpp>
#include <osmium/geom/haversine.hpp>
#include "vector3d.hpp"

class DistanceSpherePlain {
    const double EARTH_RADIUS = osmium::geom::haversine::EARTH_RADIUS_IN_METERS; //6378137;

    /**
     * \brief Convert a point from latitude and longitude to 3D geocentric cartesian coordinates
     */
    Vector3D latlon_to_3d(const osmium::Location& point);

    /**
     * \brief Get bearing initial bearing from start to end point.
     *
     * Formula from http://www.movable-type.co.uk/scripts/latlong.html
     */
    double bearing(const osmium::Location& start, const osmium::Location& end);

public:
    /**
     * \brief Get distance between a point and a line
     *
     * \param start start point of the line
     * \param end end point of the line
     * \param point point whose distance should be calculated
     *
     * This uses a self-developed, INACCURATE method.
     *
     * All calculations assume that the earth is plain between the three points and the returned distance is measured on this plain.
     *
     * This implementation calls 9 times a trigonometric function and two times sqrt().
     * \returns distance in meters
     */
    double distance_from_line(const osmium::Location& start, const osmium::Location& end, const osmium::Location& point);

    /**
     * \brief Get distance between a point and a line
     *
     * \param start start point of the line
     * \param end end point of the line
     * \param point point whose distance should be calculated
     *
     * This uses formulas from
     * https://stackoverflow.com/questions/1299567/how-to-calculate-distance-from-a-point-to-a-line-segment-on-a-sphere
     * and calculates an intersection point by intersecting plains.
     *
     * This implementation calls 14 times a trigonometric function and two times sqrt().
     *
     * The implementation does not work correctly.
     *
     * \returns distance in meters
     */
    double distance_from_line_less_trig(const osmium::Location& start, const osmium::Location& end, const osmium::Location& point);

    /**
     * \brief Get distance between a point and a line
     *
     * \param start start point of the line
     * \param end end point of the line
     * \param point point whose distance should be calculated
     *
     * This uses formulas from http://www.movable-type.co.uk/scripts/latlong.html
     *
     * This implementation calls 24 times a trigonometric function and one time sqrt().
     *
     * \returns distance in meters
     */
    double distance_from_line_sphere(const osmium::Location& start, const osmium::Location& end, const osmium::Location& point);
};



#endif /* SRC_DISTANCE_SPHERE_PLAIN_HPP_ */
