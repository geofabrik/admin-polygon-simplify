/*
 * way_admin_level_index.cpp
 *
 *  Created on:  2021-08-18
 *      Author: Michael Reichert <michael.reichert@geofabrik.de>
 */

#include <algorithm>
#include <exception>
#include <sstream>
#include "way_admin_level_index.hpp"

void WayAdminLevelIndex::store(const osmium::object_id_type id, const AdminLevel admin_level) {
    m_way_idx.emplace_back(id, admin_level);
}

void WayAdminLevelIndex::prepare_for_query() {
    std::sort(
        m_way_idx.begin(),
        m_way_idx.end(),
        [](const std::pair<osmium::object_id_type, uint8_t>& lhs, const std::pair<osmium::object_id_type, uint8_t>& rhs) {
            return lhs.first < rhs.first || (lhs.first == rhs.first && lhs.second < rhs.second);
        }
    );
    // According to documentation, std::unique keeps the first element of a group of unique elements, i.e. the entry with
    // smallest admin_level among entries with equal OSM ID.
    std::unique(
        m_way_idx.begin(),
        m_way_idx.end(),
        [](const std::pair<osmium::object_id_type, uint8_t>& lhs, const std::pair<osmium::object_id_type, uint8_t>& rhs) {
            return lhs.first == rhs.first;
        }
    );
}

WayAdminLevelIndex::AdminLevel WayAdminLevelIndex::get(const osmium::object_id_type id, const AdminLevel fallback) {
    std::pair<osmium::object_id_type, uint8_t> p {id, fallback};
    auto it = std::lower_bound(
        m_way_idx.begin(),
        m_way_idx.end(),
        p,
        [](const std::pair<osmium::object_id_type, uint8_t>& lhs, const std::pair<osmium::object_id_type, uint8_t>& rhs) {
            return lhs.first < rhs.first;
        }
    );
    if (it != m_way_idx.end() && it->first == id) {
        return it->second;
    }
    return fallback;
}

size_t WayAdminLevelIndex::size() {
    return m_way_idx.size();
}

WayAdminLevelIndex::AdminLevel WayAdminLevelIndex::parse_admin_level(const char* admin_level) {
    if (!admin_level) {
        return NO_LEVEL;
    }
    char* ptr = nullptr;
    int al = strtod(admin_level, &ptr);
    if (al == 0 && ptr) {
        return NO_LEVEL;
    }
    if (al < 0 || al > 11) {
        return NO_LEVEL;
    }
    return static_cast<AdminLevel>(al);
}
