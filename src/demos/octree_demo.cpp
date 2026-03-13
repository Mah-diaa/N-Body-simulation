#include "demos/octree_demo.h"
#include "simulation.h"
#include "algorithms/octree3d.h"
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <cstddef>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Demo initialization
AlgorithmDemoState::AlgorithmDemoState()
    : mode(DEMO_BRUTE_FORCE), currentParticleIndex(0), currentStep(0),
      theta(0.7), cameraAngleH(0.0f), cameraAngleV(0.5f), cameraDistance(80.0f), lightMode(false),
      currentTraversalStep(0), traversalComplete(false) {
    reset();
}

void AlgorithmDemoState::reset() {
    allParticles.clear();
    activeParticles.clear();
    octreeNodes.clear();
    traversalSteps.clear();
    currentParticleIndex = 0;
    currentTraversalStep = 0;
    traversalComplete = false;

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

// Octree visualization
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
    if (nodeIdx >= (int)octreeNodes.size()) return;

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
            octreeNodes.push_back(child);
        }

        insertParticleIntoOctree(nodeIdx, oldParticleIdx);
    }

    const Particle& p = activeParticles[particleIdx];
    int octant = 0;
    if (p.x > octreeNodes[nodeIdx].x_center) octant |= 1;
    if (p.y > octreeNodes[nodeIdx].y_center) octant |= 2;
    if (p.z > octreeNodes[nodeIdx].z_center) octant |= 4;

    insertParticleIntoOctree(octreeNodes[nodeIdx].children[octant], particleIdx);
}

// Center of mass calculation
void calculateNodeCOM(AlgorithmDemoState& state, int nodeIdx, double& comX, double& comY, double& comZ, double& totalMass) {
    if (nodeIdx < 0 || nodeIdx >= (int)state.octreeNodes.size()) {
        comX = comY = comZ = totalMass = 0;
        return;
    }

    const VisualOctreeNode& node = state.octreeNodes[nodeIdx];

    if (node.isLeaf) {
        if (node.particleIndex >= 0 && node.particleIndex < (int)state.allParticles.size()) {
            const Particle& p = state.allParticles[node.particleIndex];
            comX = p.x;
            comY = p.y;
            comZ = p.z;
            totalMass = p.mass;
        } else {
            comX = comY = comZ = totalMass = 0;
        }
        return;
    }

    comX = comY = comZ = totalMass = 0;
    for (int i = 0; i < 8; i++) {
        if (node.children[i] >= 0) {
            double cx, cy, cz, cm;
            calculateNodeCOM(state, node.children[i], cx, cy, cz, cm);
            if (cm > 0) {
                comX += cx * cm;
                comY += cy * cm;
                comZ += cz * cm;
                totalMass += cm;
            }
        }
    }
    if (totalMass > 0) {
        comX /= totalMass;
        comY /= totalMass;
        comZ /= totalMass;
    }
}

// Barnes-Hut traversal visualization
void buildTraversalRecursive(AlgorithmDemoState& state, int nodeIdx, double px, double py, double pz, int sourceParticleIdx) {
    if (nodeIdx < 0 || nodeIdx >= (int)state.octreeNodes.size()) return;

    const VisualOctreeNode& node = state.octreeNodes[nodeIdx];

    // Calculate COM for this node
    double comX, comY, comZ, totalMass;
    calculateNodeCOM(state, nodeIdx, comX, comY, comZ, totalMass);

    if (totalMass <= 0) return; // Empty node

    // Distance from source particle to COM
    double dx = comX - px;
    double dy = comY - py;
    double dz = comZ - pz;
    double dist = sqrt(dx*dx + dy*dy + dz*dz);

    if (dist < 0.001) return; // Skip self

    TraversalStep step;
    step.nodeIndex = nodeIdx;
    step.s = node.size;
    step.d = dist;
    step.ratio = node.size / dist;
    step.isLeaf = node.isLeaf;
    step.particleIndex = node.particleIndex;
    step.comX = comX;
    step.comY = comY;
    step.comZ = comZ;
    step.totalMass = totalMass;

    // Check if this is a leaf with the source particle - skip it
    if (node.isLeaf && node.particleIndex == sourceParticleIdx) {
        return;
    }

    if (node.isLeaf) {
        // Leaf node with a particle: must compute exactly
        step.passed = true; // "passed" means we use this node
        state.traversalSteps.push_back(step);
    } else if (step.ratio < state.theta) {
        // Passes theta test: use approximation
        step.passed = true;
        state.traversalSteps.push_back(step);
    } else {
        // Fails theta test: must go deeper
        step.passed = false;
        state.traversalSteps.push_back(step);

        // Recurse into children
        for (int i = 0; i < 8; i++) {
            if (node.children[i] >= 0) {
                buildTraversalRecursive(state, node.children[i], px, py, pz, sourceParticleIdx);
            }
        }
    }
}

