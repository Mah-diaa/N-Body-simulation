cmake_minimum_required(VERSION 3.11)
project(NBodySimulation LANGUAGES CXX)

# Auto-fetch raylib if not found
include(FetchContent)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

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

# Try to find raylib, if not found fetch it automatically
find_package(raylib QUIET)
if (NOT raylib_FOUND)
    find_package(PkgConfig QUIET)
    if (PkgConfig_FOUND)
        pkg_check_modules(RAYLIB QUIET raylib)
    endif()

    if (RAYLIB_FOUND)
        target_include_directories(nbody PRIVATE ${RAYLIB_INCLUDE_DIRS})
        target_link_directories(nbody PRIVATE ${RAYLIB_LIBRARY_DIRS})
        target_link_libraries(nbody ${RAYLIB_LIBRARIES})
    else()
        # Fetch raylib from GitHub
        message(STATUS "Raylib not found locally, fetching from GitHub...")
        FetchContent_Declare(
            raylib
            GIT_REPOSITORY https://github.com/raysan5/raylib.git
            GIT_TAG 5.0
        )
        FetchContent_MakeAvailable(raylib)
        target_link_libraries(nbody raylib)
    endif()
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
