#include "demos/octree_demo.h"
#include "simulation.h"
#include "algorithms/octree3d.h"
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <cstddef>  // For size_t on Windows/MSVC

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Constructor
AlgorithmDemoState::AlgorithmDemoState()
    : mode(DEMO_BRUTE_FORCE), currentParticleIndex(0), currentStep(0), currentParticle(0),
      theta(0.7), cameraAngleH(0.0f), cameraAngleV(0.5f), cameraDistance(80.0f) {
    reset();
}

void AlgorithmDemoState::reset() {
    allParticles.clear();
    activeParticles.clear();
    octreeNodes.clear();
    currentParticleIndex = 0;

    const int numParticles = 8;
    const double spread = 35.0;
    const double mass = 1000.0;

    std::srand(42);

    for (int i = 0; i < numParticles; i++) {
        Particle p;
        bool validPosition = false;
        int attempts = 0;

        while (!validPosition && attempts < 100) {
            double theta = (double)std::rand() / RAND_MAX * 2.0 * M_PI;
            double phi = (double)std::rand() / RAND_MAX * M_PI;
            double r = (double)std::rand() / RAND_MAX * spread;

            p.x = r * std::sin(phi) * std::cos(theta);
            p.y = r * std::sin(phi) * std::sin(theta);
            p.z = r * std::cos(phi);

            validPosition = true;
            for (size_t j = 0; j < allParticles.size(); j++) {
                double dx = p.x - allParticles[j].x;
                double dy = p.y - allParticles[j].y;
                double dz = p.z - allParticles[j].z;
                double dist = sqrt(dx*dx + dy*dy + dz*dz);
                if (dist < 8.0) {
                    validPosition = false;
                    break;
                }
            }
            attempts++;
        }

        p.mass = mass;
        p.vx = p.vy = p.vz = 0.0;
        allParticles.push_back(p);
    }
}

void AlgorithmDemoState::addNextParticle() {
    if (currentParticleIndex < (int)allParticles.size()) {
        activeParticles.push_back(allParticles[currentParticleIndex]);
        currentParticleIndex++;
        buildVisualOctree();
    }
}

void AlgorithmDemoState::buildVisualOctree() {
    octreeNodes.clear();
    if (activeParticles.empty()) return;

    VisualOctreeNode root;
    root.x_center = 0.0;
    root.y_center = 0.0;
    root.z_center = 0.0;
    root.size = 80.0;
    root.depth = 0;
    root.isLeaf = true;
    root.particleIndex = -1;
    octreeNodes.push_back(root);

    for (size_t i = 0; i < activeParticles.size(); i++) {
        insertParticleIntoOctree(0, i);
    }
}

void AlgorithmDemoState::insertParticleIntoOctree(int nodeIdx, int particleIdx) {
    if (nodeIdx >= (int)octreeNodes.size()) return;  // Safety check

    VisualOctreeNode& node = octreeNodes[nodeIdx];

    if (node.isLeaf && node.particleIndex == -1) {
        node.particleIndex = particleIdx;
        return;
    }

    if (node.isLeaf && node.particleIndex != -1) {
        int oldParticleIdx = node.particleIndex;
        octreeNodes[nodeIdx].particleIndex = -1;
        octreeNodes[nodeIdx].isLeaf = false;

        double halfSize = octreeNodes[nodeIdx].size / 2.0;
        double quarterSize = halfSize / 2.0;
        double x_center = octreeNodes[nodeIdx].x_center;
        double y_center = octreeNodes[nodeIdx].y_center;
        double z_center = octreeNodes[nodeIdx].z_center;
        int depth = octreeNodes[nodeIdx].depth;

        for (int i = 0; i < 8; i++) {
            VisualOctreeNode child;
            child.size = halfSize;
            child.depth = depth + 1;

            int xi = (i & 1);
            int yi = (i & 2) >> 1;
            int zi = (i & 4) >> 2;

            child.x_center = x_center + (xi ? quarterSize : -quarterSize);
            child.y_center = y_center + (yi ? quarterSize : -quarterSize);
            child.z_center = z_center + (zi ? quarterSize : -quarterSize);

            octreeNodes[nodeIdx].children[i] = octreeNodes.size();
            octreeNodes.push_back(child);  // This invalidates 'node' reference!
        }

        insertParticleIntoOctree(nodeIdx, oldParticleIdx);
    }

    // Re-fetch node data after potential vector reallocation
    const Particle& p = activeParticles[particleIdx];
    int octant = 0;
    if (p.x > octreeNodes[nodeIdx].x_center) octant |= 1;
    if (p.y > octreeNodes[nodeIdx].y_center) octant |= 2;
    if (p.z > octreeNodes[nodeIdx].z_center) octant |= 4;

    insertParticleIntoOctree(octreeNodes[nodeIdx].children[octant], particleIdx);
}

