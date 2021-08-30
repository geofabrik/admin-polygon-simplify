/*
 * admin_rel_handlers.cpp
 *
 *  Created on:  2021-08-19
 *      Author: Michael Reichert <michael.reichert@geofabrik.de>
 */

#include <iostream>
#include <osmium/builder/osm_object_builder.hpp>
#include "admin_rel_handlers.hpp"

WayAdminLevelIndex::AdminLevel AdminRelHandler1::check(const osmium::TagList& tags) {
    const char* type = tags.get_value_by_key("type", "");
    const char* boundary = tags.get_value_by_key("boundary", "");
    if (!strcmp(type, "boundary") || !strcmp(type, "multipolygon")) {
        if (!strcmp(boundary, "administrative")) {
            const char* admin_level = tags.get_value_by_key("admin_level");
            WayAdminLevelIndex::AdminLevel level = WayAdminLevelIndex::parse_admin_level(admin_level);
            if (2 <= level && level <= m_max_level) {
                return level;
            }
        }
    }
    return WayAdminLevelIndex::NO_LEVEL;
}

AdminRelHandler1::AdminRelHandler1(WayAdminLevelIndex& index, const int max_level) :
    m_al_index(index),
    m_max_level(max_level) {}

void AdminRelHandler1::relation(const osmium::Relation& relation) {
    WayAdminLevelIndex::AdminLevel level = check(relation.tags());
    if (level == WayAdminLevelIndex::NO_LEVEL) {
        return;
    }
    for (const auto& member : relation.members()) {
        if (member.type() == osmium::item_type::way) {
            m_al_index.store(member.ref(), level);
        }
    }
}

osmium::memory::Buffer AdminRelHandler2::init_buffer() {
    return osmium::memory::Buffer(1024*1024, osmium::memory::Buffer::auto_grow::yes);
}

void AdminRelHandler2::write_object_unchanged(const osmium::OSMObject& obj) {
    m_buffer.add_item(obj);
    m_buffer.commit();
}

void AdminRelHandler2::edit_way(const osmium::Way& way, const WayAdminLevelIndex::AdminLevel level) {
    osmium::builder::WayBuilder way_builder(m_buffer);
    osmium::Way& new_way = static_cast<osmium::Way&>(way_builder.object());
    new_way.set_id(way.id());
    if (way.changeset()) {
        new_way.set_changeset(way.changeset());
    }
    if (way.uid()) {
        new_way.set_uid(way.uid());
    }
    new_way.set_version(way.version());
    new_way.set_visible(way.visible());
    new_way.set_timestamp(way.timestamp());
    if (way.user()) {
        way_builder.set_user(way.user());
    }
    {
        osmium::builder::WayNodeListBuilder wnl_builder{m_buffer, &way_builder};
        for (const auto& nd : way.nodes()) {
            wnl_builder.add_node_ref(nd);
        }
    }
    {
        osmium::builder::TagListBuilder tl_builder(m_buffer, &way_builder);
        for (const auto& t : way.tags()) {
            if (strcmp(t.key(), "admin_level") && strcmp(t.key(), "boundary")) {
                tl_builder.add_tag(t);
            }
        }
        if (level != WayAdminLevelIndex::NO_LEVEL) {
            char admin_level[10] = "";
            snprintf(admin_level, 10, "%hu", level);
            tl_builder.add_tag("admin_level", admin_level);
            tl_builder.add_tag("boundary", "administrative");
        }
    }
}

void AdminRelHandler2::flush_ways() {
    m_writer(std::move(m_buffer));
    m_buffer = init_buffer();
}

AdminRelHandler2::AdminRelHandler2(WayAdminLevelIndex& al_index, osmium::io::File& outfile,
        const osmium::io::Header& header, WayAdminLevelIndex::AdminLevel max_level) :
    m_al_index(al_index),
    m_writer(outfile, header, osmium::io::overwrite::allow),
    m_buffer(init_buffer()),
    m_max_level(max_level) {}

AdminRelHandler2::~AdminRelHandler2() {
    m_writer.flush();
    m_writer.close();
}

void AdminRelHandler2::node(const osmium::Node& node) {
    m_writer(node);
}

void AdminRelHandler2::way(const osmium::Way& way) {
    WayAdminLevelIndex::AdminLevel level = m_al_index.get(way.id());
    const char* admin_level_old = way.get_value_by_key("admin_level");
    WayAdminLevelIndex::AdminLevel level_old = m_al_index.parse_admin_level(admin_level_old);
    if ((level == WayAdminLevelIndex::NO_LEVEL && level_old == WayAdminLevelIndex::NO_LEVEL)
            || level == level_old
            || level_old > m_max_level) {
        // We flush the buffer every 100 MB only.
        if (m_buffer.committed() > 1024 * 1024 * 100) {
            flush_ways();
        }
        write_object_unchanged(way);
    } else{
        edit_way(way, level);
        m_buffer.commit();
        // We flush the buffer every 100 MB only.
        if (m_buffer.committed() > 1024 * 1024 * 100) {
            flush_ways();
        }
    }
}

void AdminRelHandler2::relation(const osmium::Relation& relation) {
    // Flush way buffer if it still contains ways.
    if (!m_seen_relation && m_buffer.committed() > 0) {
        flush_ways();
        m_seen_relation = true;
    }
    m_writer(relation);
}


