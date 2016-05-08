cmake_minimum_required(VERSION 2.8)

cmake_minimum_required(VERSION 2.8)

# CMAKE_BUILD_TYPE=Release,Debug

enable_testing()

IF(NOT DEFINED ${CMAKE_BUILD_TYPE})
       message ("defaulting to Release mode, do you want -DCMAKE_BUILD_TYPE='Debug' ?")       
ENDIF()

IF(CMAKE_BUILD_TYPE MATCHES Debug)
  message("Debug build.")
  add_definitions("-O0 -g")
ELSEIF(CMAKE_BUILD_TYPE MATCHES Release)
  message("Release build.")
    add_definitions("-O3 -Ofast")	
ELSE()
ENDIF()

set(CMAKE_CXX_FLAGS_RELEASE, "${CMAKE_CXX_FLAGS_RELEASE} -04")

add_definitions("-std=c++11 -Wall -Wno-c++11-extensions -Weffc++ -pedantic -O3 -Wall -Wextra")

set (ROOT_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(bitstrm)
add_subdirectory(performance)
