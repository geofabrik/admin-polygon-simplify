/*
 * boundary_relation_collector.cpp
 *
 *  Created on:  2017-02-07
 *      Author: Michael Reichert
 */

#include <assert.h>
#include <iostream>
#include <list>
#include <osmium/osm/item_type.hpp>
#include "boundary_relation_collector.hpp"

RingSegment::RingSegment(const osmium::Way* way) :
        m_way(way),
        m_reversed(false) {
    assert(way);
}

bool RingSegment::reversed() {
    return m_reversed;
}

void RingSegment::set_reversed() {
    m_reversed = true;
}

osmium::object_id_type RingSegment::front_ref() {
    if (m_reversed) {
        return m_way->nodes().back().ref();
    } else {
        return m_way->nodes().front().ref();
    }
}

osmium::object_id_type RingSegment::back_ref() {
    if (m_reversed) {
        return m_way->nodes().front().ref();
    } else {
        return m_way->nodes().back().ref();
    }
}

Ring::Ring() :
        m_ring_segments() {}

Ring::Ring(RingSegment first_segment) :
        m_ring_segments() {
    m_ring_segments.push_back(first_segment);
}

osmium::object_id_type RingSegment::id() {
    return m_way->id();
}

bool Ring::check_match(RingSegment& segment) {
    if (m_ring_segments.front().front_ref() == segment.front_ref()
            || m_ring_segments.front().front_ref() == segment.back_ref()
            || m_ring_segments.back().back_ref() == segment.front_ref()
            || m_ring_segments.back().back_ref() == segment.back_ref()) {
        return true;
    }
    return false;
}

void Ring::append(RingSegment&& segment) {
    if (m_ring_segments.front().front_ref() == segment.front_ref()) {
        segment.set_reversed();
        m_ring_segments.push_front(segment);
    } else if (m_ring_segments.front().front_ref() == segment.back_ref()) {
        m_ring_segments.push_front(segment);
    } else if (m_ring_segments.back().back_ref() == segment.front_ref()) {
        m_ring_segments.push_back(segment);
    } else if (m_ring_segments.back().back_ref() == segment.back_ref()) {
        segment.set_reversed();
        m_ring_segments.push_back(segment);
    }
}

size_t Ring::size() {
    return m_ring_segments.size();
}

bool Ring::closed() {
    return m_ring_segments.front().front_ref() == m_ring_segments.back().back_ref();
}

osmium::object_id_type Ring::front_way_ref() {
    return m_ring_segments.front().id();
}

osmium::object_id_type Ring::back_way_ref() {
    return m_ring_segments.back().id();
}


BoundaryRelationCollector::BoundaryRelationCollector(std::unordered_set<osmium::object_id_type>& treat_as_rings_way) :
        m_treat_as_rings_way(treat_as_rings_way) { }

bool BoundaryRelationCollector::keep_relation(const osmium::Relation& relation) const {
    const char* type = relation.get_value_by_key("type", "");
    if (!strcmp(type, "boundary") || !strcmp(type, "multipolygon")) {
        return true;
    }
    return false;
}

bool BoundaryRelationCollector::keep_member(const osmium::relations::RelationMeta&,
        const osmium::RelationMember& member) const {
    if (member.type() == osmium::item_type::way) {
        return true;
    }
    return false;
}

osmium::Way* BoundaryRelationCollector::get_member_way(size_t offset) const {
    return static_cast<osmium::Way*>(&(this->get_member(offset)));
}

void BoundaryRelationCollector::complete_relation(osmium::relations::RelationMeta& relation_meta) {
    const osmium::Relation& relation = this->get_relation(relation_meta);
    std::vector<Ring> rings;

    // iterate over members, build preliminary rings
    for (const osmium::RelationMember& member : relation.members()) {
        if (member.type() == osmium::item_type::way) {
            std::pair<bool, size_t> avail_offset = get_availability_and_offset(member.type(), member.ref());
            if (avail_offset.first) {
                osmium::Way* way = this->get_member_way(this->get_offset(member.type(), member.ref()));
                RingSegment ring_segment(way);
                // check if any ring matches
                bool ring_found = false;
                for (Ring& r : rings) {
                    if (r.check_match(ring_segment)) {
                        r.append(std::move(ring_segment));
                        ring_found = true;
                        break;
                    }
                }
                if (!ring_found) {
                    Ring new_ring (ring_segment);
                    rings.push_back(std::move(new_ring));
                }
            }
        }
    }

    // look for rings with only one or two member ways
    for (Ring& r : rings) {
        if (r.size() <= 2 && r.closed()) {
            m_treat_as_rings_way.insert(r.front_way_ref());
            if (r.size() == 2) {
                m_treat_as_rings_way.insert(r.back_way_ref());
            }
        }
    }
}