void initAlgorithmDemo(AlgorithmDemoState& state) {
    state.camera.position = (Vector3){ 40.0f, 30.0f, 40.0f };
    state.camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    state.camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    state.camera.fovy = 45.0f;
    state.camera.projection = CAMERA_PERSPECTIVE;
    state.reset();
}

void updateAlgorithmDemo(AlgorithmDemoState& state) {
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        Vector2 mouseDelta = GetMouseDelta();
        state.cameraAngleH -= mouseDelta.x * 0.01f;
        state.cameraAngleV += mouseDelta.y * 0.01f;

        if (state.cameraAngleV < -M_PI/2.0f + 0.1f) state.cameraAngleV = -M_PI/2.0f + 0.1f;
        if (state.cameraAngleV > M_PI/2.0f - 0.1f) state.cameraAngleV = M_PI/2.0f - 0.1f;
    }

    state.cameraDistance -= GetMouseWheelMove() * 5.0f;
    if (state.cameraDistance < 30.0f) state.cameraDistance = 30.0f;
    if (state.cameraDistance > 200.0f) state.cameraDistance = 200.0f;

    state.camera.position.x = state.camera.target.x + state.cameraDistance * cos(state.cameraAngleV) * cos(state.cameraAngleH);
    state.camera.position.z = state.camera.target.z + state.cameraDistance * cos(state.cameraAngleV) * sin(state.cameraAngleH);
    state.camera.position.y = state.camera.target.y + state.cameraDistance * sin(state.cameraAngleV);

    if (IsKeyPressed(KEY_ONE)) state.mode = DEMO_BRUTE_FORCE;
    if (IsKeyPressed(KEY_TWO)) state.mode = DEMO_OCTREE_BUILD;
    if (IsKeyPressed(KEY_THREE)) state.mode = DEMO_OCTREE_STRUCTURE;
    if (IsKeyPressed(KEY_FOUR)) state.mode = DEMO_BARNES_HUT_CALC;

    if (IsKeyPressed(KEY_SPACE)) {
        if (state.mode == DEMO_OCTREE_BUILD) {
            state.addNextParticle();
        } else if (state.mode == DEMO_BRUTE_FORCE) {
            state.currentStep = (state.currentStep + 1) % state.allParticles.size();
        } else if (state.mode == DEMO_BARNES_HUT_CALC) {
            state.currentParticle = (state.currentParticle + 1) % state.allParticles.size();
        }
    }

    if (IsKeyPressed(KEY_EQUAL)) {
        state.theta += 0.1;
        if (state.theta > 2.0) state.theta = 2.0;
    }
    if (IsKeyPressed(KEY_MINUS)) {
        state.theta -= 0.1;
        if (state.theta < 0.1) state.theta = 0.1;
    }

    if (IsKeyPressed(KEY_R)) {
        state.reset();
    }
}

