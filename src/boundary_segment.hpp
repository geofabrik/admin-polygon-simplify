/*
 * boundary_segment.hpp
 *
 *  Created on:  2017-02-13
 *      Author: Michael Reichert
 */

#ifndef SRC_BOUNDARYSEGMENT_HPP_
#define SRC_BOUNDARYSEGMENT_HPP_

#include <osmium/osm/undirected_segment.hpp>
#include <osmium/osm/types.hpp>

class BoundarySegment : public osmium::UndirectedSegment {
    /**
     * segment has been reversed due to the constraints of osmium::UndirectedSegment
     */
    bool m_reverse = false;

    bool m_deactivated = false;

    /**
     * ID of the OSM way this segment belongs to
     */
    osmium::object_id_type m_way_id;

    /**
     * \brief offset of the start node from start of the way
     *
     * the offset of the start node of this segment from the beginning of the way (number of nodes)
     */
    size_t m_start_offset;

    /**
     * \brief offset of the end node from start of the way
     *
     * the offset of the end node of this segment from the beginning of the way (number of nodes)
     */
    size_t m_end_offset;

public:
    explicit BoundarySegment(const osmium::Location& location1, const osmium::Location& location2,
            osmium::object_id_type way_id, size_t start_offset, size_t end_offset);

    bool get_reverse();

    /**
     * \brief Get start ID.
     *
     * Returns start ID and respects #m_reverse.
     */
    size_t get_start_offset();

    /**
     * \brief Get start ID.
     *
     * Returns start ID and respects #m_reverse.
     */
    size_t get_end_offset();

    /**
     * \brief Get number of nodes which are omitted by this segment.
     */

    size_t omitted_count();

    /**
     * \brief Get way ID.
     */
    osmium::object_id_type id();

    bool active();

    void deactivate();
};

#endif /* SRC_BOUNDARYSEGMENT_HPP_ */
