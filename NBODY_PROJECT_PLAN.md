# N-Body Simulation Project Plan
**Computer Graphics Class - Barnes-Hut Algorithm**
**Deadline:** January 12, 2026 (~6 weeks)
**Last Updated:** December 1, 2025

---

## Project Overview

### Goals
- Implement Barnes-Hut N-body simulation algorithm in C++
- Learn C++ for future career goals
- Create engaging presentation explaining the algorithm
- Pass the class with a strong grade

### Strategy
- **Phase 1 (Weeks 1-3):** C++ Implementation - PRIORITY
  - Ensures you pass regardless of presentation format
  - Fallback: PowerPoint + working simulation
- **Phase 2 (Weeks 4-6):** Interactive Presentation - IF TIME ALLOWS
  - Only proceed after simulation works
  - Decision checkpoint at end of Week 3

### Time Constraints
- ~6 hours/day study time (split between projects)
- Realistically 2-3 hours/day for this project
- Some days off expected
- **Total estimated hours:** 90-130 hours

---

## Neovim Setup for C++ Development

### Essential Plugins

```lua
-- LSP (Language Server Protocol) - CRITICAL
'neovim/nvim-lspconfig'
'williamboman/mason.nvim'
'williamboman/mason-lspconfig.nvim'

-- Autocompletion
'hrsh7th/nvim-cmp'
'hrsh7th/cmp-nvim-lsp'
'hrsh7th/cmp-buffer'
'hrsh7th/cmp-path'
'L3MON4D3/LuaSnip'

-- Syntax Highlighting
'nvim-treesitter/nvim-treesitter'

-- Debugging (VERY helpful for C++)
'mfussenegger/nvim-dap'
'rcarriga/nvim-dap-ui'
'theHamsta/nvim-dap-virtual-text'

-- Build Integration
'Shatur/neovim-cmake'  -- Optional but nice

-- Code Formatting
'jose-elias-alvarez/null-ls.nvim'  -- For clang-format

-- File Navigation
'nvim-telescope/telescope.nvim'
'nvim-tree/nvim-tree.lua'
```

### Setup Commands
```vim
:MasonInstall clangd clang-format codelldb
```

### Key Features You'll Get
- Auto-completion with function signatures
- Go to definition (`gd`)
- Find references (`gr`)
- Inline error checking
- Integrated debugging with breakpoints
- Auto-formatting on save

---

## Week-by-Week Breakdown

### Week 1: C++ Basics + Direct N-body (Days 1-7)

#### Day 1-2: Environment Setup
**Tasks:**
- Install/configure Neovim with C++ plugins
- Setup CMake project structure
- Learn basic compilation workflow

**Milestone:** Compile and run "Hello World" with CMake

**Basic CMake Structure:**
```
nbody/
├── CMakeLists.txt
├── src/
│   └── main.cpp
├── include/
│   └── particle.h
└── build/
```

#### Day 3-4: C++ Fundamentals
**Learn:**
- Pointers, references, memory management
- Classes and structs
- `std::vector`, `std::array`
- File I/O

**Resources:**
- learncpp.com chapters 1-13, 16
- Focus on practical examples

**Milestone:** Create a Particle class with position/velocity/mass

**Example Particle Class:**
```cpp
struct Particle {
    double x, y, z;      // position
    double vx, vy, vz;   // velocity
    double ax, ay, az;   // acceleration
    double mass;
};
```

#### Day 5-7: Direct N-body (Brute Force)
**Implement:**
- O(n²) force calculation
- Newton's gravitational formula: F = G * m1 * m2 / r²
- Simple Euler integration
- Output to file or console

**Key Formula:**
```cpp
// For each pair of particles i and j:
double dx = particles[j].x - particles[i].x;
double dy = particles[j].y - particles[i].y;
double dz = particles[j].z - particles[i].z;
double dist = sqrt(dx*dx + dy*dy + dz*dz);
double force = G * particles[i].mass * particles[j].mass / (dist * dist * dist);
particles[i].ax += force * dx;
particles[i].ay += force * dy;
particles[i].az += force * dz;
```

