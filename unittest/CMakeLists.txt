cmake_minimum_required(VERSION 3.2.2)

# Unittest framework dependent on boost

if("${USE_BOOST}")

	set(TEST                       
	   bref_unittest.cpp
	   reg_unittest.cpp
	   print_unittest.cpp
	   utility_unittest.cpp   
           bitstrm_unittest.cpp 
	   )


	find_package(Boost 1.4 COMPONENTS unit_test_framework)
	include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
	include_directories("${ROOT_INCLUDE}")

	# Run through each source
	foreach(TEST_SRC ${TEST})	
           #Extract the filename without an extension (NAME_WE)
           get_filename_component(TEST_NAME ${TEST_SRC} NAME_WE)

           add_executable(${TEST_NAME} ${TEST_SRC})

           target_link_libraries(${TEST_NAME} ${Boost_LIBRARIES})

           add_test( NAME ${TEST_NAME} 
		  COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME}
                  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
           	 )
	endforeach(TEST_SRC)
endif("${USE_BOOST}")
