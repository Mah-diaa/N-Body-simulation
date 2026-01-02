# N-Body Simulation with Barnes-Hut Algorithm

A real-time 3D gravitational N-body simulation implemented in C++ with an interactive educational visualization of the Barnes-Hut algorithm.

![N-Body Simulation](https://img.shields.io/badge/C++-17-blue.svg)
![CMake](https://img.shields.io/badge/CMake-3.10+-green.svg)
![License](https://img.shields.io/badge/license-MIT-orange.svg)

<details open>
<summary>🌟 Features</summary>

### Interactive Educational Visualization
- **Algorithm Walkthrough Mode**: Step-by-step visual demonstration of how Barnes-Hut algorithm works
- **4 Educational Modes**:
  1. **Brute Force O(n²)**: Visualize all pairwise particle interactions
  2. **Octree Construction**: Watch the octree being built step-by-step as particles are added
  3. **Octree Structure**: Explore the complete hierarchical tree structure
  4. **Barnes-Hut Calculation**: See how the algorithm approximates forces using the octree

![3D Octree Demo](assets/screenshots/3D_demo.png)

- **2D QuadTree Demo**: Interactive 2D visualization of spatial partitioning

![2D QuadTree Demo](assets/screenshots/2D_demo.png)

### Realistic Physics Simulation
- **Multiple Computation Methods**:
  - CPU Brute Force O(n²) - Direct pairwise calculation
  - Barnes-Hut O(n log n) - Optimized octree approximation
- **5 Astrophysical Scenarios**:
  - Grid (simple 3D grid for testing)
  - Binary Stars (orbital dynamics)
  - Big Bang (explosive particle expansion)
  - Stable Globular Cluster (virial equilibrium)
  - Keplerian Disk (protoplanetary disks)
  - Black Hole Accretion Disk
  - Galaxy Collision (two galaxies colliding)

| Keplerian Disk | Big Bang |
|:---:|:---:|
| ![Keplerian Disk](assets/gifs/keplerdisc.gif) | ![Big Bang](assets/gifs/bigbang.gif) |

### Advanced Features
- Real-time method switching (Brute Force ↔ Barnes-Hut)
- Adjustable theta parameter for accuracy tuning
- Simulation speed control (1x-10x)
- Particle trails visualization
- Full 3D camera controls (rotate, pan, zoom)
- Progressive octree building demonstration
- UI overlay toggle

</details>

<details>
<summary>🎓 How It Works</summary>

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

</details>

<details>
<summary>📁 Project Structure</summary>

```
nbody/
├── src/
│   ├── algorithms/
│   │   ├── octree3d.cpp         # Barnes-Hut 3D (flat vector, implicit indexing)
│   │   └── quadtree2d.cpp       # Barnes-Hut 2D (educational demo)
│   ├── demos/
│   └── main.cpp, simulation.cpp, renderer.cpp, scenarios.cpp, menu.cpp
│
├── include/
│   ├── algorithms/
│   ├── demos/
│   └── particle.h, simulation.h, renderer.h, scenarios.h, menu.h
│
└── CMakeLists.txt, build.sh, run.sh
```

</details>

<details>
<summary>🛠️ Libraries & Dependencies</summary>

### Raylib
**What it is**: A simple and easy-to-use library to enjoy videogames programming

**What it provides**:
- 3D rendering (spheres, lines, cubes for visualization)
- Camera system (perspective projection, orbit controls)
- Input handling (mouse, keyboard)
- Text rendering for UI

**Installation (Linux)**:
```bash
# Ubuntu/Debian - Install raylib dependencies
sudo apt install libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev libwayland-dev libxkbcommon-dev
```

### CMake
**What it is**: Cross-platform build system generator

</details>

<details>
<summary>🚀 Quick Start</summary>

### Prerequisites
```bash
# Ubuntu/Debian - Install build tools and raylib dependencies
sudo apt install cmake g++ libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev libwayland-dev libxkbcommon-dev
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

</details>

<details>
<summary>🎮 Controls</summary>

### Camera (Universal)
- **Left Click + Drag**: Rotate camera
- **Mouse Wheel**: Zoom in/out

### Main Menu
- **Number keys (1-7)**: Select scenario
- **A**: 3D Octree Demo
- **B**: 2D QuadTree Demo

### Simulation
- **M**: Toggle method (Brute Force ↔ Barnes-Hut)
- **T**: Toggle trails
- **H**: Hide UI
- **R**: Reset
- **Q**: Return to menu

### Performance Tuning
- **+/-**: Theta parameter - Lower values = more accurate but slower, higher = faster but less accurate
- **UP/DOWN**: Simulation speed (1x-10x) - Run multiple physics steps per frame for faster evolution
- **[/]**: Tree rebuild interval - How often the octree is reconstructed (lower = more accurate, higher = faster)

### Demo Modes
- **SPACE**: Step through demonstration / Add next particle
- **1-4**: Switch visualization mode (3D Octree Demo only)
- **R**: Reset

</details>

<details>
<summary>📊 Performance Comparison</summary>

For 1,000 particles:

| Method | Calculations per Frame | Speedup |
|--------|------------------------|---------|
| Brute Force | ~500,000 | 1x |
| Barnes-Hut (θ=0.7) | ~15,000 | ~33x |

The Barnes-Hut algorithm becomes even more advantageous as particle count increases!

</details>

<details>
<summary>📝 License</summary>

This project is open source and available under the MIT License.

</details>

<details>
<summary>📚 Further Reading</summary>

- [Original Barnes-Hut Paper (1986)](https://ui.adsabs.harvard.edu/abs/1986Natur.324..446B)
- [Octree Data Structure](https://en.wikipedia.org/wiki/Octree)
- [N-body Problem](https://en.wikipedia.org/wiki/N-body_problem)
- [Raylib Documentation](https://www.raylib.com/)

</details>
