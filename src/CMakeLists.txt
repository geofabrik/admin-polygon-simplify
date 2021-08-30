#-----------------------------------------------------------------------------
#
#  CMake Config
#
#-----------------------------------------------------------------------------

add_executable(osm_adminfilter osm_adminfilter.cpp boundary_filter_collector.cpp)
target_link_libraries(osm_adminfilter ${OSMIUM_LIBRARIES} ${Boost_LIBRARIES})
install(TARGETS osm_adminfilter DESTINATION bin)

add_executable(admin_polygon_simplify admin_polygon_simplify.cpp way_simplify_handler.cpp distance_sphere_plain.cpp vector3d.cpp boundary_segment.cpp way_simplify_handler2.cpp abstract_way_simplifier.cpp intermediate_simplifier.cpp boundary_relation_collector.cpp)
target_link_libraries(admin_polygon_simplify ${OSMIUM_LIBRARIES} ${Boost_LIBRARIES})
install(TARGETS admin_polygon_simplify DESTINATION bin)

add_executable(osm_admin_level_rels2ways osm_admin_level_rels2ways.cpp way_admin_level_index.cpp admin_rel_handlers.cpp)
target_link_libraries(osm_admin_level_rels2ways ${OSMIUM_LIBRARIES} ${Boost_LIBRARIES})
install(TARGETS osm_admin_level_rels2ways DESTINATION bin)
