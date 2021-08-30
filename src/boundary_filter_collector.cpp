/*
 * boundary_filter_collector.cpp
 *
 *  Created on:  2017-02-07
 *      Author: Michael Reichert
 */

#include <osmium/osm/item_type.hpp>
#include <osmium/io/any_output.hpp>
#include <osmium/io/output_iterator.hpp>
#include <osmium/osm/object_comparisons.hpp>
#include <osmium/visitor.hpp>
#include "boundary_filter_collector.hpp"

BoundaryFilterCollector::BoundaryFilterCollector(std::string& out_filename,
        std::vector<std::function<bool (const osmium::TagList &)>>& checks,
        bool changset, bool lastchange, bool version, bool add_nodes,
        bool add_relations) :
        m_output_buffer(1024*1024, osmium::memory::Buffer::auto_grow::yes),
        m_out_filename(out_filename),
        m_changeset(changset),
        m_lastchange(lastchange),
        m_version(version),
        m_add_nodes(add_nodes),
        m_add_relations(add_relations),
        m_is_interesting_functions(checks) { }


bool BoundaryFilterCollector::keep_relation(const osmium::Relation& relation) const {
    for (std::function<bool (const osmium::TagList &)>& f : m_is_interesting_functions) {
        if (f(relation.tags())) {
            return true;
        }
    }
    return false;
}

bool BoundaryFilterCollector::keep_member(const osmium::relations::RelationMeta& relation_meta,
        const osmium::RelationMember& member) const {
    if (member.type() == osmium::item_type::way) {
        return true;
    }
    return false;
}

osmium::Way& BoundaryFilterCollector::get_member_way(size_t offset) const {
    return static_cast<osmium::Way &>(this->get_member(offset));
}

void BoundaryFilterCollector::add_relation_to_buffer(const osmium::Relation& original_relation) {
    osmium::builder::RelationBuilder relation_builder(m_output_buffer);
    osmium::Relation& modified_relation = static_cast<osmium::Relation&>(relation_builder.object());
    modified_relation.set_id(original_relation.id());
    if (m_version) {
        modified_relation.set_version(original_relation.version());
    }
    if (m_lastchange) {
        modified_relation.set_timestamp(original_relation.timestamp());
    }
    if (m_changeset) {
        modified_relation.set_changeset(original_relation.changeset());
    }
    relation_builder.set_user(original_relation.user());
    add_relation_members(&relation_builder, original_relation.members());
    add_tags(&relation_builder, original_relation.tags());
}

void BoundaryFilterCollector::add_tags(osmium::builder::Builder* builder, const osmium::TagList& tags) {
    osmium::builder::TagListBuilder tl_builder(m_output_buffer, builder);
    for (const osmium::Tag& tag : tags) {
        tl_builder.add_tag(tag);
    }
}

void BoundaryFilterCollector::add_relation_members(osmium::builder::Builder* builder, const osmium::RelationMemberList& members) {
    osmium::builder::RelationMemberListBuilder rml_builder(m_output_buffer, builder);
    for (const osmium::RelationMember& member : members) {
        if (member.type() == osmium::item_type::way) {
            rml_builder.add_member(member.type(), member.ref(), member.role());
        }
    }
}

void BoundaryFilterCollector::build_node(const osmium::NodeRef& node_ref) {
    osmium::builder::NodeBuilder node_builder(m_output_buffer);
    osmium::Node& new_node = static_cast<osmium::Node&>(node_builder.object());
    new_node.set_id(node_ref.ref());
    new_node.set_location(node_ref.location());
    node_builder.set_user("");
}

void BoundaryFilterCollector::complete_relation(osmium::relations::RelationMeta& relation_meta) {
    const osmium::Relation& relation = this->get_relation(relation_meta);
    bool valid_relation = false;
    for (auto& member : relation.members()) {
        // throw away everything which is not a way
        if (member.type() != osmium::item_type::way) {
            continue;
        }
        std::pair<bool, size_t> avail_offset = get_availability_and_offset(member.type(), member.ref());
        if (avail_offset.first) {
            osmium::Way& way = this->get_member_way(avail_offset.second);
            // add to object pointer collection
            {
                osmium::builder::WayBuilder way_builder(m_output_buffer);
                osmium::Way& new_way = static_cast<osmium::Way&>(way_builder.object());
                new_way.set_id(way.id());
                if (m_changeset) {
                    new_way.set_changeset(way.changeset());
                }
                new_way.set_uid(way.uid());
                if (m_version) {
                    new_way.set_version(way.version());
                }
                new_way.set_visible(way.visible());
                if (m_lastchange) {
                    new_way.set_timestamp(way.timestamp());
                }
                way_builder.set_user(way.user());
                {
                    osmium::builder::WayNodeListBuilder wnl_builder{m_output_buffer, &way_builder};
                    for (const osmium::NodeRef& nd_ref : way.nodes()) {
                        wnl_builder.add_node_ref(nd_ref);
                    }
                }
                add_tags(&way_builder, way.tags());
                // We don't copy tags.
            }
            m_output_buffer.commit();
            valid_relation = true;
            if (m_add_nodes) {
                for (const osmium::NodeRef& nd : way.nodes()) {
                    build_node(nd);
                    m_output_buffer.commit();
                }
            }
        }
        m_output_buffer.commit();
    }
    if (valid_relation && m_add_relations) {
        // We cannot simply add the relation to the output buffer because it contains references to objects which
        // do not exist in the input buffer and reference IDs which are 0 which is illegal in OSM XML and PBF.
        add_relation_to_buffer(relation);
    }
    m_output_buffer.commit();
}

void BoundaryFilterCollector::write_to_file() {
    osmium::io::Header header;
    header.set("generator", "osm_adminfilter");
    header.set("copyright", "OpenStreetMap and contributors");
    header.set("attribution", "http://www.openstreetmap.org/copyright");
    header.set("license", "http://opendatacommons.org/licenses/odbl/1-0/");
    osmium::io::File output_file{m_out_filename};
    output_file.set("locations_on_ways", true);
    osmium::io::Writer writer{output_file, header};
    // We have to merge the buffers and sort the objects. Therefore first all nodes are written, then all ways and as last step
    // all relations.
    sort_buffer_and_write_it(writer);
    writer.close();
}

void BoundaryFilterCollector::sort_buffer_and_write_it(osmium::io::Writer& writer) {
    auto out = osmium::io::make_output_iterator(writer);
    osmium::ObjectPointerCollection objects;
    osmium::apply(m_output_buffer, objects);
    objects.sort(osmium::object_order_type_id_reverse_version());
    std::unique_copy(objects.cbegin(), objects.cend(), out, osmium::object_equal_type_id());
}

