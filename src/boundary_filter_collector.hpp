/*
 * boundar_filter_collector.hpp
 *
 *  Created on:  2017-02-07
 *      Author: Michael Reichert
 */

#ifndef SRC_BOUNDARY_FILTER_COLLECTOR_HPP_
#define SRC_BOUNDARY_FILTER_COLLECTOR_HPP_

#include <functional>
#include <vector>
#include <osmium/relations/collector.hpp>
#include <osmium/object_pointer_collection.hpp>
#include <osmium/builder/osm_object_builder.hpp>
#include <osmium/io/writer.hpp>

class BoundaryFilterCollector : public osmium::relations::Collector<BoundaryFilterCollector,
true, true, true> {

    osmium::memory::Buffer m_output_buffer;
    std::string& m_out_filename;

    /**
     * Write changset which modified the relation to the output file.
     */
    bool m_changeset;

    /**
     * Write last_modified of the relation to the output file.
     */
    bool m_lastchange;

    /**
     * Write version of the relation to the output file.
     */
    bool m_version;

    /**
     * Write member nodes of the ways to the output buffer as nodes.
     */
    bool m_add_nodes;

    /**
     * Write the relation to the output file.
     */
    bool m_add_relations;

    /**
     * The functions in this vector determine whether a relation is interesting or not.
     */
    std::vector<std::function<bool (const osmium::TagList &)>>& m_is_interesting_functions;

    double m_maxlength = 0;
    osmium::object_id_type m_maxid = 0;

    void sort_buffer_and_write_it(osmium::io::Writer& writer);

    /** Helper to retrieve relation member */
    osmium::Way& get_member_way(size_t offset) const;

    /**
     * \brief Add a relation to the output buffer, but remove all node and relation members.
     */
    void add_relation_to_buffer(const osmium::Relation& original_relation);

    /**
     * \brief Add tags to a relation into a buffer.
     */
    void add_tags(osmium::builder::Builder* builder, const osmium::TagList& tags);

    /**
     * \brief Add members to a relation into a buffer.
     */
    void add_relation_members(osmium::builder::Builder* builder, const osmium::RelationMemberList& members);

    /**
     * \brief Add a node to the output buffer.
     */
    void build_node(const osmium::NodeRef& node_ref);

public:
    BoundaryFilterCollector() = delete;

    BoundaryFilterCollector(std::string& out_filename, std::vector<std::function<bool (const osmium::TagList &)>>& checks,
            bool changeset, bool lastchange, bool version, bool add_nodes, bool add_relations);

    /**
     * This method decides which relations we're interested in, and
     * instructs Osmium to collect their members for us.
     */
    bool keep_relation(const osmium::Relation& relation) const;

    /**
     * Tells Osmium which members to keep for a relation of interest.
     */
    bool keep_member(const osmium::relations::RelationMeta& relation_meta, const osmium::RelationMember& member) const;

    void complete_relation(osmium::relations::RelationMeta& relation_meta);

    void write_to_file();
};


#endif /* SRC_BOUNDARY_FILTER_COLLECTOR_HPP_ */
