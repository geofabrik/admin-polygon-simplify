/*
 * way_simplify_handler2.hpp
 *
 *  Created on:  2017-02-14
 *      Author: Michael Reichert
 */

#ifndef SRC_WAY_SIMPLIFY_HANDLER2_HPP_
#define SRC_WAY_SIMPLIFY_HANDLER2_HPP_

#include <unordered_set>
#include <osmium/io/writer.hpp>
#include <osmium/io/any_output.hpp>
#include <osmium/builder/osm_object_builder.hpp>
#include "abstract_way_simplifier.hpp"
#include "no_simplify_segment.hpp"

class WaySimplifyHandler2 : public AbstractWaySimplifier {
    osmium::io::Writer m_writer;
    ErrorsMap& m_error_segments;
    KeepNodesMap& m_kept_nodes;
    std::unordered_set<osmium::object_id_type>& m_treat_as_rings_way;
    osmium::memory::Buffer m_output_buffer;

    void add_kept_nodes_to_list(const osmium::WayNodeList& nodes, const osmium::object_id_type way_id,
            std::vector<const osmium::NodeRef*>& kept_nodes);

    void add_all_nodes(osmium::memory::Buffer& buffer, osmium::builder::Builder* builder,
            const osmium::WayNodeList& node_list);

    void add_tags(osmium::memory::Buffer& buffer, osmium::builder::Builder* builder, const osmium::TagList& tags);

    /**
     * \brief add this node to the output buffer
     *
     * Don't forget to commit after calling this method.
     */
    void add_node_to_buffer(const osmium::NodeRef* nd_ref);

    /***
     * \brief Sort the nodes and ways in the output buffer by item type, ID and version.
     *
     * Duplicated items will be written only one time.
     */
    void sort_buffer_and_write_it();

public:
    WaySimplifyHandler2(osmium::io::File& outfile, double epsilon,
            const osmium::io::Header& header, ErrorsMap& error_segments, KeepNodesMap& keep_nodes,
            std::unordered_set<osmium::object_id_type>& treat_as_rings_way);

    ~WaySimplifyHandler2();

    void way(const osmium::Way& way);

    void relation(const osmium::Relation& relation);

    /**
     * Simplify a way but keeping essential nodes to prevent intersections.
     *
     * \param buffer output buffer
     * \param builder Osmium object builder
     * \param node_list node list of the way
     * \param way_id ID of the way
     *
     * \returns list of nodes which have not been removed by the simplification
     */
    void add_simplified_node_list(osmium::memory::Buffer& buffer, osmium::builder::Builder* builder,
            const osmium::WayNodeList& node_list, osmium::object_id_type way_id, std::vector<const osmium::NodeRef*>& kept_node_refs);
};



#endif /* SRC_WAY_SIMPLIFY_HANDLER2_HPP_ */
