project(examples)

include_directories("${ROOT_INCLUDE}")
include_directories("${CMAKE_SOURCE_DIR}")

add_executable(sort_10_bit sort_10_bit.cpp)
add_executable(misc_perf misc_perf.cpp)
add_executable(example example.cpp)




