/*
 * admin_rel_handlers.hpp
 *
 *  Created on:  2021-08-19
 *      Author: Michael Reichert <michael.reichert@geofabrik.de>
 */

#ifndef SRC_ADMIN_REL_HANDLERS_HPP_
#define SRC_ADMIN_REL_HANDLERS_HPP_

#include <osmium/handler.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/io/writer.hpp>
#include <osmium/io/any_output.hpp>
#include "way_admin_level_index.hpp"

class AdminRelHandler1 : public osmium::handler::Handler {
    WayAdminLevelIndex& m_al_index;
    const int m_max_level;

    WayAdminLevelIndex::AdminLevel check(const osmium::TagList& tags);

public:
    explicit AdminRelHandler1(WayAdminLevelIndex& index, const int max_level);

    void relation(const osmium::Relation& relation);
};

class AdminRelHandler2 : public osmium::handler::Handler {
    WayAdminLevelIndex& m_al_index;
    osmium::io::Writer m_writer;
    osmium::memory::Buffer m_buffer;
    WayAdminLevelIndex::AdminLevel m_max_level;
    bool m_seen_relation = false;

    static osmium::memory::Buffer init_buffer();

    void write_object_unchanged(const osmium::OSMObject& obj);

    void edit_way(const osmium::Way& way, const WayAdminLevelIndex::AdminLevel level);

    void flush_ways();

public:
    explicit AdminRelHandler2(WayAdminLevelIndex& al_index, osmium::io::File& outfile,
            const osmium::io::Header& header, WayAdminLevelIndex::AdminLevel max_level);

    ~AdminRelHandler2();

    void node(const osmium::Node& node);

    void way(const osmium::Way& way);

    void relation(const osmium::Relation& relation);
};



#endif /* SRC_ADMIN_REL_HANDLERS_HPP_ */
