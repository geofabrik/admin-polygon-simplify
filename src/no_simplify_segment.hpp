/*
 * error_segment.hpp
 *
 *  Created on:  2017-02-14
 *      Author: Michael Reichert
 */

#ifndef SRC_NO_SIMPLIFY_SEGMENT_HPP_
#define SRC_NO_SIMPLIFY_SEGMENT_HPP_

#include "boundary_segment.hpp"

/**
 * segment which should not be simplified to prevent intersections
 */
struct NoSimplifySegment {
    size_t m_start_offset;
    size_t m_end_offset;
    osmium::Location m_intersection;
    bool m_deactivated = false;

    NoSimplifySegment(size_t start, size_t end, osmium::Location intersection) :
        m_start_offset(start),
        m_end_offset(end),
        m_intersection(intersection) {};

    NoSimplifySegment(BoundarySegment& segment, osmium::Location intersection) :
        m_start_offset(segment.get_start_offset()),
        m_end_offset(segment.get_end_offset()),
        m_intersection(intersection) {};

    struct LessThanComparator {
        bool operator()(const NoSimplifySegment* lhs, const NoSimplifySegment* rhs) const {
            return (lhs->m_start_offset == rhs->m_start_offset && lhs->m_end_offset < rhs->m_end_offset)
                    || lhs->m_start_offset < rhs->m_start_offset;
        }
    };

    struct EqualityComparator {
        bool operator()(const NoSimplifySegment* lhs, const NoSimplifySegment* rhs) const {
            return (lhs->m_start_offset == rhs->m_start_offset) && (lhs->m_end_offset == rhs->m_end_offset);
        }
    };
};
// necessary for sorting

inline bool operator<(const NoSimplifySegment& lhs, const NoSimplifySegment& rhs) {
    return (lhs.m_start_offset == rhs.m_start_offset && lhs.m_end_offset < rhs.m_end_offset)
            || lhs.m_start_offset < rhs.m_start_offset;
}

inline bool operator>(const NoSimplifySegment& lhs, const NoSimplifySegment& rhs) {
    return rhs < lhs;
}

inline bool operator<=(const NoSimplifySegment& lhs, const NoSimplifySegment& rhs) {
    return ! (rhs < lhs);
}

inline bool operator>=(const NoSimplifySegment& lhs, const NoSimplifySegment& rhs) {
    return ! (lhs < rhs);
}

using ErrorsMap = std::unordered_multimap<osmium::object_id_type, NoSimplifySegment>;
using KeepNodesMap = std::unordered_multimap<osmium::object_id_type, size_t>;

#endif /* SRC_NO_SIMPLIFY_SEGMENT_HPP_ */
