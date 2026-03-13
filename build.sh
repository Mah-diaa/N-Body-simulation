#!/bin/bash
# Simple build script for N-Body Simulation

echo "========================================"
echo "  N-Body Simulation - Build Script"
echo "========================================"
echo ""

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir build
fi

# Navigate to build directory
cd build

# Run CMake
echo "Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

if [ $? -ne 0 ]; then
    echo ""
    echo "CMake configuration failed!"
    echo "Make sure you have CMake and required dependencies installed."
    exit 1
fi

# Build the project
echo ""
echo "Building project..."
make -j$(nproc)

if [ $? -ne 0 ]; then
    echo ""
    echo "Build failed!"
    exit 1
fi

echo ""
echo "Build successful!"
echo ""
echo "To run the simulation:"
echo "  cd build"
echo "  ./nbody"
echo ""
echo "Or simply run: ./run.sh"
echo ""
