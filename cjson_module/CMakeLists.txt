cmake_minimum_required(VERSION 3.10)
project(cjson)

# Set C++11 standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add local include directory (Bob SDK)
include_directories(${CMAKE_SOURCE_DIR}/include)

# Find and link cJSON library using pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(CJSON REQUIRED libcjson)
include_directories(${CJSON_INCLUDE_DIRS})
link_directories(${CJSON_LIBRARY_DIRS})

add_library(cjson_module SHARED
    source/cjson_module.cpp
)

target_link_libraries(cjson_module ${CJSON_LIBRARIES})

set_target_properties(cjson_module PROPERTIES
    OUTPUT_NAME "cjson"
    PREFIX "lib"
)
