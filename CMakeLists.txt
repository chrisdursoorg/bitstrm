cmake_minimum_required(VERSION 2.8)

# CMAKE_BUILD_TYPE=[Release|Debug]

enable_testing()

project(bitstrm)

################################################################################
# *this directory should be bitstrm*
#
# then this repo can be built, in place as a submodule, or as an independent
# build with an abritrary path
#
# in including '..' files within this project as root, all headers can be
# prefaced with bitstrm (as #include "bitstrm/header_name.hpp";) and no
# conflicts will arise 

add_definitions("-std=c++11 -Wall -pedantic -Wall -Wextra")

set (ROOT_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/..")

add_subdirectory(unittest)
add_subdirectory(examples)
