/*
 * admin_shp_handler.cpp
 *
 *  Created on: 25.01.2022
 *      Author: Michael Reichert
 */

#include <iostream>
#include "admin_shp_handler.hpp"

AdminSHPHandler::AdminSHPHandler(WayAdminLevelIndex& al_index, std::string& outfile,
        WayAdminLevelIndex::AdminLevel max_level) :
    m_al_index(al_index),
    m_max_level(max_level),
	m_dataset("ESRI Shapefile", outfile, gdalcpp::SRS{"+proj=longlat +datum=WGS84 +no_defs"}, {}),
	m_layer(m_dataset, get_layer_name(outfile), wkbLineString) {
	m_layer.add_field("osm_id", OFTString, 10);
	m_layer.add_field("level", OFTInteger, 10);
}

std::string AdminSHPHandler::get_layer_name(const std::string& path) {
	size_t pos = path.find_last_of('/');
	if (pos == std::string::npos) {
		std::cerr << "ERROR: Output path ends with a slash.\n";
		exit(1);
	}
	std::string f = path.substr(pos + 1);
	pos = f.find_last_of('.');
	if (pos == std::string::npos) {
		std::cerr << "ERROR: File name must end with '.shp' but it does not have a suffix.\n";
		exit(1);
	}
	std::string suffix = f.substr(pos + 1);
	if (suffix == "shp") {
		return f.substr(0, pos);
	}
	std::cerr << "ERROR: File name must end with '.shp' but it has a different suffix.\n";
	exit(1);
}

void AdminSHPHandler::way(const osmium::Way& way) {
    WayAdminLevelIndex::AdminLevel level = m_al_index.get(way.id());
    if (level == WayAdminLevelIndex::NO_LEVEL || level > m_max_level) {
    	return;
    }
    try {
		gdalcpp::Feature feature{m_layer, m_factory.create_linestring(way)};
		static char idbuffer[20];
		sprintf(idbuffer, "%ld", way.id());
		feature.set_field("osm_id", idbuffer);
		feature.set_field("level", level);
		feature.add_to_layer();
    } catch (const osmium::geometry_error&) {
    	std::cerr << "Ignoring illegal geometry for way " << way.id() << ".\n";
    }
}