**Resources:**
- Philip Mocz: "Create Your Own N-body Simulation"
- Wikipedia: N-body simulation

**Milestone:** Working simulation with 100 particles, output positions over time

---

### Week 2: Barnes-Hut Algorithm (Days 8-14)

#### Day 8-10: Quadtree/Octree Structure
**Learn:**
- Spatial data structures
- Recursive tree building
- Bounding boxes

**Implement:**
```cpp
struct Node {
    double x, y, z;          // center of mass
    double mass;             // total mass
    double size;             // size of region
    bool isLeaf;
    Node* children[8];       // 8 children for octree (or 4 for quadtree)
    Particle* particle;      // if leaf node
};
```

**Algorithm:**
1. Start with bounding box containing all particles
2. If region has 0 or 1 particles, done (leaf node)
3. Otherwise, subdivide into 8 octants
4. Recursively place particles into appropriate octants
5. Calculate center of mass for each node

**Resources:**
- Original Barnes-Hut paper (1986): "A hierarchical O(N log N) force-calculation algorithm"
- Wikipedia: Barnes-Hut simulation
- Tom Quinn's tutorial: "An Introduction to the Hierarchical Tree Method"

**Milestone:** Build octree from particle positions, verify structure

#### Day 11-13: Barnes-Hut Force Calculation
**Key Concept:** θ (theta) criterion
- If `size/distance < θ`, treat node as single body (use center of mass)
- Otherwise, recursively open node and check children
- Typical θ values: 0.5 to 1.0

**Algorithm:**
```cpp
void calculateForce(Particle& p, Node* node, double theta) {
    if (node->isLeaf) {
        // Calculate force with single particle
        if (node->particle != &p) {
            // Direct force calculation
        }
    } else {
        double dx = node->x - p.x;
        double dy = node->y - p.y;
        double dz = node->z - p.z;
        double distance = sqrt(dx*dx + dy*dy + dz*dz);

        if (node->size / distance < theta) {
            // Use center of mass approximation
            // Calculate force with node->mass at (node->x, node->y, node->z)
        } else {
            // Recursively traverse children
            for (int i = 0; i < 8; i++) {
                if (node->children[i] != nullptr) {
                    calculateForce(p, node->children[i], theta);
                }
            }
        }
    }
}
```

**Testing:**
- Compare results with direct method for small N
- Verify O(n log n) scaling

**Milestone:** Barnes-Hut giving similar results to direct method

#### Day 14: Better Time Integration
**Why:** Euler integration loses energy over time

**Implement:** Leapfrog (Kick-Drift-Kick) or Verlet integration

**Leapfrog Method:**
```cpp
// Half-step velocity update (kick)
vx += 0.5 * dt * ax;
vy += 0.5 * dt * ay;
vz += 0.5 * dt * az;

// Full-step position update (drift)
x += dt * vx;
y += dt * vy;
z += dt * vz;

// Recalculate forces with new positions
calculateForces();

// Half-step velocity update (kick)
vx += 0.5 * dt * ax;
vy += 0.5 * dt * ay;
vz += 0.5 * dt * az;
```

**Test:** Monitor total energy conservation

**Milestone:** Stable long-term simulation

---

### Week 3: Visualization + Testing (Days 15-21)

#### Day 15-17: Graphics Library Integration

**Option A: Raylib (RECOMMENDED - Easiest)**

Installation:
```bash
# Linux
sudo apt install libraylib-dev

# macOS
brew install raylib

# Or build from source (works everywhere)
git clone https://github.com/raysan5/raylib.git
cd raylib/src
make
```

Basic Visualization:
```cpp
#include "raylib.h"

int main() {
    InitWindow(800, 600, "N-Body Simulation");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // Update simulation
        updateParticles();

        BeginDrawing();
        ClearBackground(BLACK);

        // Draw particles
        for (auto& p : particles) {
            DrawCircle(p.x, p.y, 2, WHITE);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
```

**Option B: SFML**
- More features, slightly more complex
- Good documentation at sfml-dev.org

**Resources:**
- Raylib examples: raylib.com/examples.html
- Look at "particles" and "physics" examples

**Milestone:** Real-time visualization of particle motion

