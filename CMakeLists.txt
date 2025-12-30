cmake_minimum_required(VERSION 3.10)
project(NBodySimulation LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(SOURCES
    src/main.cpp
    src/simulation.cpp
    src/renderer.cpp
    src/scenarios.cpp
    src/menu.cpp

    # Algorithm implementations (pure logic, no rendering)
    src/algorithms/quadtree2d.cpp
    src/algorithms/octree3d.cpp

    # Demo/visualization code
    src/demos/quadtree_demo.cpp
    src/demos/octree_demo.cpp
)

# Add executable
add_executable(nbody ${SOURCES})

# Include directories
target_include_directories(nbody PRIVATE include)

# Find and link raylib
find_package(raylib QUIET)
if (NOT raylib_FOUND)
    # If raylib is not found via CMake, try pkg-config
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(RAYLIB REQUIRED raylib)
    target_include_directories(nbody PRIVATE ${RAYLIB_INCLUDE_DIRS})
    target_link_libraries(nbody ${RAYLIB_LIBRARIES})
else()
    target_link_libraries(nbody raylib)
endif()

# Optimization flags for Release build
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(nbody PRIVATE -O3 -march=native)
endif()

# Debug flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(nbody PRIVATE -g -Wall -Wextra)
endif()
