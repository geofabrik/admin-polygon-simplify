/*
 * abstract_way_simplifier.cpp
 *
 *  Created on:  2017-02-14
 *      Author: Michael Reichert
 */

#include "abstract_way_simplifier.hpp"
#include <assert.h>

AbstractWaySimplifier::AbstractWaySimplifier(double epsilon) :
        m_epsilon(epsilon) { }

void AbstractWaySimplifier::simplify_closed_ring(const osmium::WayNodeList& node_list, std::vector<const osmium::NodeRef*>& kept_node_refs) {
    assert(kept_node_refs.size() == node_list.size());
    // Keep the most distant and the second most distant node. The resulting area will not look nice if it
    // is only about as large as the maximum error or even smaller. But it will be valid!
    size_t vertex_max_distance = 1;
    size_t vertex_second_max_distance = 2;
    double largest_distance = 0;
    double second_largest_distance = 0;
    for (size_t i = 1; i < node_list.size() - 1 ; ++i) {
        double distance = m_distance_calculator.distance_from_line_sphere(node_list.front().location(),
                node_list.back().location(), node_list[i].location());
        if (distance > second_largest_distance) {
            if (distance > largest_distance) {
                second_largest_distance = largest_distance;
                vertex_second_max_distance = vertex_max_distance;
                vertex_max_distance = i;
                largest_distance = distance;
            } else {
                vertex_second_max_distance = i;
                second_largest_distance = distance;
            }
        }
    }
    if (vertex_max_distance > vertex_second_max_distance) {
        std::swap(vertex_max_distance, vertex_second_max_distance);
    }
    // The node with the largest distance will be kept.
    kept_node_refs.at(vertex_max_distance) = &node_list[vertex_max_distance];
    kept_node_refs.at(vertex_second_max_distance) = &node_list[vertex_second_max_distance];
    if (largest_distance < m_epsilon) {
        // The largest distance from the start node to any other node is smaller than the threshold.
        // The area becomes a triangle (start point–most distant point–end point).
        return;
    } else {
        // Add split point between the two subsegments to the vector of kept nodes.
        // All remaining work is done by the method for non-closed ways.
        simplify_node_list(node_list, kept_node_refs, 0, vertex_max_distance);
        simplify_node_list(node_list, kept_node_refs, vertex_max_distance, vertex_second_max_distance);
        simplify_node_list(node_list, kept_node_refs, vertex_second_max_distance, node_list.size() - 1);
    }
}

void AbstractWaySimplifier::simplify_node_list(const osmium::WayNodeList& node_list, std::vector<const osmium::NodeRef*>& kept_node_refs,
        size_t segment_start_offset, size_t segment_end_offset) {
    assert(segment_end_offset < node_list.size());
    if (segment_end_offset - segment_start_offset == 1) {
        // Shortcut: If the segment is only two nodes long, it cannot be simplified any more.
        return;
    }
    size_t vertex_max_distance = segment_end_offset;
    double largest_distance = 0;
    for (size_t i = segment_start_offset + 1; i < segment_end_offset ; ++i) {
        double distance = m_distance_calculator.distance_from_line_sphere(node_list[segment_start_offset].location(),
                node_list[segment_end_offset].location(), node_list[i].location());
        if (distance > largest_distance && distance > m_epsilon) {
            vertex_max_distance = i;
            largest_distance = distance;
        }
    }
    if (vertex_max_distance == segment_end_offset) {
        // Discard all inner nodes.
        // There is no vertex whose distance from the line is larger than epsilon.
        return;
    } else {
        // Add split point between the two subsegments to the vector of kept nodes.
        kept_node_refs.at(vertex_max_distance) = &node_list[vertex_max_distance];
        simplify_node_list(node_list, kept_node_refs, segment_start_offset, vertex_max_distance);
        simplify_node_list(node_list, kept_node_refs, vertex_max_distance, segment_end_offset);
    }
}

