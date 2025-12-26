# N-Body Simulation with Barnes-Hut Algorithm

A real-time 3D gravitational N-body simulation implemented in C++ with an interactive educational visualization of the Barnes-Hut algorithm.

![N-Body Simulation](https://img.shields.io/badge/C++-17-blue.svg)
![CMake](https://img.shields.io/badge/CMake-3.10+-green.svg)
![License](https://img.shields.io/badge/license-MIT-orange.svg)

## 🌟 Features

### Interactive Educational Visualization
- **Algorithm Walkthrough Mode**: Step-by-step visual demonstration of how Barnes-Hut algorithm works
- **4 Educational Modes**:
  1. **Brute Force O(n²)**: Visualize all pairwise particle interactions
  2. **Octree Construction**: Watch the octree being built step-by-step
  3. **Octree Structure**: Explore the complete hierarchical tree structure
  4. **Barnes-Hut Calculation**: See how the algorithm approximates forces using the octree

### Realistic Physics Simulation
- **Multiple Computation Methods**:
  - CPU Brute Force O(n²) - Direct pairwise calculation
  - Barnes-Hut O(n log n) - Optimized octree approximation
- **9 Astrophysical Scenarios**:
  - Plummer Sphere (globular clusters)
  - Galaxy Disk (spiral galaxies)
  - Keplerian Disk (protoplanetary disks)
  - Binary Star System
  - Solar System (with realistic masses)
  - Black Hole Accretion Disk
  - Galaxy Collision
  - Figure-8 Three-Body Orbit
  - Simple Grid (for testing)

### Advanced Features
- Real-time method switching (Brute Force ↔ Barnes-Hut)
- Adjustable theta parameter for accuracy tuning
- Simulation speed control (1x-10x)
- Particle trails visualization
- Interactive projectile shooting
- Full 3D camera controls (rotate, pan, zoom)

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
├── build.sh                     # Easy build script (Linux/Mac)
├── run.sh                       # Easy run script
│
├── src/                         # Source files (.cpp)
│   ├── main.cpp                 # Entry point, scenario selection menu
│   ├── menu.cpp                 # Interactive menu system
│   ├── simulation.cpp           # Brute force physics engine
│   ├── octree_optimized.cpp     # Barnes-Hut octree implementation
│   ├── scenarios.cpp            # 9 astrophysical scenario generators
│   ├── algorithm_demo.cpp       # Educational visualization mode
│   └── renderer.cpp             # 3D rendering and visualization
│
├── include/                     # Header files (.h)
│   ├── particle.h               # Particle data structure
│   ├── menu.h                   # Menu system declarations
│   ├── simulation.h             # Physics engine interface
│   ├── octree_optimized.h       # Octree structure and algorithms
│   ├── scenarios.h              # Scenario types and generators
│   ├── algorithm_demo.h         # Educational mode interface
│   └── renderer.h               # Rendering functions
│
└── build/                       # Build output (gitignored)
    └── nbody                    # Compiled executable
```

### File Descriptions

#### Core Files
- **main.cpp**: Program entry point, handles scenario selection menu and main loop
- **particle.h**: Defines the `Particle` struct (position, velocity, mass, rendering info)

#### Physics Engine
- **simulation.cpp/h**: Implements brute force O(n²) gravity calculations
- **octree_optimized.cpp/h**: Barnes-Hut tree structure with optimized flat storage
  - Flat vector storage for better cache locality
  - Implicit child indexing (children stored sequentially)

#### Visualization
- **renderer.cpp/h**: All rendering functions using Raylib
  - Camera controls, particle rendering, trails, UI
- **algorithm_demo.cpp/h**: Educational walkthrough visualization
  - Step-by-step algorithm demonstration
  - Visual octree construction
  - Interactive learning mode

#### Scenarios & Menu
- **scenarios.cpp/h**: Generators for 9 different astrophysical scenarios
  - Each scenario has custom parameters (G, dt, particle distribution)
- **menu.cpp/h**: Interactive menu system for scenario and mode selection

## 🛠️ Libraries & Dependencies

### Raylib
**What it is**: A simple and easy-to-use library to enjoy videogames programming

**Why we use it**:
- Easy 3D visualization without complex OpenGL setup
- Built-in camera controls
- Cross-platform (Windows, Linux, macOS)
- Lightweight and fast

**What it provides**:
- 3D rendering (spheres, lines, cubes for visualization)
- Camera system (perspective projection, orbit controls)
- Input handling (mouse, keyboard)
- Text rendering for UI

**Installation**:
- Ubuntu/Debian: `sudo apt install libraylib-dev`
- macOS: `brew install raylib`
- Windows: See [Raylib Installation Guide](https://github.com/raysan5/raylib)

### CMake
**What it is**: Cross-platform build system generator

**Why we use it**: Manages the build process across different platforms

## 🚀 Quick Start

### Prerequisites
```bash
# Ubuntu/Debian
sudo apt install cmake g++ libraylib-dev

# macOS
brew install cmake raylib

# Windows (using vcpkg)
vcpkg install raylib
```

### Build & Run (Easy Way)

#### Linux/macOS:
```bash
# Make scripts executable (first time only)
chmod +x build.sh run.sh

# Build
./build.sh

# Run
./run.sh
```

#### Manual Build (All Platforms):
```bash
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
make        # Linux/macOS
# or
cmake --build . --config Release    # Windows

# Run
./nbody     # Linux/macOS
# or
.\nbody.exe # Windows
```

## 🎮 Controls

### Main Menu
- **Number keys (1-9)**: Select astrophysical scenario
- **A**: Educational Algorithm Demonstration Mode
- **ESC**: Exit program

### Simulation Mode

#### Camera Controls
- **Left Click + Drag**: Rotate camera around scene
- **Middle Click + Drag**: Pan camera (move viewpoint)
- **Mouse Wheel**: Zoom in/out
- **WASD**: Move camera target

#### Simulation Controls
- **M**: Toggle method (Brute Force ↔ Barnes-Hut)
- **T**: Toggle particle trails
- **UP/DOWN**: Adjust simulation speed (1x-10x)
- **+/-**: Adjust theta parameter (Barnes-Hut accuracy)
- **[/]**: Adjust tree rebuild interval
- **R**: Reset to initial state
- **SPACE**: Shoot projectile from camera
- **Q**: Return to menu
- **ESC**: Exit program

### Educational Mode

#### Mode Selection
- **1**: Brute Force Visualization
- **2**: Octree Construction (step-by-step)
- **3**: Complete Octree Structure
- **4**: Barnes-Hut Force Calculation

#### Controls
- **SPACE**: Step through demonstration
- **R**: Reset demonstration
- **+/-**: Adjust theta parameter (mode 4 only)
- **Left Click + Drag**: Rotate camera
- **Middle Click + Drag**: Pan camera
- **Mouse Wheel**: Zoom
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
- ✅ Spatial data structures (Octree)
- ✅ Algorithm optimization (O(n²) → O(n log n))
- ✅ Computational physics (gravitational N-body problem)
- ✅ 3D graphics and visualization
- ✅ Interactive real-time applications
- ✅ Educational software design

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

---

**Made with ❤️ for learning computational physics and computer graphics**