#### Day 18-20: Visualization Features
**Add:**
- Camera controls (zoom, pan)
- Performance metrics (FPS, particle count)
- Toggle between direct/Barnes-Hut
- Display octree boundaries (optional but cool)
- Particle trails (optional)

**Pro Tip:** Use different colors for different particle masses

**Milestone:** Interactive visualization with controls

#### Day 21: Testing & Optimization
**Tests to Run:**
1. **Correctness:** Compare direct vs Barnes-Hut for N=100
2. **Performance:** Time scaling from N=100 to N=10000
3. **Energy Conservation:** Plot total energy over time
4. **Edge Cases:**
   - Particles very close together
   - High-velocity particles
   - Escape velocity scenarios

**Profiling:**
```bash
# Linux/Mac
g++ -pg -O2 ...
./nbody
gprof nbody gmon.out > analysis.txt
```

**Milestone:** Verified O(n log n) performance, stable simulation

---

## CHECKPOINT (End of Week 3)

### Decision Time

**Questions to Ask Yourself:**
1. Does your Barnes-Hut simulation work correctly?
2. Can you explain the algorithm confidently?
3. How much time/energy do you have left?
4. Are you on schedule with other projects?

**Decision A: Proceed with Interactive Presentation**
- You feel good about the implementation
- You have time and motivation
- You want to create something special

**Decision B: PowerPoint + Code Demo**
- Implementation took longer than expected
- Other projects need attention
- Still get full marks with good explanation

**Both options are valid!** The code itself is impressive.

---

### Week 4-5: Interactive Presentation (IF PROCEEDING) (Days 22-35)

#### Day 22-24: Presentation Framework
**Concept:** Different "modes" or "slides" in your application

**Implementation:**
```cpp
enum PresentationMode {
    INTRO,
    DIRECT_METHOD,
    OCTREE_DEMO,
    BARNES_HUT_DEMO,
    THETA_COMPARISON,
    PERFORMANCE,
    CONCLUSION
};

PresentationMode currentMode = INTRO;

// In your main loop:
if (IsKeyPressed(KEY_RIGHT)) {
    currentMode = (PresentationMode)(currentMode + 1);
}
if (IsKeyPressed(KEY_LEFT)) {
    currentMode = (PresentationMode)(currentMode - 1);
}

// Render based on mode
switch(currentMode) {
    case INTRO:
        renderIntro();
        break;
    case DIRECT_METHOD:
        renderDirectMethodExplanation();
        break;
    // ...
}
```

**Milestone:** Navigate between different presentation states

#### Day 25-28: Interactive Elements
**Features to Add:**
1. **Real-time θ adjustment:**
   - Slider to change θ from 0.1 to 2.0
   - Show effect on accuracy/performance

2. **Visualize octree:**
   - Draw bounding boxes
   - Color-code by depth

3. **Highlight selected particle:**
   - Click particle to follow
   - Show forces acting on it

4. **Side-by-side comparison:**
   - Split screen: direct vs Barnes-Hut
   - Show same simulation with both methods

5. **Step-through mode:**
   - Pause and step frame-by-frame
   - Show algorithm execution

**UI Library:** Dear ImGui (optional but powerful)
```cpp
// Example slider for theta
ImGui::SliderFloat("Theta", &theta, 0.1f, 2.0f);
ImGui::Text("Particles: %d", particleCount);
ImGui::Text("FPS: %.1f", GetFPS());
```

**Milestone:** Interactive controls that affect simulation

#### Day 29-35: Content & Explanations
**Create Visual Explanations for:**

1. **Problem Statement:**
   - N-body problem
   - Why it's computationally expensive
   - Show O(n²) scaling visually

2. **Direct Method:**
   - Animation showing all pairwise calculations
   - Highlight the nested loops
   - Show why it's slow for large N

3. **Octree Structure:**
   - Build tree step-by-step
   - Show how particles are organized
   - Explain spatial hierarchy

4. **Barnes-Hut Approximation:**
   - Show distant cluster treated as single body
   - Demonstrate θ criterion
   - Compare exact vs approximate forces

