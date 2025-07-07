project(examples)

include_directories("${ROOT_INCLUDE}")
include_directories("${CMAKE_SOURCE_DIR}")

add_executable(sort_10_bit sort_10_bit.cpp)	
add_executable(cycle_test cycle_test.cpp)
add_executable(sort_perf sort_perf.cpp)
add_executable(example example.cpp)
add_executable(consolidate_perf consolidate_perf.cpp)
add_executable(interior_block_delete interior_block_delete.cpp)
add_executable(system system.cpp)



