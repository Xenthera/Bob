cmake_minimum_required(VERSION 3.10)
project(raylib)

# Set C++11 standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build type defaults to Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler-specific optimization options
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(MODULE_COMPILE_OPTIONS
        -Wall -Wextra
        -Wno-unused-variable
        -Wno-unused-parameter
    )
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        list(APPEND MODULE_COMPILE_OPTIONS -O3 -march=native -DNDEBUG)
    endif()
elseif(MSVC)
    set(MODULE_COMPILE_OPTIONS
        /W4
        /wd4100  # unreferenced formal parameter
        /wd4101  # unreferenced local variable
    )
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        list(APPEND MODULE_COMPILE_OPTIONS /O2 /DNDEBUG)
    endif()
endif()

# Add local include directory (Bob SDK)
include_directories(${CMAKE_SOURCE_DIR}/include)

# Find and link raylib library using pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(RAYLIB REQUIRED raylib)
include_directories(${RAYLIB_INCLUDE_DIRS})
link_directories(${RAYLIB_LIBRARY_DIRS})

add_library(raylib_module SHARED
    source/raylib_module.cpp
)

target_link_libraries(raylib_module ${RAYLIB_LIBRARIES})

# Apply compiler options
target_compile_options(raylib_module PRIVATE ${MODULE_COMPILE_OPTIONS})

set_target_properties(raylib_module PROPERTIES
    OUTPUT_NAME "raylib"
    PREFIX "lib"
)
