/*
 * util.hpp
 *
 *  Created on: 2016-11-21
 *      Author: Michael Reichert
 */

#ifndef TEST_INCLUDE_UTIL_HPP_
#define TEST_INCLUDE_UTIL_HPP_

#include <iostream>
#include <assert.h>
#include <osmium/memory/buffer.hpp>
#include <osmium/builder/osm_object_builder.hpp>
#include <osmium/io/file.hpp>
#include <way_simplify_handler.hpp>

namespace test_utils {
    /**
     * \brief Build a WayNodeList
     */
    void add_node_refs(osmium::memory::Buffer& buffer, osmium::builder::WayBuilder* way_builder, std::vector<osmium::object_id_type> refs,
            std::vector<osmium::Location> locations) {
        osmium::builder::WayNodeListBuilder wnl_builder{buffer, way_builder};
        assert(refs.size() == locations.size());
        for (size_t i = 0; i < refs.size(); ++i) {
            wnl_builder.add_node_ref(osmium::NodeRef(refs.at(i), locations.at(i)));
        }
    }

    /**
     * \brief Count how many members of a vector a no nullptr.
     */
    template <class T>
    size_t count_non_nullptr_elements(std::vector<T*> vec) {
        size_t result = 0;
        for (T* element : vec) {
            if (element != nullptr) {
                result++;
            }
        }
        return result;
    }

} // namespace test_utils

namespace test_douglas_peucker{
    void simplify_node_list(const osmium::WayNodeList& node_list, std::vector<const osmium::NodeRef*>& kept_node_refs,
        size_t segment_start_offset, size_t segment_end_offset) {
        osmium::io::File outfile ("/tmp/test.osm");
        std::vector<BoundarySegment> segments;
        std::unordered_set<osmium::object_id_type> treat_as_rings_way;
        WaySimplifyHandler handler (75, segments, treat_as_rings_way);
        handler.simplify_node_list(node_list, kept_node_refs, segment_start_offset, segment_end_offset);
    }
    void simplify_node_list_area(const osmium::WayNodeList& node_list, std::vector<const osmium::NodeRef*>& kept_node_refs) {
        osmium::io::File outfile ("/tmp/test.osm");
        std::vector<BoundarySegment> segments;
        std::unordered_set<osmium::object_id_type> treat_as_rings_way;
        WaySimplifyHandler handler (75, segments, treat_as_rings_way);
        handler.simplify_node_list(node_list, kept_node_refs, 0, node_list.size() - 1);
    }

    void build_way(osmium::memory::Buffer& buffer, std::vector<osmium::object_id_type>& refs, std::vector<osmium::Location>& node_locations) {
        {
            osmium::builder::WayBuilder way_builder(buffer);
            osmium::Way& way = static_cast<osmium::Way&>(way_builder.object());
            way.set_id(1);
            way_builder.set_user("");
            test_utils::add_node_refs(buffer, &way_builder, refs, node_locations);
        }
        buffer.commit();
    }
} // namespace test_douglas_peucker

#endif /* TEST_INCLUDE_UTIL_HPP_ */
