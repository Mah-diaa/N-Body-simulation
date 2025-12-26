#include "algorithm_demo.h"
#include "simulation.h"
#include "octree_optimized.h"
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <cstddef>  // For size_t on Windows/MSVC

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Constructor
AlgorithmDemoState::AlgorithmDemoState()
    : mode(DEMO_BRUTE_FORCE), currentStep(0), currentParticle(0),
      theta(0.7), isPaused(true), cameraAngleH(0.0f), cameraAngleV(0.5f), cameraDistance(80.0f) {
    reset();
}

void AlgorithmDemoState::reset() {
    particles.clear();
    currentStep = 0;
    currentParticle = 0;

    // Create randomly distributed particles for clearer visualization
    const int numParticles = 15;  // Fewer particles for less clutter
    const double spread = 35.0;  // Larger spread for better visibility
    const double mass = 1000.0;

    // Seed for consistent random positions
    std::srand(42);

    for (int i = 0; i < numParticles; i++) {
        Particle p;
        // Random positions with minimum separation
        bool validPosition = false;
        int attempts = 0;

        while (!validPosition && attempts < 100) {
            // Random positions in a sphere
            double theta = (double)std::rand() / RAND_MAX * 2.0 * M_PI;
            double phi = (double)std::rand() / RAND_MAX * M_PI;
            double r = (double)std::rand() / RAND_MAX * spread;

            p.x = r * std::sin(phi) * std::cos(theta);
            p.y = r * std::sin(phi) * std::sin(theta);
            p.z = r * std::cos(phi);

            // Check minimum distance from other particles
            validPosition = true;
            for (size_t j = 0; j < particles.size(); j++) {
                double dx = p.x - particles[j].x;
                double dy = p.y - particles[j].y;
                double dz = p.z - particles[j].z;
                double dist = sqrt(dx*dx + dy*dy + dz*dz);
                if (dist < 8.0) {  // Minimum separation
                    validPosition = false;
                    break;
                }
            }
            attempts++;
        }

        p.mass = mass;
        p.vx = p.vy = p.vz = 0.0;
        particles.push_back(p);
    }
}

void initAlgorithmDemo(AlgorithmDemoState& state) {
    // Initialize camera
    state.camera.position = (Vector3){ 40.0f, 30.0f, 40.0f };
    state.camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    state.camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    state.camera.fovy = 45.0f;
    state.camera.projection = CAMERA_PERSPECTIVE;

    state.reset();
}

