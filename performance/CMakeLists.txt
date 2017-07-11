project(performnance_tests)

set(SRCS  main.cpp)
include_directories("${CMAKE_SOURCE_DIR}")

add_executable(${PROJECT_NAME} main.cpp)
add_executable(binary_search binary_search.cpp)
add_executable(cache_killer cache_killer.cpp)

target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} bitstrm)
target_link_libraries(binary_search ${Boost_LIBRARIES} bitstrm)
target_link_libraries(cache_killer ${Boost_LIBRARIES} bitstrm)


