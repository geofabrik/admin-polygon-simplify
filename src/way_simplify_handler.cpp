/*
 * way_simplify_handler.cpp
 *
 *  Created on:  2017-02-08
 *      Author: Michael Reichert
 */

#include <iostream>
#include "way_simplify_handler.hpp"
#include "distance_sphere_plain.hpp"

WaySimplifyHandler::WaySimplifyHandler(double epsilon, std::vector<BoundarySegment>& segments,
        std::unordered_set<osmium::object_id_type>& treat_as_rings_way) :
        AbstractWaySimplifier(epsilon),
        m_segments(segments),
        m_treat_as_rings_way(treat_as_rings_way) { }

void WaySimplifyHandler::add_simplified_node_list(const osmium::WayNodeList& node_list, osmium::object_id_type way_id) {
    // vector for kept node references; entrys which are nullptr mean that we discard these nodes
    std::vector<const osmium::NodeRef*> kept_node_refs {node_list.size(), nullptr};
    // We don't discard the first and last node
    kept_node_refs.front() = &(node_list.front());
    kept_node_refs.back() = &(node_list.back());

    // simplify the node list
    if (node_list.front() == node_list.back() || m_treat_as_rings_way.count(way_id) == 1) {
        // the way is a ring
        simplify_closed_ring(node_list, kept_node_refs);
    } else {
        simplify_node_list(node_list, kept_node_refs, 0, node_list.size() - 1);
    }

    // add node references to the to final object
    for (size_t i = 0; i != kept_node_refs.size() - 1; ++i) {
        if (kept_node_refs.at(i)) {
            // find next non-NULL node reference
            for (size_t j = i+1; j != kept_node_refs.size(); ++j) {
                if (kept_node_refs.at(j)) {
                    m_segments.emplace_back(kept_node_refs.at(i)->location(), kept_node_refs.at(j)->location(), way_id, i, j);
                    i = j - 1; // set forward. To compensate the auto-increment of the for loop, we have to decrement here by 1.
                    break;
                }
            }
        }
    }
}

void WaySimplifyHandler::way(const osmium::Way& way) {
    // write ways with less than four nodes directly to the output file
    if (way.nodes().size() <= 3 || (way.nodes().size() == 4 && (way.nodes().front() == way.nodes().back()))) {
        // add segments to segments vector
        for (osmium::WayNodeList::const_iterator it = way.nodes().cbegin(); it != way.nodes().cend() - 1; ++it) {
            m_segments.emplace_back(it->location(), (it + 1)->location(), way.id(), it - way.nodes().cbegin(),
                    (it + 1) - way.nodes().cbegin());
        }
    } else  {
        add_simplified_node_list(way.nodes(), way.id());
    }
}