void renderAlgorithmDemo(const AlgorithmDemoState& state, int screenWidth, int screenHeight, bool uiVisible) {
    BeginDrawing();
    ClearBackground(BLACK);

    BeginMode3D(state.camera);

    // Draw coordinate axes
    DrawLine3D((Vector3){-15, 0, 0}, (Vector3){15, 0, 0}, ColorAlpha(RED, 0.3f));
    DrawLine3D((Vector3){0, -15, 0}, (Vector3){0, 15, 0}, ColorAlpha(GREEN, 0.3f));
    DrawLine3D((Vector3){0, 0, -15}, (Vector3){0, 0, 15}, ColorAlpha(BLUE, 0.3f));

    // Draw octree nodes (progressive subdivision)
    if (state.mode == DEMO_BRUTE_FORCE) {
        // Draw all particles
        for (size_t i = 0; i < state.allParticles.size(); i++) {
            Vector3 pos = {(float)state.allParticles[i].x, (float)state.allParticles[i].y, (float)state.allParticles[i].z};
            Color pColor = (i == (size_t)state.currentStep) ? RED : YELLOW;
            float pSize = (i == (size_t)state.currentStep) ? 1.5f : 1.2f;
            DrawSphere(pos, pSize, pColor);

            // Draw particle number
            if (i == (size_t)state.currentStep) {
                DrawSphere(pos, 3.0f, ColorAlpha(RED, 0.2f));  // Highlight halo
            }
        }

        // Draw all pairwise interactions
        for (size_t i = 0; i < state.allParticles.size(); i++) {
            for (size_t j = i + 1; j < state.allParticles.size(); j++) {
                Vector3 p1 = {(float)state.allParticles[i].x, (float)state.allParticles[i].y, (float)state.allParticles[i].z};
                Vector3 p2 = {(float)state.allParticles[j].x, (float)state.allParticles[j].y, (float)state.allParticles[j].z};

                // Highlight interactions involving current particle
                Color lineColor;
                if (i == (size_t)state.currentStep || j == (size_t)state.currentStep) {
                    lineColor = ColorAlpha(SKYBLUE, 0.7f);
                } else {
                    lineColor = ColorAlpha(GRAY, 0.15f);
                }

                DrawLine3D(p1, p2, lineColor);
            }
        }
    }

    // === MODE: OCTREE BUILDING ===
    else if (state.mode == DEMO_OCTREE_BUILD) {
        // Draw all octree nodes (progressive subdivision)
        Color depthColors[] = {
            ColorAlpha(WHITE, 0.4f),     // Depth 0 - root
            ColorAlpha(SKYBLUE, 0.3f),   // Depth 1
            ColorAlpha(GREEN, 0.3f),     // Depth 2
            ColorAlpha(YELLOW, 0.3f),    // Depth 3
            ColorAlpha(ORANGE, 0.3f)     // Depth 4+
        };

        for (const auto& node : state.octreeNodes) {
            int colorIdx = node.depth < 5 ? node.depth : 4;
            drawOctreeNodeBox(node.x_center, node.y_center, node.z_center,
                            node.size, depthColors[colorIdx]);
        }

        // Draw active particles (already added)
        for (size_t i = 0; i < state.activeParticles.size(); i++) {
            Vector3 pos = {(float)state.activeParticles[i].x, (float)state.activeParticles[i].y, (float)state.activeParticles[i].z};
            DrawSphere(pos, 1.5f, YELLOW);

            // Draw particle label
            DrawSphere(pos, 2.5f, ColorAlpha(YELLOW, 0.2f));  // Glow effect
        }

        // Calculate and draw center of mass
        if (!state.activeParticles.empty()) {
            double cm_x = 0.0, cm_y = 0.0, cm_z = 0.0;
            double total_mass = 0.0;

            for (const auto& p : state.activeParticles) {
                cm_x += p.x * p.mass;
                cm_y += p.y * p.mass;
                cm_z += p.z * p.mass;
                total_mass += p.mass;
            }

            cm_x /= total_mass;
            cm_y /= total_mass;
            cm_z /= total_mass;

            Vector3 cmPos = {(float)cm_x, (float)cm_y, (float)cm_z};
            DrawSphere(cmPos, 2.5f, RED);  // Center of mass dot
            DrawSphere(cmPos, 4.0f, ColorAlpha(RED, 0.3f));  // Glow
        }

        // Draw next particle to be added (pulsing)
        if (state.currentParticleIndex < (int)state.allParticles.size()) {
            const Particle& nextP = state.allParticles[state.currentParticleIndex];
            Vector3 pos = {(float)nextP.x, (float)nextP.y, (float)nextP.z};

            // Pulsing effect
            float pulse = (sin(GetTime() * 5.0f) + 1.0f) / 2.0f;
            float radius = 1.5f + pulse * 1.0f;

            DrawSphere(pos, radius, Fade(GREEN, 0.7f));
            DrawSphere(pos, radius + 2.0f, Fade(GREEN, 0.2f));
        }
    }

    // === MODE: OCTREE STRUCTURE ===
    else if (state.mode == DEMO_OCTREE_STRUCTURE) {
        // Draw particles
        for (size_t i = 0; i < state.allParticles.size(); i++) {
            Vector3 pos = {(float)state.allParticles[i].x, (float)state.allParticles[i].y, (float)state.allParticles[i].z};
            DrawSphere(pos, 1.2f, YELLOW);
        }

        // Draw complete octree hierarchy
        // Root
        drawOctreeNodeBox(0, 0, 0, 80.0, ColorAlpha(WHITE, 0.3f));

        // Level 1
        double halfSize = 40.0;
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 2; j++) {
                for (int k = 0; k < 2; k++) {
                    double x = (i - 0.5) * halfSize;
                    double y = (j - 0.5) * halfSize;
                    double z = (k - 0.5) * halfSize;
                    drawOctreeNodeBox(x, y, z, halfSize, ColorAlpha(GREEN, 0.25f));
                }
            }
        }

        // Level 2 (leaf cells)
        double quarterSize = 10.0;  // Smaller for less clutter
        for (size_t i = 0; i < state.allParticles.size(); i++) {
            double x = state.allParticles[i].x;
            double y = state.allParticles[i].y;
            double z = state.allParticles[i].z;
            drawOctreeNodeBox(x, y, z, quarterSize, ColorAlpha(ORANGE, 0.4f));

            // Draw center of mass marker for each cell - MUCH BIGGER AND BRIGHTER
            Vector3 cm = {(float)x, (float)y, (float)z};
            DrawSphere(cm, 1.5f, RED);  // Increased from 0.3f to 1.5f
        }
    }

    // === MODE: BARNES-HUT CALCULATION ===
    else if (state.mode == DEMO_BARNES_HUT_CALC) {
        // Draw all particles (dimmed except current)
        for (size_t i = 0; i < state.allParticles.size(); i++) {
            Vector3 pos = {(float)state.allParticles[i].x, (float)state.allParticles[i].y, (float)state.allParticles[i].z};
            Color pColor = (i == (size_t)state.currentParticle) ? RED : ColorAlpha(GRAY, 0.6f);
            float pSize = (i == (size_t)state.currentParticle) ? 1.5f : 1.0f;
            DrawSphere(pos, pSize, pColor);
        }

        // Highlight current particle
        Vector3 currentPos = {
            (float)state.allParticles[state.currentParticle].x,
            (float)state.allParticles[state.currentParticle].y,
            (float)state.allParticles[state.currentParticle].z
        };
        DrawSphere(currentPos, 3.0f, ColorAlpha(RED, 0.2f));

        // THIS IS THE KEY DIFFERENCE FROM BRUTE FORCE:
        // Instead of computing forces with every particle individually,
        // Barnes-Hut uses CELLS from the octree structure

        // We'll demonstrate by checking octree cells at different levels
        // and showing which ones can be approximated vs which need subdivision

        double currentX = state.allParticles[state.currentParticle].x;
        double currentY = state.allParticles[state.currentParticle].y;
        double currentZ = state.allParticles[state.currentParticle].z;

        // Check Level 1 cells (8 large cells)
        double halfSize = 40.0;
        int cellsUsed = 0;
        int cellsSubdivided = 0;

        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 2; j++) {
                for (int k = 0; k < 2; k++) {
                    double cellX = (i - 0.5) * halfSize;
                    double cellY = (j - 0.5) * halfSize;
                    double cellZ = (k - 0.5) * halfSize;

                    // Distance from current particle to cell center
                    double dx = cellX - currentX;
                    double dy = cellY - currentY;
                    double dz = cellZ - currentZ;
                    double dist = sqrt(dx*dx + dy*dy + dz*dz);

                    if (dist < 0.1) continue; // Skip if current particle is in this cell

                    // Barnes-Hut criterion: size / distance
                    double ratio = halfSize / dist;

                    Color cellColor;
                    if (ratio < state.theta) {
                        // PASS: Can use this entire cell as a single point!
                        // This is the approximation - treat all particles in cell as one mass
                        cellColor = ColorAlpha(GREEN, 0.6f);
                        cellsUsed++;

                        // Draw line to cell's center of mass
                        Vector3 cellCenter = {(float)cellX, (float)cellY, (float)cellZ};
                        DrawLine3D(currentPos, cellCenter, ColorAlpha(GREEN, 0.7f));
                        DrawSphere(cellCenter, 1.5f, GREEN); // Show center of mass point - BIGGER
                    } else {
                        // FAIL: Cell too close, must subdivide further
                        cellColor = ColorAlpha(ORANGE, 0.4f);
                        cellsSubdivided++;
                    }

                    drawOctreeNodeBox(cellX, cellY, cellZ, halfSize, cellColor);
                }
            }
        }

        // For cells that need subdivision, show the leaf level
        double quarterSize = 10.0;  // Smaller for less clutter
        for (size_t i = 0; i < state.allParticles.size(); i++) {
            if (i == (size_t)state.currentParticle) continue;

            double dx = state.allParticles[i].x - currentX;
            double dy = state.allParticles[i].y - currentY;
            double dz = state.allParticles[i].z - currentZ;
            double dist = sqrt(dx*dx + dy*dy + dz*dz);

            // Check if this leaf cell is in a subdivided parent
            double ratio = quarterSize / dist;

            // Only show if parent was subdivided (close enough)
            if (halfSize / dist >= state.theta) {
                Color leafColor;
                if (ratio < state.theta) {
                    // Can use leaf cell
                    leafColor = ColorAlpha(GREEN, 0.5f);
                    Vector3 leafCenter = {(float)state.allParticles[i].x, (float)state.allParticles[i].y, (float)state.allParticles[i].z};
                    DrawLine3D(currentPos, leafCenter, ColorAlpha(GREEN, 0.5f));
                } else {
                    // Must compute exactly with this particle
                    leafColor = ColorAlpha(YELLOW, 0.5f);
                    Vector3 particlePos = {(float)state.allParticles[i].x, (float)state.allParticles[i].y, (float)state.allParticles[i].z};
                    DrawLine3D(currentPos, particlePos, ColorAlpha(YELLOW, 0.6f));
                }

                drawOctreeNodeBox(state.allParticles[i].x, state.allParticles[i].y, state.allParticles[i].z,
                                quarterSize, leafColor);
            }
        }

        // Draw info about approximations being used
        if (uiVisible) {
            DrawText(TextFormat("Cells approximated: %d", cellsUsed), screenWidth - 250, 20, 14, GREEN);
            DrawText(TextFormat("Cells subdivided: %d", cellsSubdivided), screenWidth - 250, 40, 14, ORANGE);
        }
    }

    EndMode3D();

    // === UI OVERLAY ===
    if (uiVisible) {
        const char* modeNames[] = {
            "MODE 1: BRUTE FORCE O(n²)",
            "MODE 2: OCTREE CONSTRUCTION",
            "MODE 3: OCTREE STRUCTURE",
            "MODE 4: BARNES-HUT O(n log n)"
        };

        // Top info panel - larger and more detailed
        DrawRectangle(0, 0, screenWidth, 280, ColorAlpha(BLACK, 0.85f));
        DrawText("ALGORITHM VISUAL WALKTHROUGH", 20, 10, 26, WHITE);
        DrawText(modeNames[state.mode], 20, 45, 20, YELLOW);
        DrawText(TextFormat("Particles: %d", (int)state.allParticles.size()), 20, 75, 16, LIGHTGRAY);

        // Mode-specific detailed explanations
        if (state.mode == DEMO_BRUTE_FORCE) {
            int totalCalcs = (state.allParticles.size() * (state.allParticles.size() - 1)) / 2;
            DrawText(TextFormat("Total Calculations: %d (every particle pair!)", totalCalcs), 20, 105, 18, ORANGE);
            DrawText(TextFormat("Current: Computing forces for particle %d/%d", state.currentStep + 1, (int)state.allParticles.size()), 20, 130, 16, SKYBLUE);

            DrawText("HOW IT WORKS:", 20, 160, 16, WHITE);
            DrawText("• Each particle must calculate force from EVERY other particle", 20, 180, 14, GRAY);
            DrawText("• Complexity: O(n²) - doubles particles = 4x computation!", 20, 200, 14, GRAY);
            DrawText("• Blue lines show forces being calculated for current particle", 20, 220, 14, SKYBLUE);
            DrawText("• Gray lines show all other particle pairs", 20, 240, 14, GRAY);
            DrawText("• Press SPACE to step through each particle", 20, 260, 13, GREEN);
        }
        else if (state.mode == DEMO_OCTREE_BUILD) {
            DrawText(TextFormat("Particles Added: %d/%d  |  Tree Nodes: %d",
                     state.currentParticleIndex, (int)state.allParticles.size(),
                     (int)state.octreeNodes.size()),
                     20, 105, 18, ORANGE);

            DrawText("HOW IT WORKS:", 20, 135, 16, WHITE);
            DrawText("• Starts with 1 WHITE root box containing entire space", 20, 155, 14, WHITE);
            DrawText("• When 2 particles in same box: box SPLITS into 8 children", 20, 175, 14, SKYBLUE);
            DrawText("• YELLOW spheres: Particles already added to the tree", 20, 195, 14, YELLOW);
            DrawText("• RED sphere: Center of mass of all added particles", 20, 215, 14, RED);
            DrawText("• GREEN pulsing sphere: Next particle to be added", 20, 235, 14, GREEN);
            DrawText("• Press SPACE to add next particle and watch tree grow!", 20, 255, 13, GREEN);
        }
        else if (state.mode == DEMO_OCTREE_STRUCTURE) {
            DrawText("Complete Hierarchical Tree", 20, 105, 18, ORANGE);

            DrawText("HOW IT WORKS:", 20, 135, 16, WHITE);
            DrawText("• WHITE box: Root - represents entire simulation space", 20, 155, 14, WHITE);
            DrawText("• GREEN boxes: Level 1 - 8 subdivisions of root", 20, 175, 14, GREEN);
            DrawText("• ORANGE boxes: Level 2 - final leaf cells containing particles", 20, 195, 14, ORANGE);
            DrawText("• RED dots: Center of mass for each cell (used in approximation)", 20, 215, 14, RED);
            DrawText("• Far particles grouped together → treated as single mass point", 20, 235, 14, GRAY);
            DrawText("• This hierarchy reduces computation from O(n²) to O(n log n)", 20, 255, 14, GRAY);
        }
        else if (state.mode == DEMO_BARNES_HUT_CALC) {
            DrawText(TextFormat("Computing forces for particle: %d/%d", state.currentParticle + 1, (int)state.allParticles.size()),
                     20, 105, 18, ORANGE);
            DrawText(TextFormat("Theta: %.2f (adjust with +/-)", state.theta), 20, 130, 16, LIGHTGRAY);

            DrawText("HOW IT WORKS (THE KEY DIFFERENCE):", 20, 160, 16, WHITE);
            DrawText("• Instead of computing with every particle (brute force)...", 20, 180, 14, GRAY);
            DrawText("• Barnes-Hut uses CELLS to approximate groups of distant particles!", 20, 200, 14, GRAY);
            DrawText("• GREEN cells/spheres: Far enough → treat entire cell as ONE point", 20, 220, 14, GREEN);
            DrawText("• ORANGE boxes: Too close → must subdivide to get better detail", 20, 240, 14, ORANGE);
            DrawText("• YELLOW: Individual particles computed exactly (can't approximate)", 20, 260, 14, YELLOW);
        }

        // Controls overlay (bottom)
        DrawRectangle(0, screenHeight - 150, screenWidth, 150, ColorAlpha(BLACK, 0.7f));
        DrawText("CONTROLS:", 20, screenHeight - 140, 18, WHITE);
        DrawText("1: Brute Force | 2: Build Tree | 3: Tree Structure | 4: Barnes-Hut", 20, screenHeight - 115, 14, GRAY);
        DrawText("SPACE: Next Step | R: Reset | +/-: Adjust Theta", 20, screenHeight - 95, 14, GRAY);
        DrawText("Left Click: Rotate | Middle Click: Pan | Wheel: Zoom", 20, screenHeight - 75, 14, GRAY);
        DrawText("H: Hide UI | Q: Return to Menu | ESC: Exit Program", 20, screenHeight - 55, 14, GRAY);

        DrawText(TextFormat("FPS: %d", GetFPS()), screenWidth - 100, 20, 18, GREEN);
    }

    EndDrawing();
}

