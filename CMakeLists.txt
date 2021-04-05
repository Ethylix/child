cmake_minimum_required(VERSION 3.8)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

project(Child C)
set(CHILD_VERSION "2.0-dev")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -W -O2")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
add_definitions(-DHAVE_CONFIG_H)
set(CONFIG_HEADER ${CMAKE_CURRENT_BINARY_DIR}/config.h)

find_package(OpenSSL REQUIRED)
find_package(MySQL REQUIRED)

find_library(DL_LIBRARY NAMES dl)
if (DL_LIBRARY)
  MESSAGE(STATUS "Found dl: ${DL_LIBRARY}")
ELSE (DL_LIBRARY)
  MESSAGE(FATAL_ERROR "Could not find dl library")
ENDIF (DL_LIBRARY)

add_subdirectory(src)

enable_testing()
add_subdirectory(tests)
