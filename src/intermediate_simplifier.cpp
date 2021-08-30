/*
 * intermediate_simplifier.cpp
 *
 *  Created on:  2017-05-03
 *      Author: Michael Reichert <michael.reichert@geofabrik.de>
 */

#include "intermediate_simplifier.hpp"
#include <iostream>

osmium::Location IntermediateSimplifier::intersection(const osmium::Segment& s1, const osmium::Segment&s2) {
    if (s1.first()  == s2.first()  ||
        s1.first()  == s2.second() ||
        s1.second() == s2.first()  ||
        s1.second() == s2.second()) {
        return osmium::Location();
    }

    double t1x = s1.second().lon() - s1.first().lon();
    double t1y = s1.second().lat() - s1.first().lat();
    double t2x = s2.second().lon() - s2.first().lon();
    double t2y = s2.second().lat() - s2.first().lat();
    double p11x = s1.first().lon();
    double p11y = s1.first().lat();
    double p21x = s2.first().lon();
    double p21y = s2.first().lat();

    double s;
    double r;
    if (t1x == 0) {
        s = (p11x - p21x) / t2x; // kx = p21x + s * t2x
        r = (p21y + s * t2y - p11y) / t1y;
    } else if (t1y / t1x == t2y / t2x) {
        if ((p21x - p11x) / t2x == (p21y - p11y) / t2y) {
            // Every point on s1 is also a point on s2.
            return osmium::Location(s1.first());
        } else {
            // They are parallel and not intersecting.
            return osmium::Location();
        }
    } else {
        s = (p11y - p21y + ((p21x - p11x) * (t1y / t1x)))
                / (t2y - ((t2x * t1y) / t1x));
        r = (p21x - p11x + s * t2x) / t1x;
    }
    // we have to neglet a small error r and s to circumvent some numerical problems
    // If the base line is 100 km long, this will result in an error of 5 mm.
    if (r >= -0.0000001 && r <= 1.0000001 && s >= -0.0000001 && s <= 1.0000001) {
        return osmium::Location(p11x + r * t1x, p11y + r * t1y);
    }
    return osmium::Location();
}

bool IntermediateSimplifier::outside_x_range(const osmium::UndirectedSegment& s1, const osmium::UndirectedSegment& s2) {
    if (s1.first().x() > s2.second().x()) {
        return true;
    }
    return false;
}

bool IntermediateSimplifier::y_range_overlap(const osmium::UndirectedSegment& s1, const osmium::UndirectedSegment& s2) {
    const int tmin = s1.first().y() < s1.second().y() ? s1.first().y( ) : s1.second().y();
    const int tmax = s1.first().y() < s1.second().y() ? s1.second().y() : s1.first().y();
    const int omin = s2.first().y() < s2.second().y() ? s2.first().y()  : s2.second().y();
    const int omax = s2.first().y() < s2.second().y() ? s2.second().y() : s2.first().y();
    if (tmin > omax || omin > tmax) {
        return false;
    }
    return true;
}

IntermediateSimplifier::IntermediateSimplifier(double epsilon, ErrorsMap& error_segments, std::vector<BoundarySegment>& all_segments,
        KeepNodesMap& keep_nodes, osmium::util::VerboseOutput& vout) :
    AbstractWaySimplifier(epsilon),
    m_error_segments(error_segments),
    m_all_segments(all_segments),
    m_kept_nodes(keep_nodes),
    m_vout(vout) { }

std::vector<NoSimplifySegment*> IntermediateSimplifier::sort_no_simplify_segments(osmium::object_id_type way_id) {
    std::pair<ErrorsMap::iterator, ErrorsMap::iterator> it_range = m_error_segments.equal_range(way_id);
    std::vector<NoSimplifySegment*> ordered;
    for (ErrorsMap::iterator it = it_range.first; it != it_range.second; it++) {
        if (!(it->second.m_deactivated)) {
            ordered.push_back(&(it->second));
            it->second.m_deactivated = true;
        }
    }
    // sort and remove duplicates
    std::sort(ordered.begin(), ordered.end(), NoSimplifySegment::LessThanComparator());
    ordered.erase(std::unique(ordered.begin(), ordered.end(), NoSimplifySegment::EqualityComparator()), ordered.end());
    return ordered;
}

