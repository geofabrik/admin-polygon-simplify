/*
 * way_simplify_handler.hpp
 *
 *  Created on:  2017-02-08
 *      Author: Michael Reichert
 */

#ifndef SRC_WAY_SIMPLIFY_HANDLER_HPP_
#define SRC_WAY_SIMPLIFY_HANDLER_HPP_

#include <unordered_set>
#include "boundary_segment.hpp"
#include "abstract_way_simplifier.hpp"

#include "distance_sphere_plain.hpp"

class WaySimplifyHandler : public AbstractWaySimplifier {
protected:
    std::vector<BoundarySegment>& m_segments;

    std::unordered_set<osmium::object_id_type>& m_treat_as_rings_way;

    /**
     * ID the next node reference we will write to the output file will get
     */
    osmium::object_id_type m_next_way_id = 1;

    void add_simplified_node_list(const osmium::WayNodeList& node_list, osmium::object_id_type way_id);

public:
    WaySimplifyHandler(double epsilon, std::vector<BoundarySegment>& segments,
            std::unordered_set<osmium::object_id_type>& treat_as_rings_way);

    void way(const osmium::Way& way);
};


#endif /* SRC_WAY_SIMPLIFY_HANDLER_HPP_ */
