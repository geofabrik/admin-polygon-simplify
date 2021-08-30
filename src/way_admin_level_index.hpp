/*
 * way_admin_level_index.hpp
 *
 *  Created on:  2021-08-18
 *      Author: Michael Reichert <michael.reichert@geofabrik.de>
 */

#ifndef SRC_WAY_ADMIN_LEVEL_INDEX_HPP_
#define SRC_WAY_ADMIN_LEVEL_INDEX_HPP_

#include <vector>
#include <osmium/osm/types.hpp>

class WayAdminLevelIndex {

    using IndexEntry = std::pair<osmium::object_id_type, uint8_t>;
    std::vector<IndexEntry> m_way_idx;

public:
    using AdminLevel = uint8_t;
    static constexpr AdminLevel NO_LEVEL = 0;

    /**
     * Add way ID and the admin level of the relation using it to the index.
     * If the way is already present in the index, update the admin_level if the stored value is
     * larger than the new value.
     */
    void store(const osmium::object_id_type id, const AdminLevel admin_level);

    /**
     * Prepare index for querying by sorting it and removing duplicates.
     */
    void prepare_for_query();

    /**
     * Get admin level stored in the index.
     */
    AdminLevel get(const osmium::object_id_type id, const AdminLevel fallback = NO_LEVEL);

    size_t size();

    /**
     * Parse value of admin_level key from string. Returns 0 for failures.
     */
    static AdminLevel parse_admin_level(const char* admin_level);
};



#endif /* SRC_WAY_ADMIN_LEVEL_INDEX_HPP_ */
