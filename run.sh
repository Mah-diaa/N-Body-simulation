#!/bin/bash
# Simple run script for N-Body Simulation

# Check if executable exists
if [ ! -f "build/nbody" ]; then
    echo "Executable not found. Building project first..."
    ./build.sh

    if [ $? -ne 0 ]; then
        echo "Build failed. Cannot run simulation."
        exit 1
    fi
fi

# Run the simulation
echo "Starting N-Body Simulation..."
cd build
./nbody