void updateAlgorithmDemo(AlgorithmDemoState& state) {
    // Camera control - left mouse drag for rotation
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        Vector2 mouseDelta = GetMouseDelta();
        state.cameraAngleH -= mouseDelta.x * 0.01f;
        state.cameraAngleV += mouseDelta.y * 0.01f;

        // Clamp vertical angle to prevent flipping
        if (state.cameraAngleV < -M_PI/2.0f + 0.1f) state.cameraAngleV = -M_PI/2.0f + 0.1f;
        if (state.cameraAngleV > M_PI/2.0f - 0.1f) state.cameraAngleV = M_PI/2.0f - 0.1f;
    }

    // Middle mouse button (scroll wheel press) for panning
    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
        Vector2 mouseDelta = GetMouseDelta();

        // Calculate camera right and up vectors for proper panning
        Vector3 forward = {
            state.camera.target.x - state.camera.position.x,
            state.camera.target.y - state.camera.position.y,
            state.camera.target.z - state.camera.position.z
        };

        // Right vector (perpendicular to forward and up)
        Vector3 right = {
            (float)cos(state.cameraAngleH - M_PI/2.0f),
            0.0f,
            (float)sin(state.cameraAngleH - M_PI/2.0f)
        };

        // Up vector (perpendicular to forward and right)
        Vector3 up = {
            (float)(-sin(state.cameraAngleV) * cos(state.cameraAngleH)),
            (float)cos(state.cameraAngleV),
            (float)(-sin(state.cameraAngleV) * sin(state.cameraAngleH))
        };

        // Pan speed based on distance
        float panSpeed = state.cameraDistance * 0.002f;

        // Move target (and position to maintain orbit)
        state.camera.target.x -= right.x * mouseDelta.x * panSpeed;
        state.camera.target.y += up.y * mouseDelta.y * panSpeed;
        state.camera.target.z -= right.z * mouseDelta.x * panSpeed;
    }

    // Mouse wheel for zoom
    state.cameraDistance -= GetMouseWheelMove() * 5.0f;
    if (state.cameraDistance < 30.0f) state.cameraDistance = 30.0f;
    if (state.cameraDistance > 200.0f) state.cameraDistance = 200.0f;

    // Update camera position with both horizontal and vertical rotation around target
    state.camera.position.x = state.camera.target.x + state.cameraDistance * cos(state.cameraAngleV) * cos(state.cameraAngleH);
    state.camera.position.z = state.camera.target.z + state.cameraDistance * cos(state.cameraAngleV) * sin(state.cameraAngleH);
    state.camera.position.y = state.camera.target.y + state.cameraDistance * sin(state.cameraAngleV);

    // Mode switching
    if (IsKeyPressed(KEY_ONE)) {
        state.mode = DEMO_BRUTE_FORCE;
        state.currentStep = 0;
    }
    if (IsKeyPressed(KEY_TWO)) {
        state.mode = DEMO_OCTREE_BUILD;
        state.currentStep = 0;
    }
    if (IsKeyPressed(KEY_THREE)) {
        state.mode = DEMO_OCTREE_STRUCTURE;
    }
    if (IsKeyPressed(KEY_FOUR)) {
        state.mode = DEMO_BARNES_HUT_CALC;
        state.currentParticle = 0;
    }

    // Step through
    if (IsKeyPressed(KEY_SPACE)) {
        if (state.mode == DEMO_BRUTE_FORCE) {
            state.currentStep++;
            if (state.currentStep >= (int)state.particles.size()) {
                state.currentStep = 0;
            }
        }
        else if (state.mode == DEMO_BARNES_HUT_CALC) {
            state.currentParticle++;
            if (state.currentParticle >= (int)state.particles.size()) {
                state.currentParticle = 0;
            }
        }
        else if (state.mode == DEMO_OCTREE_BUILD) {
            state.currentStep++;
        }
    }

    // Theta adjustment
    if (IsKeyPressed(KEY_EQUAL)) {
        state.theta += 0.1;
        if (state.theta > 2.0) state.theta = 2.0;
    }
    if (IsKeyPressed(KEY_MINUS)) {
        state.theta -= 0.1;
        if (state.theta < 0.1) state.theta = 0.1;
    }

    // Reset
    if (IsKeyPressed(KEY_R)) {
        state.reset();
    }
}