void IntermediateSimplifier::improve_simplification(const osmium::Way& way) {
    std::vector<NoSimplifySegment*> ordered = sort_no_simplify_segments(way.id());
    if (ordered.size() == 0 || ordered.size() == way.nodes().size() - 1) {
        // every segment intersects, nothing to simplify
        // OR: no segment intersects
        return;
    }
    // vector for kept node references; entrys which are nullptr mean that we discard these nodes
    std::vector<const osmium::NodeRef*> kept_node_refs {way.nodes().size(), nullptr};
    for (NoSimplifySegment* segment : ordered) {
        if (segment->m_end_offset - segment->m_start_offset > 1) {
            // ignore if the intersection is at the beginning of the segment or the segment cannot be shortened any more
            // get node with largest distance for this segment
            size_t offset_largest = 0;
            double distance_max = 0;
            for (size_t i = segment->m_start_offset + 1; i < segment->m_end_offset; ++i) {
                double distance = m_distance_calculator.distance_from_line_sphere(way.nodes()[segment->m_start_offset].location(),
                        way.nodes()[segment->m_end_offset].location(), way.nodes()[i].location());
                // We have to use >=, not > because otherwise offset_largest will be 0 at the end of the loop
                // if the segment is straight.
                if (distance >= distance_max) {
                    distance_max = distance;
                    offset_largest = i;
                }
            }
            // split segment
            m_all_segments.emplace_back(way.nodes()[segment->m_start_offset].location(), way.nodes()[offset_largest].location(), way.id(),
                    segment->m_start_offset, offset_largest);
            m_all_segments.emplace_back(way.nodes()[offset_largest].location(), way.nodes()[segment->m_end_offset].location(), way.id(),
                    offset_largest, segment->m_end_offset);
            m_kept_nodes.insert(std::make_pair<osmium::object_id_type, size_t>(way.id(), static_cast<size_t>(offset_largest)));
        } else {
            m_all_segments.emplace_back(way.nodes()[segment->m_start_offset].location(), way.nodes()[segment->m_end_offset].location(), way.id(),
                                segment->m_start_offset, segment->m_end_offset);
        }
    }
}

void IntermediateSimplifier::way(const osmium::Way& way) {
    improve_simplification(way);
}

bool IntermediateSimplifier::recheck_intersections() {
    // code copied from osmcoastline by Jochen Topf, GPL license
    m_vout << "Sort segments ...\n";
    std::sort(m_all_segments.begin(), m_all_segments.end());
    bool intersection_found = false;
    m_vout << "Looking for intersections ...\n";
    for (std::vector<BoundarySegment>::iterator it1 = m_all_segments.begin(); it1 != m_all_segments.end()-1; ++it1) {
        BoundarySegment& s1 = *it1;
        if (!s1.active()) {
            continue;
        }
        for (std::vector<BoundarySegment>::iterator it2 = it1 + 1; it2 != m_all_segments.end(); ++it2) {
            BoundarySegment& s2 = *it2;
            if (!s2.active()) {
                continue;
            }
            if (s1 == s2 && (s1.omitted_count() > 0 || s2.omitted_count() > 0)) {
                // At least one of the segments must not be an unsimplified segment.
                if (s1.omitted_count() > 0) {
                    m_error_segments.insert(std::make_pair<osmium::object_id_type, NoSimplifySegment>(s1.id(), NoSimplifySegment(s1, s1.first())));
                    s1.deactivate();
                }
                if (s2.omitted_count() > 0) {
                    m_error_segments.insert(std::make_pair<osmium::object_id_type, NoSimplifySegment>(s2.id(), NoSimplifySegment(s2, s1.first())));
                    s2.deactivate();
                }
                intersection_found = true;
            } else if (s1 != s2) {
                if (outside_x_range(s2, s1)) {
                    break;
                }
                if (y_range_overlap(s1, s2)) {
                    osmium::Location i = intersection(s1, s2);
                    if (i) {
                        // We should not report segments as erroreouns which are only two nodes long. They cannot become better.
                        if (i != s1.first() && i != s1.second() && s1.omitted_count() > 0) {
                            m_error_segments.insert(std::make_pair<osmium::object_id_type, NoSimplifySegment>(s1.id(), NoSimplifySegment(s1, i)));
                            s1.deactivate();
                        }
                        if (i != s2.first() && i != s2.second() && s2.omitted_count() > 0) {
                            m_error_segments.insert(std::make_pair<osmium::object_id_type, NoSimplifySegment>(s2.id(), NoSimplifySegment(s2, i)));
                            s2.deactivate();
                        }
                        intersection_found = true;
                    }
                }
            }
        }
    }
    return intersection_found;
}