// Helper function implementations
void drawOctreeNodeBox(double x, double y, double z, double size, Color color) {
    double half = size / 2.0;

    Vector3 corners[8] = {
        {(float)(x - half), (float)(y - half), (float)(z - half)},
        {(float)(x + half), (float)(y - half), (float)(z - half)},
        {(float)(x + half), (float)(y + half), (float)(z - half)},
        {(float)(x - half), (float)(y + half), (float)(z - half)},
        {(float)(x - half), (float)(y - half), (float)(z + half)},
        {(float)(x + half), (float)(y - half), (float)(z + half)},
        {(float)(x + half), (float)(y + half), (float)(z + half)},
        {(float)(x - half), (float)(y + half), (float)(z + half)}
    };

    // Bottom face
    DrawLine3D(corners[0], corners[1], color);
    DrawLine3D(corners[1], corners[2], color);
    DrawLine3D(corners[2], corners[3], color);
    DrawLine3D(corners[3], corners[0], color);

    // Top face
    DrawLine3D(corners[4], corners[5], color);
    DrawLine3D(corners[5], corners[6], color);
    DrawLine3D(corners[6], corners[7], color);
    DrawLine3D(corners[7], corners[4], color);

    // Vertical edges
    DrawLine3D(corners[0], corners[4], color);
    DrawLine3D(corners[1], corners[5], color);
    DrawLine3D(corners[2], corners[6], color);
    DrawLine3D(corners[3], corners[7], color);
}

void drawForceInteraction(Vector3 p1, Vector3 p2, Color color, float thickness) {
    DrawLine3D(p1, p2, color);
}

void drawParticleWithLabel(const Particle& p, int index, Color color, float size) {
    Vector3 pos = {(float)p.x, (float)p.y, (float)p.z};
    DrawSphere(pos, size, color);
}
