/*
 * abstract_way_simplifier.hpp
 *
 *  Created on:  2017-02-14
 *      Author: Michael Reichert
 */

#ifndef SRC_ABSTRACT_WAY_SIMPLIFIER_HPP_
#define SRC_ABSTRACT_WAY_SIMPLIFIER_HPP_

#include <vector>
#include <osmium/handler.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/osm/relation.hpp>

#include "distance_sphere_plain.hpp"

class AbstractWaySimplifier : public osmium::handler::Handler {
protected:
    double m_epsilon = 70;
    bool m_reached_relations = false;

    DistanceSpherePlain m_distance_calculator;

public:
    AbstractWaySimplifier(double epsilon);

    virtual ~AbstractWaySimplifier() {}

    virtual void way(const osmium::Way& way) = 0;

    void relation(const osmium::Relation&) {}

    /**
     * Douglas-Peucker algorithm for closed rings. They have to be splitted up into three subsets
     * because otherwise it would be possible that the two subsets can be simplified that much that only the start==end
     * node of the ring and the node with the largest distance remain. The result is a degenerated area.
     *
     * \param node_list reference to node list of the way to be simplified
     * \param kept_node_refs Vector of pointers to instances of osmium::NodeRef. If a node should be preserved to prevent
     * intersections, its pointer has to be set before this method is called. The method will add valid pointers to all
     * nodes which are not omitted by the simplification algorithm.
     */
    void simplify_closed_ring(const osmium::WayNodeList& node_list, std::vector<const osmium::NodeRef*>& kept_node_refs);

    /**
     * Douglas-Peucker implementation for non-closed ways.
     *
     * \param node_list reference to node list of the way to be simplified
     * \param kept_node_refs Vector of pointers to instances of osmium::NodeRef. If a node should be preserved to prevent
     * intersections, its pointer has to be set before this method is called. The method will add valid pointers to all
     * nodes which are not omitted by the simplification algorithm.
     */
    void simplify_node_list(const osmium::WayNodeList& node_list, std::vector<const osmium::NodeRef*>& kept_node_refs,
            size_t segment_start_offset, size_t segment_end_offset);
};



#endif /* SRC_ABSTRACT_WAY_SIMPLIFIER_HPP_ */
