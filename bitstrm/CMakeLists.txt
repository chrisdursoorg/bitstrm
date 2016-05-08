cmake_minimum_required(VERSION 2.8)

project(bitstrm)

set (SRC bitstrm.cpp)

include_directories("${ROOT_INCLUDE}")

add_library(${PROJECT_NAME} ${SRC})

add_subdirectory(unittest)

