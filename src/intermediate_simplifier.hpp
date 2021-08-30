/*
 * intermediate_simplifier.hpp
 *
 *  Created on:  2017-05-03
 *      Author: Michael Reichert <michael.reichert@geofabrik.de>
 */

#ifndef SRC_INTERMEDIATE_SIMPLIFIER_HPP_
#define SRC_INTERMEDIATE_SIMPLIFIER_HPP_

#include <unordered_map>
#include <osmium/util/verbose_output.hpp>
#include "abstract_way_simplifier.hpp"
#include "no_simplify_segment.hpp"

class IntermediateSimplifier : public AbstractWaySimplifier {
    ErrorsMap& m_error_segments;
    std::vector<BoundarySegment>& m_all_segments;
    KeepNodesMap& m_kept_nodes;
    osmium::util::VerboseOutput& m_vout;

    void improve_simplification(const osmium::Way& way);

    osmium::Location get_nearest_node_to_intersection(const osmium::Location& intersection,
            BoundarySegment& segment1, BoundarySegment& segment2);

    std::vector<NoSimplifySegment*> sort_no_simplify_segments(osmium::object_id_type way_id);

    bool outside_x_range(const osmium::UndirectedSegment& s1, const osmium::UndirectedSegment& s2);

    bool y_range_overlap(const osmium::UndirectedSegment& s1, const osmium::UndirectedSegment& s2);
public:
    IntermediateSimplifier(double epsilon, ErrorsMap& error_segments, std::vector<BoundarySegment>& all_segments,
            KeepNodesMap& keep_nodes, osmium::util::VerboseOutput& vout);

    void way(const osmium::Way& way);

    /**
     * Check if still intersections exist.
     */
    bool recheck_intersections();

    osmium::Location intersection(const osmium::Segment& s1, const osmium::Segment&s2);
};



#endif /* SRC_INTERMEDIATE_SIMPLIFIER_HPP_ */
