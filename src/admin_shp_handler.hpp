/*
 * admin_shp_handler.hpp
 *
 *  Created on: 25.01.2022
 *      Author: michael
 */

#ifndef SRC_ADMIN_SHP_HANDLER_HPP_
#define SRC_ADMIN_SHP_HANDLER_HPP_

#include <gdalcpp.hpp>
#include <osmium/handler.hpp>
#include <osmium/geom/ogr.hpp>
#include "way_admin_level_index.hpp"

class AdminSHPHandler : public osmium::handler::Handler {
    WayAdminLevelIndex& m_al_index;
    WayAdminLevelIndex::AdminLevel m_max_level;
    osmium::geom::OGRFactory<> m_factory {};
    gdalcpp::Dataset m_dataset;
    gdalcpp::Layer m_layer;
    bool m_seen_relation = false;

    /**
     * Return file name component of a path except file name suffix.
     */
    static std::string get_layer_name(const std::string& path);

public:
    explicit AdminSHPHandler(WayAdminLevelIndex& al_index, std::string& outfile,
            WayAdminLevelIndex::AdminLevel max_level);

    void way(const osmium::Way& way);
};



#endif /* SRC_ADMIN_SHP_HANDLER_HPP_ */
