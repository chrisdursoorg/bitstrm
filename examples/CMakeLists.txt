project(examples)

include_directories("${ROOT_INCLUDE}")
include_directories("${CMAKE_SOURCE_DIR}")


# sort_10_bit and cycle_test depend of Boost libarary
if("${USE_BOOST}")
	find_package(Boost 1.4 COMPONENTS unit_test_framework)
	include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
	set(TEST "${TEST} bit_int_itr_unittest.cpp")
	include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
	add_executable(sort_10_bit sort_10_bit.cpp)	
	add_executable(cycle_test cycle_test.cpp)
	add_executable(sort_perf sort_perf.cpp)
endif("${USE_BOOST}")

add_executable(misc_perf misc_perf.cpp)
add_executable(example example.cpp)





