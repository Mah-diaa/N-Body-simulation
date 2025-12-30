# N-Body Simulation with Barnes-Hut Algorithm

A real-time 3D gravitational N-body simulation implemented in C++ with an interactive educational visualization of the Barnes-Hut algorithm.

![N-Body Simulation](https://img.shields.io/badge/C++-17-blue.svg)
![CMake](https://img.shields.io/badge/CMake-3.10+-green.svg)
![License](https://img.shields.io/badge/license-MIT-orange.svg)

## 📑 Table of Contents

- [Features](#-features)
- [How It Works](#-how-it-works)
- [Project Structure](#-project-structure)
- [Libraries & Dependencies](#️-libraries--dependencies)
- [Quick Start](#-quick-start)
- [Controls](#-controls)
- [Performance Comparison](#-performance-comparison)
- [Learning Outcomes](#-learning-outcomes)
- [Further Reading](#-further-reading)

## 🌟 Features

### Interactive Educational Visualization
- **Algorithm Walkthrough Mode**: Step-by-step visual demonstration of how Barnes-Hut algorithm works
- **4 Educational Modes**:
  1. **Brute Force O(n²)**: Visualize all pairwise particle interactions
  2. **Octree Construction**: Watch the octree being built step-by-step as particles are added
  3. **Octree Structure**: Explore the complete hierarchical tree structure
  4. **Barnes-Hut Calculation**: See how the algorithm approximates forces using the octree
- **2D QuadTree Demo**: Interactive 2D visualization of spatial partitioning

### Realistic Physics Simulation
- **Multiple Computation Methods**:
  - CPU Brute Force O(n²) - Direct pairwise calculation
  - Barnes-Hut O(n log n) - Optimized octree approximation
- **5 Astrophysical Scenarios**:
  - Grid (simple 3D grid for testing)
  - Galaxy Disk (spiral galaxies with rotation)
  - Keplerian Disk (protoplanetary disks)
  - Black Hole Accretion Disk
  - Galaxy Collision (two galaxies colliding)

### Advanced Features
- Real-time method switching (Brute Force ↔ Barnes-Hut)
- Adjustable theta parameter for accuracy tuning
- Simulation speed control (1x-10x)
- Particle trails visualization
- Full 3D camera controls (rotate, pan, zoom)
- Progressive octree building demonstration
- UI overlay toggle

## 🎓 How It Works

### The N-Body Problem
The N-body problem involves simulating the gravitational interactions between N particles. Each particle exerts a gravitational force on every other particle according to Newton's law of gravitation.

### Brute Force Method O(n²)
The naive approach calculates forces between every pair of particles:
- For N particles, requires N×(N-1)/2 calculations
- Exact but becomes extremely slow as N increases
- 1,000 particles = 499,500 calculations per timestep!

### Barnes-Hut Algorithm O(n log n)
An intelligent approximation that groups distant particles:

1. **Build Octree**: Divide 3D space into a hierarchical tree structure
   - Root node contains all particles
   - Recursively subdivide into 8 octants (children)
   - Stop when each cell contains ≤1 particle

2. **Calculate Center of Mass**: For each node, compute:
   - Total mass of all particles in that region
   - Center of mass position

3. **Force Calculation**: For each particle:
   - Traverse the octree from root
   - If a node is "far enough" (size/distance < θ), treat all its particles as a single point mass
   - If too close, recurse into children
   - This reduces calculations from O(n²) to O(n log n)

4. **Time Integration**: Update particle positions and velocities using Euler method

### Theta Parameter (θ)
Controls the accuracy-speed tradeoff:
- **Low θ (0.3-0.5)**: More accurate, closer to brute force
- **High θ (1.0-2.0)**: Faster, more approximation
- Default: 0.7

## 📁 Project Structure

```
nbody/
├── README.md                    # This file
├── .gitignore                   # Git ignore rules
├── CMakeLists.txt               # CMake build configuration
├── build.sh                     # Easy build script
├── run.sh                       # Easy run script
│
├── src/                         # Source files (.cpp)
│   ├── main.cpp                 # Entry point, main loop, state management
│   ├── menu.cpp                 # Interactive menu system
│   ├── simulation.cpp           # Brute force physics engine
│   ├── scenarios.cpp            # Astrophysical scenario generators
│   ├── renderer.cpp             # 3D rendering and visualization
│   │
│   ├── algorithms/              # Pure algorithm implementations (no rendering)
│   │   ├── README.md            # Algorithm directory documentation
│   │   ├── octree3d.cpp         # 3D Octree (Barnes-Hut in 3D)
│   │   └── quadtree2d.cpp       # 2D QuadTree (Barnes-Hut in 2D)
│   │
│   └── demos/                   # Visualization demos (uses algorithms + rendering)
│       ├── README.md            # Demos directory documentation
│       ├── octree_demo.cpp      # 3D Octree educational visualization
│       └── quadtree_demo.cpp    # 2D QuadTree interactive demo
│
├── include/                     # Header files (.h)
│   ├── particle.h               # Particle data structure
│   ├── menu.h                   # Menu system declarations
│   ├── simulation.h             # Physics engine interface
│   ├── scenarios.h              # Scenario types and generators
│   ├── renderer.h               # Rendering functions
│   │
│   ├── algorithms/              # Algorithm headers
│   │   ├── octree3d.h           # 3D Octree structure and interface
│   │   └── quadtree2d.h         # 2D QuadTree structure and interface
│   │
│   └── demos/                   # Demo headers
│       ├── octree_demo.h        # 3D demo interface
│       └── quadtree_demo.h      # 2D demo interface
│
└── build/                       # Build output (gitignored)
    └── nbody                    # Compiled executable
```

### File Organization

#### Core Files
- **main.cpp**: Program entry point, scenario selection menu, main loop
- **particle.h**: Defines the `Particle` struct (position, velocity, mass)
- **menu.cpp/h**: Interactive menu system for scenario selection
- **renderer.cpp/h**: 3D rendering functions using Raylib

#### Physics Engine
- **simulation.cpp/h**: Brute force O(n²) gravity calculations
- **scenarios.cpp/h**: Generators for 5 astrophysical scenarios

#### Algorithms (Pure Logic)
- **algorithms/octree3d.cpp/h**: Barnes-Hut 3D octree implementation
  - Flat vector storage for cache locality
  - Implicit child indexing
  - Optimized for computation
- **algorithms/quadtree2d.cpp/h**: Barnes-Hut 2D quadtree implementation
  - 2D spatial partitioning
  - Center of mass calculation

#### Demos (Visualization)
- **demos/octree_demo.cpp/h**: Educational 3D octree walkthrough
  - 4 interactive visualization modes
  - Progressive tree building demonstration
  - Step-by-step algorithm explanation
- **demos/quadtree_demo.cpp/h**: Interactive 2D quadtree demo
  - Step-by-step particle insertion
  - Real-time tree structure visualization
  - Center of mass tracking

## 🛠️ Libraries & Dependencies

### Raylib
**What it is**: A simple and easy-to-use library to enjoy videogames programming

**What it provides**:
- 3D rendering (spheres, lines, cubes for visualization)
- Camera system (perspective projection, orbit controls)
- Input handling (mouse, keyboard)
- Text rendering for UI

**Installation (Linux)**:
```bash
# Ubuntu/Debian
sudo apt install libraylib-dev
```

### CMake
**What it is**: Cross-platform build system generator

## 🚀 Quick Start

### Prerequisites
```bash
# Ubuntu/Debian
sudo apt install cmake g++ libraylib-dev
```

### Build & Run

```bash
# Make scripts executable (first time only)
chmod +x build.sh run.sh

# Build
./build.sh

# Run
./run.sh
```

### Manual Build
```bash
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
make

# Run
./nbody
```

## 🎮 Controls

### Main Menu
- **Number keys (1-5)**: Select astrophysical scenario
- **A**: 3D Octree Educational Demonstration Mode
- **B**: 2D QuadTree Educational Demonstration Mode
- **ESC**: Exit program

### Simulation Mode

#### Camera Controls
- **Left Click + Drag**: Rotate camera around scene
- **Mouse Wheel**: Zoom in/out

#### Simulation Controls
- **M**: Toggle method (Brute Force ↔ Barnes-Hut)
- **T**: Toggle particle trails
- **H**: Toggle UI overlay visibility
- **UP/DOWN**: Adjust simulation speed (1x-10x)
- **+/-**: Adjust theta parameter (Barnes-Hut accuracy)
- **[/]**: Adjust tree rebuild interval
- **R**: Reset to initial state
- **Q**: Return to menu
- **ESC**: Exit program

### 3D Octree Demo Mode

#### Mode Selection
- **1**: Brute Force Visualization
- **2**: Progressive Octree Building (step-by-step)
- **3**: Complete Octree Structure
- **4**: Barnes-Hut Force Calculation

#### Controls
- **SPACE**: Step through demonstration (add next particle in mode 2)
- **R**: Reset demonstration
- **+/-**: Adjust theta parameter (mode 4 only)
- **Left Click + Drag**: Rotate camera
- **Mouse Wheel**: Zoom
- **Q**: Return to menu
- **ESC**: Exit program

### 2D QuadTree Demo Mode

#### Controls
- **SPACE**: Add next particle to tree
- **R**: Reset demonstration
- **Q**: Return to menu
- **ESC**: Exit program

## 📊 Performance Comparison

For 1,000 particles:

| Method | Calculations per Frame | Speedup |
|--------|------------------------|---------|
| Brute Force | ~500,000 | 1x |
| Barnes-Hut (θ=0.7) | ~15,000 | ~33x |

The Barnes-Hut algorithm becomes even more advantageous as particle count increases!

## 🎯 Learning Outcomes

This project demonstrates:
- ✅ Advanced C++ programming (STL, structs, headers)
- ✅ Spatial data structures (Octree, QuadTree)
- ✅ Algorithm optimization (O(n²) → O(n log n))
- ✅ Computational physics (gravitational N-body problem)
- ✅ 3D graphics and visualization
- ✅ Interactive real-time applications
- ✅ Educational software design
- ✅ Separation of concerns (algorithms vs visualization)

## 🤝 Contributing

This is an educational project, but suggestions and improvements are welcome! Feel free to:
- Report bugs
- Suggest new scenarios
- Propose algorithm optimizations
- Improve visualizations

## 📝 License

This project is open source and available under the MIT License.

## 🙏 Acknowledgments

- **Barnes & Hut (1986)**: For the original O(n log n) algorithm
- **Raylib**: For making 3D graphics accessible
- Inspired by astronomical simulations and computational astrophysics

## 📚 Further Reading

- [Original Barnes-Hut Paper (1986)](https://ui.adsabs.harvard.edu/abs/1986Natur.324..446B)
- [Octree Data Structure](https://en.wikipedia.org/wiki/Octree)
- [N-body Problem](https://en.wikipedia.org/wiki/N-body_problem)
- [Raylib Documentation](https://www.raylib.com/)