void renderAlgorithmDemo(const AlgorithmDemoState& state, int screenWidth, int screenHeight, bool uiVisible) {
    BeginDrawing();
    ClearBackground(BLACK);

    BeginMode3D(state.camera);

    // Draw coordinate axes (subtle)
    DrawLine3D((Vector3){-15, 0, 0}, (Vector3){15, 0, 0}, ColorAlpha(RED, 0.3f));
    DrawLine3D((Vector3){0, -15, 0}, (Vector3){0, 15, 0}, ColorAlpha(GREEN, 0.3f));
    DrawLine3D((Vector3){0, 0, -15}, (Vector3){0, 0, 15}, ColorAlpha(BLUE, 0.3f));

    // === MODE: BRUTE FORCE ===
    if (state.mode == DEMO_BRUTE_FORCE) {
        // Draw all particles
        for (size_t i = 0; i < state.particles.size(); i++) {
            Vector3 pos = {(float)state.particles[i].x, (float)state.particles[i].y, (float)state.particles[i].z};
            Color pColor = (i == (size_t)state.currentStep) ? RED : YELLOW;
            float pSize = (i == (size_t)state.currentStep) ? 1.5f : 1.2f;
            DrawSphere(pos, pSize, pColor);

            // Draw particle number
            if (i == (size_t)state.currentStep) {
                DrawSphere(pos, 3.0f, ColorAlpha(RED, 0.2f));  // Highlight halo
            }
        }

        // Draw all pairwise interactions
        for (size_t i = 0; i < state.particles.size(); i++) {
            for (size_t j = i + 1; j < state.particles.size(); j++) {
                Vector3 p1 = {(float)state.particles[i].x, (float)state.particles[i].y, (float)state.particles[i].z};
                Vector3 p2 = {(float)state.particles[j].x, (float)state.particles[j].y, (float)state.particles[j].z};

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
        // Draw particles
        for (size_t i = 0; i < state.particles.size(); i++) {
            Vector3 pos = {(float)state.particles[i].x, (float)state.particles[i].y, (float)state.particles[i].z};
            DrawSphere(pos, 1.2f, YELLOW);
        }

        // Show progressive octree building
        // Root bounding box - large enough to contain all particles
        drawOctreeNodeBox(0, 0, 0, 80.0, ColorAlpha(WHITE, 0.4f));

        if (state.currentStep >= 1) {
            // Level 1 subdivisions
            double halfSize = 40.0;
            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 2; j++) {
                    for (int k = 0; k < 2; k++) {
                        double x = (i - 0.5) * halfSize;
                        double y = (j - 0.5) * halfSize;
                        double z = (k - 0.5) * halfSize;
                        drawOctreeNodeBox(x, y, z, halfSize, ColorAlpha(GREEN, 0.3f));
                    }
                }
            }
        }

        if (state.currentStep >= 2) {
            // Level 2 subdivisions (leaf cells with particles)
            double quarterSize = 10.0;  // Smaller for less clutter
            for (size_t i = 0; i < state.particles.size(); i++) {
                double x = state.particles[i].x;
                double y = state.particles[i].y;
                double z = state.particles[i].z;
                drawOctreeNodeBox(x, y, z, quarterSize, ColorAlpha(ORANGE, 0.5f));
            }
        }
    }

    // === MODE: OCTREE STRUCTURE ===
    else if (state.mode == DEMO_OCTREE_STRUCTURE) {
        // Draw particles
        for (size_t i = 0; i < state.particles.size(); i++) {
            Vector3 pos = {(float)state.particles[i].x, (float)state.particles[i].y, (float)state.particles[i].z};
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
        for (size_t i = 0; i < state.particles.size(); i++) {
            double x = state.particles[i].x;
            double y = state.particles[i].y;
            double z = state.particles[i].z;
            drawOctreeNodeBox(x, y, z, quarterSize, ColorAlpha(ORANGE, 0.4f));

            // Draw center of mass marker for each cell - MUCH BIGGER AND BRIGHTER
            Vector3 cm = {(float)x, (float)y, (float)z};
            DrawSphere(cm, 1.5f, RED);  // Increased from 0.3f to 1.5f
        }
    }

    // === MODE: BARNES-HUT CALCULATION ===
    else if (state.mode == DEMO_BARNES_HUT_CALC) {
        // Draw all particles (dimmed except current)
        for (size_t i = 0; i < state.particles.size(); i++) {
            Vector3 pos = {(float)state.particles[i].x, (float)state.particles[i].y, (float)state.particles[i].z};
            Color pColor = (i == (size_t)state.currentParticle) ? RED : ColorAlpha(GRAY, 0.6f);
            float pSize = (i == (size_t)state.currentParticle) ? 1.5f : 1.0f;
            DrawSphere(pos, pSize, pColor);
        }

        // Highlight current particle
        Vector3 currentPos = {
            (float)state.particles[state.currentParticle].x,
            (float)state.particles[state.currentParticle].y,
            (float)state.particles[state.currentParticle].z
        };
        DrawSphere(currentPos, 3.0f, ColorAlpha(RED, 0.2f));

        // THIS IS THE KEY DIFFERENCE FROM BRUTE FORCE:
        // Instead of computing forces with every particle individually,
        // Barnes-Hut uses CELLS from the octree structure

        // We'll demonstrate by checking octree cells at different levels
        // and showing which ones can be approximated vs which need subdivision

        double currentX = state.particles[state.currentParticle].x;
        double currentY = state.particles[state.currentParticle].y;
        double currentZ = state.particles[state.currentParticle].z;

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
        for (size_t i = 0; i < state.particles.size(); i++) {
            if (i == (size_t)state.currentParticle) continue;

            double dx = state.particles[i].x - currentX;
            double dy = state.particles[i].y - currentY;
            double dz = state.particles[i].z - currentZ;
            double dist = sqrt(dx*dx + dy*dy + dz*dz);

            // Check if this leaf cell is in a subdivided parent
            double ratio = quarterSize / dist;

            // Only show if parent was subdivided (close enough)
            if (halfSize / dist >= state.theta) {
                Color leafColor;
                if (ratio < state.theta) {
                    // Can use leaf cell
                    leafColor = ColorAlpha(GREEN, 0.5f);
                    Vector3 leafCenter = {(float)state.particles[i].x, (float)state.particles[i].y, (float)state.particles[i].z};
                    DrawLine3D(currentPos, leafCenter, ColorAlpha(GREEN, 0.5f));
                } else {
                    // Must compute exactly with this particle
                    leafColor = ColorAlpha(YELLOW, 0.5f);
                    Vector3 particlePos = {(float)state.particles[i].x, (float)state.particles[i].y, (float)state.particles[i].z};
                    DrawLine3D(currentPos, particlePos, ColorAlpha(YELLOW, 0.6f));
                }

                drawOctreeNodeBox(state.particles[i].x, state.particles[i].y, state.particles[i].z,
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
        DrawText(TextFormat("Particles: %d", (int)state.particles.size()), 20, 75, 16, LIGHTGRAY);

        // Mode-specific detailed explanations
        if (state.mode == DEMO_BRUTE_FORCE) {
            int totalCalcs = (state.particles.size() * (state.particles.size() - 1)) / 2;
            DrawText(TextFormat("Total Calculations: %d (every particle pair!)", totalCalcs), 20, 105, 18, ORANGE);
            DrawText(TextFormat("Current: Computing forces for particle %d/%d", state.currentStep + 1, (int)state.particles.size()), 20, 130, 16, SKYBLUE);

            DrawText("HOW IT WORKS:", 20, 160, 16, WHITE);
            DrawText("• Each particle must calculate force from EVERY other particle", 20, 180, 14, GRAY);
            DrawText("• Complexity: O(n²) - doubles particles = 4x computation!", 20, 200, 14, GRAY);
            DrawText("• Blue lines show forces being calculated for current particle", 20, 220, 14, SKYBLUE);
            DrawText("• Gray lines show all other particle pairs", 20, 240, 14, GRAY);
            DrawText("• Press SPACE to step through each particle", 20, 260, 13, GREEN);
        }
        else if (state.mode == DEMO_OCTREE_BUILD) {
            DrawText(TextFormat("Build Step: %d/3", std::min(state.currentStep, 2) + 1), 20, 105, 18, ORANGE);

            DrawText("HOW IT WORKS:", 20, 135, 16, WHITE);
            DrawText("• Step 1: Create root bounding box (WHITE) containing all particles", 20, 155, 14, GRAY);
            DrawText("• Step 2: Subdivide into 8 children (GREEN) - octree gets its name here!", 20, 175, 14, GRAY);
            DrawText("• Step 3: Further subdivide cells with particles (ORANGE leaf cells)", 20, 195, 14, GRAY);
            DrawText("• Each cell stores total mass & center of mass of particles inside", 20, 215, 14, GRAY);
            DrawText("• This spatial organization enables fast approximations", 20, 235, 14, GRAY);
            DrawText("• Press SPACE to see each subdivision step", 20, 255, 13, GREEN);
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
            DrawText(TextFormat("Computing forces for particle: %d/%d", state.currentParticle + 1, (int)state.particles.size()),
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