void AlgorithmDemoState::buildTraversalSteps() {
    traversalSteps.clear();
    currentTraversalStep = 0;
    traversalComplete = false;

    if (allParticles.empty()) return;

    // Build octree with all particles first
    activeParticles = allParticles;
    buildVisualOctree();

    if (octreeNodes.empty()) return;

    // Use particle 0 as our source particle
    const Particle& source = allParticles[0];

    // Start traversal from root (node 0)
    buildTraversalRecursive(*this, 0, source.x, source.y, source.z, 0);
}

void initAlgorithmDemo(AlgorithmDemoState& state) {
    state.camera.position = (Vector3){ 40.0f, 30.0f, 40.0f };
    state.camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    state.camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    state.camera.fovy = 45.0f;
    state.camera.projection = CAMERA_PERSPECTIVE;
    state.reset();
}

// Demo update loop
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
    if (IsKeyPressed(KEY_FOUR)) {
        state.mode = DEMO_BARNES_HUT_CALC;
        state.buildTraversalSteps();  // Build the step-by-step traversal
    }

    if (IsKeyPressed(KEY_SPACE)) {
        if (state.mode == DEMO_OCTREE_BUILD) {
            state.addNextParticle();
        } else if (state.mode == DEMO_BRUTE_FORCE) {
            state.currentStep = (state.currentStep + 1) % state.allParticles.size();
        } else if (state.mode == DEMO_BARNES_HUT_CALC) {
            // Step through traversal
            if (!state.traversalComplete && state.currentTraversalStep < (int)state.traversalSteps.size() - 1) {
                state.currentTraversalStep++;
            } else {
                state.traversalComplete = true;
            }
        }
    }

    if (IsKeyPressed(KEY_EQUAL)) {
        state.theta += 0.1;
        if (state.theta > 2.0) state.theta = 2.0;
        if (state.mode == DEMO_BARNES_HUT_CALC) state.buildTraversalSteps();
    }
    if (IsKeyPressed(KEY_MINUS)) {
        state.theta -= 0.1;
        if (state.theta < 0.1) state.theta = 0.1;
        if (state.mode == DEMO_BARNES_HUT_CALC) state.buildTraversalSteps();
    }

    if (IsKeyPressed(KEY_R)) {
        state.reset();
        if (state.mode == DEMO_BARNES_HUT_CALC) state.buildTraversalSteps();
    }

    if (IsKeyPressed(KEY_L)) {
        state.toggleLightMode();
    }
}

// 3D line drawing utility
void DrawLine3DThick(Vector3 start, Vector3 end, float thickness, Color color) {
    Vector3 dir = {end.x - start.x, end.y - start.y, end.z - start.z};
    float length = sqrtf(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);
    if (length < 0.001f) return;

    Vector3 mid = {(start.x + end.x) / 2, (start.y + end.y) / 2, (start.z + end.z) / 2};
    DrawCylinderEx(start, end, thickness, thickness, 6, color);
}

