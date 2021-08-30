/*
 * way_simplify_handler2.cpp
 *
 *  Created on:  2017-02-14
 *      Author: Michael Reichert
 */

#include "way_simplify_handler2.hpp"
#include <iostream>
#include <memory>
#include <osmium/io/output_iterator.hpp>
#include <osmium/object_pointer_collection.hpp>
#include <osmium/osm/object_comparisons.hpp>

WaySimplifyHandler2::~WaySimplifyHandler2() {
    m_writer.close();
}

WaySimplifyHandler2::WaySimplifyHandler2(osmium::io::File& outfile, double epsilon,
        const osmium::io::Header& header, ErrorsMap& error_segments, KeepNodesMap& keep_nodes,
        std::unordered_set<osmium::object_id_type>& treat_as_rings_way) :
        AbstractWaySimplifier(epsilon),
        m_writer(outfile, header, osmium::io::overwrite::allow),
        m_error_segments(error_segments),
        m_kept_nodes(keep_nodes),
        m_treat_as_rings_way(treat_as_rings_way),
        m_output_buffer(1024*1024, osmium::memory::Buffer::auto_grow::yes) { }

void WaySimplifyHandler2::add_tags(osmium::memory::Buffer& buffer, osmium::builder::Builder* builder, const osmium::TagList& tags) {
    osmium::builder::TagListBuilder tl_builder(buffer, builder);
    for (const osmium::Tag& t : tags) {
        tl_builder.add_tag(t);
    }
}

void WaySimplifyHandler2::relation(const osmium::Relation& relation) {
    if (!m_reached_relations) {
        sort_buffer_and_write_it();
        m_reached_relations = true;
    }
    // We do not have to sort the relations because they are sorted in the input file.
    // Therefore we can write them directly to the disc.
    m_writer(relation);
    m_writer.flush();
}

void WaySimplifyHandler2::way(const osmium::Way& way) {
    // write ways with less than four nodes directly to the output file
    if (way.nodes().size() <= 3 || (way.nodes().size() == 4 && (way.nodes().front() == way.nodes().back()))) {
        m_output_buffer.add_item(way);
        m_output_buffer.commit();
        for (const osmium::NodeRef& nd_ref : way.nodes()) {
            add_node_to_buffer(&nd_ref);
        }
        m_output_buffer.commit();
    } else  {
        // vector for kept node references; entrys which are nullptr mean that we discard these nodes
        std::vector<const osmium::NodeRef*> kept_node_refs {way.nodes().size(), nullptr};
        {
            osmium::builder::WayBuilder way_builder(m_output_buffer);
            osmium::Way& new_way = static_cast<osmium::Way&>(way_builder.object());
            new_way.set_id(way.id());
            new_way.set_changeset(way.changeset());
            new_way.set_uid(way.uid());
            new_way.set_version(way.version());
            new_way.set_visible(way.visible());
            new_way.set_timestamp(way.timestamp());
            way_builder.set_user(way.user());
            add_simplified_node_list(m_output_buffer, &way_builder, way.nodes(), way.id(), kept_node_refs);
            add_tags(m_output_buffer, &way_builder, way.tags());
        }
        m_output_buffer.commit();
        // Write every node to the node buffer
        for (const osmium::NodeRef* nd_ref : kept_node_refs) {
            if (nd_ref) {
                add_node_to_buffer(nd_ref);
            }
        }
        m_output_buffer.commit();
    }
}

void WaySimplifyHandler2::add_kept_nodes_to_list(const osmium::WayNodeList& nodes,
        const osmium::object_id_type way_id, std::vector<const osmium::NodeRef*>& kept_nodes) {
    std::pair<KeepNodesMap::iterator, KeepNodesMap::iterator> it_range = m_kept_nodes.equal_range(way_id);
    for (KeepNodesMap::iterator it = it_range.first; it != it_range.second; it++) {
        kept_nodes.at(it->second) = &(nodes[it->second]);
    }
}

void WaySimplifyHandler2::add_simplified_node_list(osmium::memory::Buffer& buffer, osmium::builder::Builder* builder,
        const osmium::WayNodeList& node_list, osmium::object_id_type way_id, std::vector<const osmium::NodeRef*>& kept_node_refs) {
    // add nodes which are preserved to prevent intersections
    add_kept_nodes_to_list(node_list, way_id, kept_node_refs);

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
    osmium::builder::WayNodeListBuilder wnl_builder{buffer, builder};
    for (size_t i = 0; i != kept_node_refs.size(); ++i) {
        if (kept_node_refs.at(i)) {
            wnl_builder.add_node_ref(*(kept_node_refs.at(i)));
        }
    }
}

void WaySimplifyHandler2::add_all_nodes(osmium::memory::Buffer& buffer, osmium::builder::Builder* builder,
        const osmium::WayNodeList& node_list) {
    osmium::builder::WayNodeListBuilder wnl_builder{buffer, builder};
    for (const osmium::NodeRef& nd_ref : node_list) {
        wnl_builder.add_node_ref(nd_ref);
    }
}

void WaySimplifyHandler2::add_node_to_buffer(const osmium::NodeRef* nd_ref) {
    osmium::builder::NodeBuilder builder(m_output_buffer);
    osmium::Node& node = static_cast<osmium::Node&>(builder.object());
    node.set_id(nd_ref->ref());
    // otherwise the resulting OSM file does not contain the visible=true attribute and some programs behave strange
    node.set_visible(true);
    builder.set_user("");
    node.set_location(nd_ref->location());
}

void WaySimplifyHandler2::sort_buffer_and_write_it() {
    auto out = osmium::io::make_output_iterator(m_writer);
    osmium::ObjectPointerCollection nodes;
    osmium::apply(m_output_buffer, nodes);
    nodes.sort(osmium::object_order_type_id_reverse_version());
    // std::copy (i.e. copy without comparing the objects) does not work. Nodes with tags beyond the
    // bounding box will not be written to the output file.
    std::unique_copy(nodes.cbegin(), nodes.cend(), out, osmium::object_equal_type_id());
}
