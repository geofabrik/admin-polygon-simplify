/*
 * boundary_relation_collector.hpp
 *
 *  Created on:  2017-02-07
 *      Author: Michael Reichert
 */

#ifndef SRC_BOUNDARY_RELATION_COLLECTOR_HPP_
#define SRC_BOUNDARY_RELATION_COLLECTOR_HPP_

#include <map>
#include <unordered_set>
#include <osmium/relations/collector.hpp>

using intmap_t = std::map<int, int>;

/**
 * \brief This class represents a segment of a ring, i.e. a way with additional information.
 */
class RingSegment {
    /// geometry of this segment
    const osmium::Way* m_way;

    /// true if the way has to be reversed to get a continous ring
    bool m_reversed;

public:
    RingSegment() = delete;

    RingSegment(const osmium::Way* way);

    /**
     * \brief Is the segment's way reversed?
     */
    bool reversed();

    /**
     * \brief Set the reversed flag.
     */
    void set_reversed();

    /**
     * Get ID of first node.
     *
     * m_reverse is considered
     */
    osmium::object_id_type front_ref();

    /**
     * Get ID of last node.
     *
     * m_reverse is considered
     */
    osmium::object_id_type back_ref();

    /**
     * Get ID of the underlying way
     */
    osmium::object_id_type id();
};

/**
 * \brief ring of a (multi)polygon.
 */
class Ring {
    std::list<RingSegment> m_ring_segments;

public:
    Ring();

    /**
     * The first segment is handed over
     *
     * \brief The segment should be handed over by value.
     *
     * Suggestion: Call the constructor like this: `Ring myring(std::move(segment));`
     */
    Ring(RingSegment first_segment);

    /**
     * Check if the segment matches to this ring.
     *
     * \param segment segment to be checked
     *
     * \returns true if it matches
     */
    bool check_match(RingSegment& segment);

    /**
     * Add a segment to this ring if it matches.
     *
     * \param segment segment to be added
     */
    void append(RingSegment&& segment);

    /**
     * Get number of member ways of this ring.
     */
    size_t size();

    /**
     * Check if the ring is already closed
     */
    bool closed();

    /**
     * Get ID of the first way.
     */
    osmium::object_id_type front_way_ref();

    /**
     * Get ID of the last way.
     */
    osmium::object_id_type back_way_ref();
};

/**
 * \brief Relation collector for boundary relations.
 *
 * This collector collects boundary relations and tries to assemble rings in order to
 * detect which rings have only one or two members.
 */
class BoundaryRelationCollector : public osmium::relations::Collector<BoundaryRelationCollector,
true, true, true> {
    /**
     * Set to track the IDs of the member ways of small rings (i.e. rings with only one or two members).
     */
    std::unordered_set<osmium::object_id_type>& m_treat_as_rings_way;

    /** Helper to retrieve relation member */
    osmium::Way* get_member_way(size_t offset) const;

public:
    BoundaryRelationCollector(std::unordered_set<osmium::object_id_type>& treat_as_rings_way);

    /**
     * This method decides which relations we're interested in, and
     * instructs Osmium to collect their members for us.
     */
    bool keep_relation(const osmium::Relation&) const;

    /**
     * Tells Osmium which members to keep for a relation of interest.
     */
    bool keep_member(const osmium::relations::RelationMeta& relation_meta, const osmium::RelationMember& member) const;

    /**
     * Called by Osmium when a relation has been fully read (i.e. all
     * members are present)
     */
    void complete_relation(osmium::relations::RelationMeta& relation_meta);
};


#endif /* SRC_BOUNDARY_RELATION_COLLECTOR_HPP_ */
