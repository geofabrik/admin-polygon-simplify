/*
 * boundary_segment.cpp
 *
 *  Created on:  2017-02-14
 *      Author: Michael Reichert
 */

#include "boundary_segment.hpp"

BoundarySegment::BoundarySegment(const osmium::Location& location1, const osmium::Location& location2,
        osmium::object_id_type way_id, size_t start_offset, size_t end_offset) :
        osmium::UndirectedSegment(location1, location2),
        m_reverse (first() == location1 ? false : true),
        m_way_id(way_id),
        // We have to check if locations have been swapped, i.e. segment is reverse.
        m_start_offset(first() == location1 ? start_offset : end_offset),
        m_end_offset(first() == location1 ? end_offset : start_offset) {}

bool BoundarySegment::get_reverse() {
    return m_reverse;
}

size_t BoundarySegment::get_start_offset() {
    return m_reverse ? m_end_offset : m_start_offset;
}

size_t BoundarySegment::get_end_offset() {
    return m_reverse ? m_start_offset : m_end_offset;
}

size_t BoundarySegment::omitted_count() {
    return m_end_offset - m_start_offset - 1;
}

osmium::object_id_type BoundarySegment::id() {
    return m_way_id;
}

bool BoundarySegment::active() {
    return !m_deactivated;
}

void BoundarySegment::deactivate() {
    m_deactivated = true;
}