5. **Performance Comparison:**
   - Graph of time vs N
   - Show O(n²) vs O(n log n)
   - Real measurements from your code

6. **Edge Cases:**
   - Close encounters (when approximation fails)
   - Empty octants
   - Single particles in large regions

**Text Overlays:**
Use simple text rendering to explain concepts on screen

**Milestone:** Complete interactive presentation

---

### Week 6: Polish & Practice (Days 36-42)

#### Day 36-38: Polish
**Code:**
- Clean up and comment
- Remove debug code
- Add command-line arguments
- Create README with instructions

**Presentation:**
- Test all modes work smoothly
- Ensure smooth transitions
- Add keyboard shortcuts reference
- Test on different machines/resolutions

**Documentation:**
```markdown
# N-Body Simulation - Barnes-Hut Algorithm

## Controls
- Arrow Keys: Navigate slides
- Space: Pause/Resume
- R: Reset simulation
- +/-: Zoom
- Mouse: Pan camera
- 1-5: Different scenarios

## Building
mkdir build && cd build
cmake ..
make
./nbody
```

#### Day 39-42: Presentation Practice
**Prepare to Explain:**
1. **Algorithm Overview (5 min)**
   - Problem statement
   - Why Barnes-Hut is needed
   - High-level approach

2. **Technical Details (10 min)**
   - Octree construction
   - Center of mass calculation
   - θ criterion and trade-offs
   - Force calculation recursion

3. **Implementation (5 min)**
   - Key data structures
   - Main algorithm loops
   - Challenges faced

4. **Results & Demo (5 min)**
   - Performance measurements
   - Interactive demonstration
   - Edge cases handled

5. **Q&A (5 min)**
   - Prepare for common questions

**Common Questions to Prepare For:**
- Why choose θ = X value?
- What happens when particles collide?
- How does performance scale with N?
- What about 3D vs 2D?
- How accurate is the approximation?
- Could you use GPU acceleration?

**Practice Run:**
- Time yourself (aim for 25-30 min total)
- Record and review
- Practice on a friend/roommate

**Milestone:** Confident, polished presentation ready!

---

## Critical Barnes-Hut Resources

### Must-Read Papers
1. **Barnes, J., & Hut, P. (1986).** "A hierarchical O(N log N) force-calculation algorithm"
   - Nature, 324(6096), 446-449
   - THE original paper - read this first

2. **Salmon, J. K., & Warren, M. S. (1994).** "Skeletons from the treecode closet"
   - Practical implementation details

### Best Tutorials
1. **Tom Quinn's Tutorial:** "An Introduction to SPH and the Hierarchical Tree Method"
   - Google Scholar or university website

2. **Philip Mocz:** "Create Your Own N-body Simulation"
   - Has Python example, but concepts apply to C++
   - arXiv or Medium

3. **Rosetta Code:** Barnes-Hut simulation examples
   - Multiple language implementations
   - Good for comparing approaches

### Video Resources
- Search YouTube: "Barnes Hut algorithm visualization"
- MIT OpenCourseWare: Computational Science lectures
- Computer Graphics course lectures on spatial data structures

### Online References
- Wikipedia: "Barnes-Hut simulation"
- cppreference.com: C++ standard library reference
- learncpp.com: Comprehensive C++ tutorials

### Books (Optional)
- "The Art of Computational Science" - Piet Hut & Jun Makino
  - Free online at artcompsci.org
  - Comprehensive N-body methods

- "Computer Simulation Methods" - Gould & Tobochnik
  - Good physics background

---

## Key Barnes-Hut Concepts to Master

### 1. Multipole Expansion
**Why it works:**
- Distant clusters of particles can be approximated as single body
- Error decreases with distance
- Center of mass represents the cluster

**Math:**
- Gravitational potential: φ ≈ GM/r for distant cluster
- Error is O((size/distance)²)

### 2. θ (Theta) Criterion
**Definition:** `size / distance < θ`
- `size`: dimension of octree node
- `distance`: from particle to node's center of mass
- `θ`: user-defined accuracy parameter

