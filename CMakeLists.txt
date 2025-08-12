cmake_minimum_required(VERSION 3.20)

# Project definition
project(bob 
    VERSION 0.0.3
    DESCRIPTION "Bob Language Interpreter"
    LANGUAGES CXX
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type defaults
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(BOB_COMPILE_OPTIONS
        -Wall -Wextra 
        -Wno-unused-variable 
        -Wno-unused-parameter 
        -Wno-switch
    )
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        list(APPEND BOB_COMPILE_OPTIONS -O3 -march=native)
    endif()
elseif(MSVC)
    set(BOB_COMPILE_OPTIONS
        /W4 
        /wd4100  # unreferenced formal parameter
        /wd4101  # unreferenced local variable
    )
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        list(APPEND BOB_COMPILE_OPTIONS /O2)
    endif()
endif()

# Collect source files
file(GLOB_RECURSE BOB_RUNTIME_SOURCES CONFIGURE_DEPENDS "src/sources/runtime/*.cpp")
file(GLOB_RECURSE BOB_PARSING_SOURCES CONFIGURE_DEPENDS "src/sources/parsing/*.cpp")
file(GLOB_RECURSE BOB_STDLIB_SOURCES CONFIGURE_DEPENDS "src/sources/stdlib/*.cpp")
file(GLOB_RECURSE BOB_BUILTIN_SOURCES CONFIGURE_DEPENDS "src/sources/builtinModules/*.cpp")
file(GLOB_RECURSE BOB_CLI_SOURCES CONFIGURE_DEPENDS "src/sources/cli/*.cpp")

# All source files
set(BOB_ALL_SOURCES
    ${BOB_RUNTIME_SOURCES}
    ${BOB_PARSING_SOURCES}
    ${BOB_STDLIB_SOURCES}
    ${BOB_BUILTIN_SOURCES}
    ${BOB_CLI_SOURCES}
)

# Create the executable
add_executable(bob ${BOB_ALL_SOURCES})

# Include directories
target_include_directories(bob PRIVATE
    src/headers/runtime
    src/headers/parsing
    src/headers/stdlib
    src/headers/builtinModules
    src/headers/cli
    src/headers/common
)

# Apply compiler options
target_compile_options(bob PRIVATE ${BOB_COMPILE_OPTIONS})

# Enable Link Time Optimization (LTO) for Release builds
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
    if(ipo_supported)
        message(STATUS "IPO/LTO enabled")
        set_property(TARGET bob PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(WARNING "IPO/LTO not supported: ${ipo_error}")
    endif()
endif()

# Platform-specific settings
if(WIN32)
    # Windows-specific settings
    target_compile_definitions(bob PRIVATE _CRT_SECURE_NO_WARNINGS)
elseif(UNIX AND NOT APPLE)
    # Linux-specific settings
    target_link_libraries(bob PRIVATE pthread)
elseif(APPLE)
    # macOS-specific settings
    set_target_properties(bob PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist
    )
endif()

# Generate compile_commands.json for language servers
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Testing support
enable_testing()

# Add test for the main test suite
add_test(
    NAME bob_test_suite
    COMMAND bob test_bob_language.bob
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Custom target for running tests with verbose output
add_custom_target(test_verbose
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose
    DEPENDS bob
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Install rules
install(TARGETS bob
    RUNTIME DESTINATION bin
    COMPONENT Runtime
)

# Install test files (optional)
install(FILES test_bob_language.bob
    DESTINATION share/bob/tests
    COMPONENT Tests
)

# CPack configuration for packaging
set(CPACK_PACKAGE_NAME "Bob Language")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Bob Language Interpreter")
set(CPACK_PACKAGE_VENDOR "Bob Language Team")
set(CPACK_PACKAGE_CONTACT "your-email@example.com")

if(WIN32)
    set(CPACK_GENERATOR "ZIP;NSIS")
elseif(APPLE)
    set(CPACK_GENERATOR "TGZ;DragNDrop")
else()
    set(CPACK_GENERATOR "TGZ;DEB;RPM")
endif()

include(CPack)

# Print configuration summary
message(STATUS "Bob Language Build Configuration:")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Install Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Runtime Sources: ${BOB_RUNTIME_SOURCES}")
message(STATUS "  Parsing Sources: ${BOB_PARSING_SOURCES}")
message(STATUS "  Stdlib Sources: ${BOB_STDLIB_SOURCES}")
message(STATUS "  CLI Sources: ${BOB_CLI_SOURCES}")