// Demo rendering
void renderAlgorithmDemo(const AlgorithmDemoState& state, int screenWidth, int screenHeight, bool uiVisible) {
    BeginDrawing();
    ClearBackground(state.lightMode ? WHITE : BLACK);

    BeginMode3D(state.camera);

    Color axisAlpha = state.lightMode ? ColorAlpha(DARKGRAY, 0.5f) : ColorAlpha(WHITE, 0.3f);
    DrawLine3DThick((Vector3){-15, 0, 0}, (Vector3){15, 0, 0}, 0.15f, ColorAlpha(RED, 0.5f));
    DrawLine3DThick((Vector3){0, -15, 0}, (Vector3){0, 15, 0}, 0.15f, ColorAlpha(GREEN, 0.5f));
    DrawLine3DThick((Vector3){0, 0, -15}, (Vector3){0, 0, 15}, 0.15f, ColorAlpha(BLUE, 0.5f));

    if (state.mode == DEMO_BRUTE_FORCE) {
        for (size_t i = 0; i < state.allParticles.size(); i++) {
            Vector3 pos = {(float)state.allParticles[i].x, (float)state.allParticles[i].y, (float)state.allParticles[i].z};
            Color pColor = (i == (size_t)state.currentStep) ? RED : YELLOW;
            float pSize = (i == (size_t)state.currentStep) ? 1.5f : 1.2f;
            DrawSphere(pos, pSize, pColor);

            if (i == (size_t)state.currentStep) {
                DrawSphere(pos, 3.0f, ColorAlpha(RED, 0.2f));
            }
        }

        for (size_t i = 0; i < state.allParticles.size(); i++) {
            for (size_t j = i + 1; j < state.allParticles.size(); j++) {
                Vector3 p1 = {(float)state.allParticles[i].x, (float)state.allParticles[i].y, (float)state.allParticles[i].z};
                Vector3 p2 = {(float)state.allParticles[j].x, (float)state.allParticles[j].y, (float)state.allParticles[j].z};

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

        for (size_t i = 0; i < state.activeParticles.size(); i++) {
            Vector3 pos = {(float)state.activeParticles[i].x, (float)state.activeParticles[i].y, (float)state.activeParticles[i].z};
            DrawSphere(pos, 1.5f, YELLOW);

            DrawSphere(pos, 2.5f, ColorAlpha(YELLOW, 0.2f));
        }

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
            DrawSphere(cmPos, 4.0f, ColorAlpha(RED, 0.3f));
        }

        if (state.currentParticleIndex < (int)state.allParticles.size()) {
            const Particle& nextP = state.allParticles[state.currentParticleIndex];
            Vector3 pos = {(float)nextP.x, (float)nextP.y, (float)nextP.z};

            float pulse = (sin(GetTime() * 5.0f) + 1.0f) / 2.0f;
            float radius = 1.5f + pulse * 1.0f;

            DrawSphere(pos, radius, Fade(GREEN, 0.7f));
            DrawSphere(pos, radius + 2.0f, Fade(GREEN, 0.2f));
        }
    }

    // === MODE: OCTREE STRUCTURE ===
    else if (state.mode == DEMO_OCTREE_STRUCTURE) {
        for (size_t i = 0; i < state.allParticles.size(); i++) {
            Vector3 pos = {(float)state.allParticles[i].x, (float)state.allParticles[i].y, (float)state.allParticles[i].z};
            DrawSphere(pos, 1.2f, YELLOW);
        }

        drawOctreeNodeBox(0, 0, 0, 80.0, ColorAlpha(WHITE, 0.3f));

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

    // === MODE: BARNES-HUT CALCULATION (Step-by-step traversal) ===
    else if (state.mode == DEMO_BARNES_HUT_CALC) {
        // Colors adjusted for light/dark mode
        Color passColor = state.lightMode ? DARKGREEN : GREEN;
        Color failColor = state.lightMode ? Color{200, 100, 0, 255} : ORANGE;  // Darker orange
        Color leafColor = state.lightMode ? DARKBLUE : BLUE;
        Color dimNodeColor = state.lightMode ? ColorAlpha(DARKGRAY, 0.2f) : ColorAlpha(WHITE, 0.1f);
        Color dimParticleColor = state.lightMode ? ColorAlpha(ORANGE, 0.6f) : ColorAlpha(YELLOW, 0.4f);

        // Source particle (particle 0) - the one we're calculating forces FOR
        Vector3 sourcePos = {
            (float)state.allParticles[0].x,
            (float)state.allParticles[0].y,
            (float)state.allParticles[0].z
        };

        // Draw all particles (dimmed except source)
        for (size_t i = 0; i < state.allParticles.size(); i++) {
            Vector3 pos = {(float)state.allParticles[i].x, (float)state.allParticles[i].y, (float)state.allParticles[i].z};
            if (i == 0) {
                // Source particle - bright red
                DrawSphere(pos, 2.0f, RED);
                DrawSphere(pos, 4.0f, ColorAlpha(RED, 0.2f));
            } else {
                // Other particles - dimmed
                DrawSphere(pos, 1.2f, dimParticleColor);
            }
        }

        // Draw all octree nodes (dimmed)
        for (const auto& node : state.octreeNodes) {
            drawOctreeNodeBox(node.x_center, node.y_center, node.z_center,
                            node.size, dimNodeColor);
        }

        // Count approximations vs exact calculations
        int approxCount = 0;
        int exactCount = 0;

        // Draw completed steps (up to currentTraversalStep)
        for (int i = 0; i <= state.currentTraversalStep && i < (int)state.traversalSteps.size(); i++) {
            const TraversalStep& step = state.traversalSteps[i];
            const VisualOctreeNode& node = state.octreeNodes[step.nodeIndex];

            bool isCurrent = (i == state.currentTraversalStep);

            Vector3 comPos = {(float)step.comX, (float)step.comY, (float)step.comZ};

            if (step.passed) {
                // This node is USED (either leaf or passes theta)
                if (step.isLeaf) {
                    exactCount++;
                } else {
                    approxCount++;
                }

                // Green box - we use this node
                Color boxColor = isCurrent ? ColorAlpha(passColor, 0.8f) : ColorAlpha(passColor, 0.4f);
                drawOctreeNodeBox(node.x_center, node.y_center, node.z_center, node.size, boxColor);

                // Draw line from source to COM
                Color lineColor = isCurrent ? passColor : ColorAlpha(passColor, 0.6f);
                DrawLine3DThick(sourcePos, comPos, 0.5f, lineColor);

                // Draw COM point
                float comSize = isCurrent ? 2.5f : 1.5f;
                DrawSphere(comPos, comSize, step.isLeaf ? leafColor : passColor);

                if (isCurrent && !step.isLeaf) {
                    // Show it's an approximation with a glow
                    DrawSphere(comPos, 4.0f, ColorAlpha(passColor, 0.2f));
                }
            } else {
                // This node FAILED theta test - we go deeper (shown in red/orange)
                Color boxColor = isCurrent ? ColorAlpha(failColor, 0.7f) : ColorAlpha(RED, 0.3f);
                drawOctreeNodeBox(node.x_center, node.y_center, node.z_center, node.size, boxColor);

                // Draw COM point to show what we're checking
                if (isCurrent) {
                    DrawSphere(comPos, 2.0f, failColor);
                    // Draw thick line to show we're checking this
                    DrawLine3DThick(sourcePos, comPos, 0.5f, ColorAlpha(failColor, 0.8f));
                }
            }
        }

        // Draw current step info in 3D (near the node)
        if (state.currentTraversalStep < (int)state.traversalSteps.size()) {
            const TraversalStep& current = state.traversalSteps[state.currentTraversalStep];
            const VisualOctreeNode& node = state.octreeNodes[current.nodeIndex];

            // Highlight current node being evaluated with pulsing effect
            float pulse = (sin(GetTime() * 4.0f) + 1.0f) / 2.0f;
            Color highlightColor = current.passed ?
                ColorAlpha(passColor, 0.3f + pulse * 0.3f) :
                ColorAlpha(failColor, 0.3f + pulse * 0.3f);

            drawOctreeNodeBox(node.x_center, node.y_center, node.z_center,
                            node.size * 1.02, highlightColor);
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

        // Colors based on light/dark mode
        Color panelBg = state.lightMode ? ColorAlpha(WHITE, 0.9f) : ColorAlpha(BLACK, 0.85f);
        Color titleColor = state.lightMode ? BLACK : WHITE;
        Color accentColor = state.lightMode ? DARKBLUE : YELLOW;
        Color textColor = state.lightMode ? BLACK : LIGHTGRAY;

        // Top info panel - larger and more detailed
        DrawRectangle(0, 0, screenWidth, 280, panelBg);
        DrawText("ALGORITHM VISUAL WALKTHROUGH", 20, 10, 26, titleColor);
        DrawText(modeNames[state.mode], 20, 45, 20, accentColor);
        DrawText(TextFormat("Particles: %d  [%s MODE]", (int)state.allParticles.size(), state.lightMode ? "LIGHT" : "DARK"), 20, 75, 16, textColor);

        // Mode-specific detailed explanations
        if (state.mode == DEMO_BRUTE_FORCE) {
            int totalCalcs = (state.allParticles.size() * (state.allParticles.size() - 1)) / 2;
            DrawText(TextFormat("Total Calculations: %d (every particle pair!)", totalCalcs), 20, 105, 18, ORANGE);
            DrawText(TextFormat("Current: Computing forces for particle %d/%d", state.currentStep + 1, (int)state.allParticles.size()), 20, 130, 16, SKYBLUE);

            DrawText("HOW IT WORKS:", 20, 160, 16, titleColor);
            DrawText("• Each particle must calculate force from EVERY other particle", 20, 180, 14, textColor);
            DrawText("• Complexity: O(n²) - doubles particles = 4x computation!", 20, 200, 14, textColor);
            DrawText("• Blue lines show forces being calculated for current particle", 20, 220, 14, SKYBLUE);
            DrawText("• Gray lines show all other particle pairs", 20, 240, 14, textColor);
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
            DrawText("• Far particles grouped together → treated as single mass point", 20, 235, 14, textColor);
            DrawText("• This hierarchy reduces computation from O(n²) to O(n log n)", 20, 255, 14, textColor);
        }
        else if (state.mode == DEMO_BARNES_HUT_CALC) {
            DrawText(TextFormat("Step %d/%d | Theta: %.2f (+/-)",
                     state.currentTraversalStep + 1, (int)state.traversalSteps.size(), state.theta),
                     20, 105, 18, ORANGE);
            DrawText("Press SPACE to step through traversal", 20, 135, 14, GREEN);

            if (state.traversalComplete) {
                DrawText("TRAVERSAL COMPLETE!", 20, 160, 18, GREEN);
            }
        }

        // Controls overlay (bottom)
        DrawRectangle(0, screenHeight - 150, screenWidth, 150, panelBg);
        DrawText("CONTROLS:", 20, screenHeight - 140, 18, titleColor);
        DrawText("1: Brute Force | 2: Build Tree | 3: Tree Structure | 4: Barnes-Hut", 20, screenHeight - 115, 14, textColor);
        DrawText("SPACE: Next Step | R: Reset | +/-: Adjust Theta | L: Light Mode", 20, screenHeight - 95, 14, textColor);
        DrawText("Left Click: Rotate | Middle Click: Pan | Wheel: Zoom", 20, screenHeight - 75, 14, textColor);
        DrawText("H: Hide UI | Q: Return to Menu | ESC: Exit Program", 20, screenHeight - 55, 14, textColor);

        DrawText(TextFormat("FPS: %d", GetFPS()), screenWidth - 100, 20, 18, GREEN);
    }

    // === BARNES-HUT CALCULATION INFO (Always visible, lower right) ===
    if (state.mode == DEMO_BARNES_HUT_CALC && state.currentTraversalStep < (int)state.traversalSteps.size()) {
        const TraversalStep& step = state.traversalSteps[state.currentTraversalStep];

        // Background panel - lower right
        int panelWidth = 420;
        int panelHeight = 200;
        int panelX = screenWidth - panelWidth - 20;
        int panelY = screenHeight - panelHeight - 20;

        Color panelBg = state.lightMode ? ColorAlpha(WHITE, 0.95f) : ColorAlpha(BLACK, 0.9f);
        Color textColor = state.lightMode ? BLACK : WHITE;

        DrawRectangle(panelX, panelY, panelWidth, panelHeight, panelBg);
        DrawRectangleLines(panelX, panelY, panelWidth, panelHeight, textColor);

        // Title
        DrawText("THETA CRITERION", panelX + 15, panelY + 15, 24, textColor);

        // Values - large and readable
        Color valueColor = state.lightMode ? BLACK : LIGHTGRAY;
        DrawText(TextFormat("s = %.1f", step.s), panelX + 15, panelY + 55, 28, valueColor);
        DrawText(TextFormat("d = %.1f", step.d), panelX + 15, panelY + 90, 28, valueColor);
        DrawText(TextFormat("s/d = %.2f", step.ratio), panelX + 15, panelY + 125, 32, YELLOW);

        // Result
        if (step.isLeaf) {
            DrawText("LEAF -> Exact", panelX + 220, panelY + 125, 28, BLUE);
        } else if (step.passed) {
            DrawText(TextFormat("< %.1f PASS", state.theta), panelX + 220, panelY + 125, 28, GREEN);
            DrawText("-> Approximate!", panelX + 15, panelY + 165, 24, GREEN);
        } else {
            DrawText(TextFormat(">= %.1f FAIL", state.theta), panelX + 220, panelY + 125, 28, ORANGE);
            DrawText("-> Go deeper!", panelX + 15, panelY + 165, 24, ORANGE);
        }
    }

    EndDrawing();
}

// Octree wireframe drawing
void drawOctreeNodeBox(double x, double y, double z, double size, Color color) {
    double half = size / 2.0;
    float thickness = 0.3f;  // Thicker lines for better visibility

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
    DrawCylinderEx(corners[0], corners[1], thickness, thickness, 6, color);
    DrawCylinderEx(corners[1], corners[2], thickness, thickness, 6, color);
    DrawCylinderEx(corners[2], corners[3], thickness, thickness, 6, color);
    DrawCylinderEx(corners[3], corners[0], thickness, thickness, 6, color);

    // Top face
    DrawCylinderEx(corners[4], corners[5], thickness, thickness, 6, color);
    DrawCylinderEx(corners[5], corners[6], thickness, thickness, 6, color);
    DrawCylinderEx(corners[6], corners[7], thickness, thickness, 6, color);
    DrawCylinderEx(corners[7], corners[4], thickness, thickness, 6, color);

    // Vertical edges
    DrawCylinderEx(corners[0], corners[4], thickness, thickness, 6, color);
    DrawCylinderEx(corners[1], corners[5], thickness, thickness, 6, color);
    DrawCylinderEx(corners[2], corners[6], thickness, thickness, 6, color);
    DrawCylinderEx(corners[3], corners[7], thickness, thickness, 6, color);
}