**Trade-offs:**
- Small θ (e.g., 0.3): More accurate, slower (O(n²) in limit)
- Large θ (e.g., 1.5): Less accurate, faster (O(n) in limit)
- Sweet spot: θ = 0.5 to 1.0 gives O(n log n) with good accuracy

**Typical values:**
- θ = 0.5: High accuracy, astrophysics standard
- θ = 1.0: Good balance, common default
- θ = 1.5: Fast approximation, lower accuracy

### 3. Tree Building - O(n log n)
**Steps:**
1. Find bounding box for all particles
2. For each particle:
   - Start at root
   - Navigate tree to find appropriate leaf
   - If leaf is empty, place particle
   - If leaf has particle, subdivide and redistribute
3. Calculate center of mass recursively

**Implementation tip:** Build new tree each timestep (simpler than updating)

### 4. Force Calculation - O(n log n)
**For each particle:**
```
function calculateForce(particle, node):
    if node is far enough (size/distance < θ):
        treat node as single body at center of mass
    else if node is leaf:
        calculate exact force with particle in leaf
    else:
        recursively call on all children
```

**Optimization:** Pre-calculate center of mass during tree build

### 5. Edge Cases to Handle

**Very close particles:**
- Add softening parameter: `F = GM₁M₂/(r² + ε²)^(3/2)`
- Prevents singularity when r → 0
- Typical ε = 0.01 × average_separation

**Empty octants:**
- Check for null children before recursion
- Don't allocate unnecessary nodes

**Periodic boundaries (optional):**
- For cosmological simulations
- Wrap coordinates at box edges

**High velocities:**
- Use adaptive timestep
- Or Leapfrog integration (more stable)

---

## Project Structure

### Recommended File Organization
```
nbody/
├── CMakeLists.txt
├── README.md
├── include/
│   ├── particle.h
│   ├── octree.h
│   ├── simulation.h
│   └── renderer.h
├── src/
│   ├── main.cpp
│   ├── particle.cpp
│   ├── octree.cpp
│   ├── simulation.cpp
│   └── renderer.cpp
├── data/
│   └── initial_conditions/
│       ├── galaxy.txt
│       └── solar_system.txt
└── build/
```

### CMakeLists.txt Template
```cmake
cmake_minimum_required(VERSION 3.10)
project(NBodySimulation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add executable
add_executable(nbody
    src/main.cpp
    src/particle.cpp
    src/octree.cpp
    src/simulation.cpp
    src/renderer.cpp
)

target_include_directories(nbody PRIVATE include)

# Find and link Raylib (if using)
find_package(raylib REQUIRED)
target_link_libraries(nbody raylib)

# Optimization flags
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(nbody PRIVATE -O3 -march=native)
endif()
```

---

## Development Tips

### 1. Start Simple, Build Up
- Get direct method working first
- Add visualization early (helps debugging)
- Then implement Barnes-Hut
- Test frequently

### 2. Version Control
```bash
git init
git add .
git commit -m "Initial commit"

# After each milestone:
git add .
git commit -m "Implemented octree structure"
```

**Commit after:**
- Direct method works
- Tree building works
- Barnes-Hut forces work
- Visualization added
- Each presentation mode

### 3. Testing Strategy
**Unit tests:**
- Tree correctly contains all particles
- Center of mass calculations are correct
- Force calculations match direct method for small N

**Integration tests:**
- Energy conservation over time
- Performance scaling measurements
- Edge case handling

### 4. Debugging Techniques
**For C++:**
- Use debugger (gdb or nvim-dap)
- Print octree structure
- Visualize tree boundaries
- Check energy conservation

**Common bugs:**
- Off-by-one in octant indexing
- Forgetting to update center of mass
- Wrong θ comparison (should be <, not >)
- Memory leaks in tree nodes

### 5. Performance Optimization
**Only after it works correctly!**
- Profile first: find actual bottlenecks
- Use -O3 compilation flag
- Consider OpenMP for parallelization
- Pre-allocate memory
- Cache-friendly data structures

### 6. Get Help When Stuck
**Resources:**
- StackOverflow (tag: c++, barnes-hut)
- Reddit: r/cpp, r/computergraphics
- Discord: C++ servers, graphics programming
- Office hours with professor/TA

