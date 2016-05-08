project(performnance_tests)

set(SRCS  main.cpp)

include_directories("${CMAKE_SOURCE_DIR}")

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} bitstrm)


