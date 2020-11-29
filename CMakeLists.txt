cmake_minimum_required(VERSION 2.8)

enable_testing()

project(bitstrm)

add_definitions("-std=c++17 -Wall -pedantic -Wall -Wextra")

IF(NOT DEFINED ${CMAKE_BUILD_TYPE})
    message ("defaulting to Release mode, do you want -DCMAKE_BUILD_TYPE='Debug' ?")
ENDIF()

IF(CMAKE_BUILD_TYPE MATCHES Debug)
    message("Debug build.")
    add_definitions("-O0 -g")
ELSE()
    message("Release build.")
    add_definitions("-O3 -Ofast")
ENDIF()

# Uncomment below if Boost is installed and you wish to use
# unittest or bit_int_itr
set(USE_BOOST "TRUE")


set(HEADER_FILES  ${CMAKE_CURRENT_SOURCE_DIR}/bitstrm)

add_library(${PROJECT_NAME} INTERFACE)
target_sources(${PROJECT_NAME} INTERFACE $<BUILD_INTERFACE:${HEADER_FILES}>)
target_include_directories(${PROJECT_NAME} INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/bitstrm/>)
target_include_directories(${PROJECT_NAME} SYSTEM INTERFACE $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/bitstrm>)

set (ROOT_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(unittest)
add_subdirectory(examples)

install(CODE "MESSAGE(\"Header only library Installing to ${CMAKE_INSTALL_PREFIX}/include/${PROJECT_NAME}. \")")

install(DIRECTORY bitstrm DESTINATION include)
#install(TARGETS "${PROJECT_NAME}" PUBLIC_HEADER DESTINATION include)
