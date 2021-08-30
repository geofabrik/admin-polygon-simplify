enable_testing()

message(STATUS "Configuring unit tests")

include(CMakeParseArguments)
include_directories(include)
include_directories(../src)

add_library(testlib STATIC test_main.cpp)

set(ALL_TESTS "")

# Otherwise GCC throws a lot of warnings for REQUIRE(...) from Catch v.1.2.1
if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(-Wno-parentheses)
endif()


add_executable(test_distance_sphere t/test_distance_sphere.cpp ../src/distance_sphere_plain.cpp ../src/vector3d.cpp)
target_link_libraries(test_distance_sphere testlib)
add_test(NAME test_distance_sphere
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND test_distance_sphere)

add_executable(test_douglas_peucker_nonclosed t/test_douglas_peucker_nonclosed.cpp ../src/distance_sphere_plain.cpp ../src/vector3d.cpp ../src/way_simplify_handler.cpp ../src/abstract_way_simplifier.cpp ../src/boundary_segment.cpp)
target_link_libraries(test_douglas_peucker_nonclosed testlib ${OSMIUM_LIBRARIES})
add_test(NAME test_douglas_peucker_nonclosed
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND test_douglas_peucker_nonclosed)

add_executable(test_douglas_peucker_closed t/test_douglas_peucker_closed.cpp ../src/distance_sphere_plain.cpp ../src/vector3d.cpp ../src/way_simplify_handler.cpp ../src/abstract_way_simplifier.cpp ../src/boundary_segment.cpp)
target_link_libraries(test_douglas_peucker_closed testlib ${OSMIUM_LIBRARIES})
add_test(NAME test_douglas_peucker_closed
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND test_douglas_peucker_closed)

add_executable(test_intersection t/test_intersection.cpp ../src/intermediate_simplifier.cpp ../src/boundary_segment.cpp ../src/abstract_way_simplifier.cpp ../src/distance_sphere_plain.cpp ../src/vector3d.cpp)
target_link_libraries(test_intersection testlib ${OSMIUM_LIBRARIES})
add_test(NAME test_intersection
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND test_intersection)