**Time rule:** If stuck for >2 hours, ask for help!

---

## Fallback Plan

### If Behind Schedule

**At Week 3 Checkpoint:**

**Option 1: Simplify Presentation**
- Skip interactive features
- Use PowerPoint/PDF
- Run simulation as separate demo
- Still impressive with code explanation

**Option 2: Simplify Implementation**
- Use 2D instead of 3D (quadtree vs octree)
- Fewer particles (still shows concept)
- Skip some edge cases
- Focus on core algorithm

**Option 3: Use Python**
- If C++ is taking too long
- Faster development
- Still learn Barnes-Hut deeply
- Can always port to C++ later

**Remember:** Understanding the algorithm deeply is most important!

---

## Presentation Grading Focus

### What Professors Look For

**Algorithm Understanding (40%):**
- Can you explain why O(n log n)?
- Do you understand the θ criterion?
- Can you explain trade-offs?
- Do you know edge cases?

**Implementation (30%):**
- Does it work correctly?
- Is code reasonably clean?
- Can you explain your choices?

**Presentation Quality (30%):**
- Clear explanations
- Good visuals/demos
- Engaging delivery
- Handles questions well

**Interactive presentation helps with presentation quality, but solid understanding matters most!**

---

## Final Checklist

### One Week Before Presentation
- [ ] Simulation runs correctly
- [ ] Can explain algorithm confidently
- [ ] Presentation/demo prepared
- [ ] Tested on presentation machine
- [ ] Backup plan (video recording) ready
- [ ] Slides/notes prepared
- [ ] Q&A answers practiced

### Day Before
- [ ] Get good sleep
- [ ] Test everything one more time
- [ ] Charge laptop fully
- [ ] Have backup on USB drive
- [ ] Review key concepts
- [ ] Relax and be confident

### During Presentation
- [ ] Speak clearly and slowly
- [ ] Make eye contact
- [ ] Show enthusiasm
- [ ] Don't panic if something breaks (have backup)
- [ ] Welcome questions
- [ ] Have fun with it!

---

## Additional Ideas (If Extra Time)

### Cool Features to Add
- Multiple scenarios (galaxy collision, solar system)
- Particle trails to show orbits
- Color-coding by velocity/energy
- Music/sound effects
- GPU acceleration (CUDA/OpenCL)
- Web version (C++ → WebAssembly)

### Extensions for Learning
- Implement tree update instead of rebuild
- Add different force laws
- Implement Fast Multipole Method
- Add collision detection
- Parallel version with OpenMP/MPI

---

## Conclusion

You've got this! The key is:
1. **Focus on Week 1-3 implementation first**
2. **Understand Barnes-Hut deeply**
3. **Make checkpoint decision wisely**
4. **Don't stress about making it perfect**

The fact that you're planning this carefully already puts you ahead. C++ + Barnes-Hut is a great learning combination. Even if you end up with PowerPoint instead of interactive presentation, implementing this algorithm in C++ is impressive.

**Remember:** The goal is to learn and pass the class. Everything beyond that is bonus!

Good luck! You can do this.

---

## Quick Reference Card

### Barnes-Hut Algorithm Summary
1. Build octree from particle positions O(n log n)
2. Calculate center of mass for each node O(n)
3. For each particle: O(n log n)
   - Traverse tree
   - If far (size/dist < θ): use center of mass
   - If near: recurse on children
   - If leaf: calculate exact force
4. Update positions with Leapfrog integration O(n)
5. Repeat

**Total:** O(n log n) per timestep

### Key Parameters
- **G:** Gravitational constant (usually 1 in simulation units)
- **θ:** Accuracy parameter (0.5-1.0 typical)
- **dt:** Timestep (0.001-0.01 typical)
- **ε:** Softening length (0.01 typical)
- **N:** Number of particles (100-10000)

### Essential Commands
```bash
# Build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make

# Run
./nbody

# Debug
cmake .. -DCMAKE_BUILD_TYPE=Debug
gdb ./nbody
```

---

**Last updated:** December 1, 2025
**Your deadline:** January 12, 2026
**You've got this!**
