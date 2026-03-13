#include <iostream>
#include <vector>
#include "particle.h"
#include "simulation.h"
#include "algorithms/octree3d.h"
#include "renderer.h"
#include "scenarios.h"
#include "menu.h"
#include "demos/octree_demo.h"
#include "demos/quadtree_demo.h"
#include "raylib.h"

enum SimulationMethod {
    BRUTE_FORCE,
    BARNES_HUT
};

enum AppState {
    MENU,
    SIMULATION,
    ALGORITHM_DEMO,
    QUADTREE_DEMO
};

int main() {
    std::cout << "==============================================\n";
    std::cout << "  N-Body Simulation - Barnes-Hut Algorithm\n";
    std::cout << "==============================================\n\n";

    // Window initialization
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    int monitorWidth = GetMonitorWidth(0);
    int monitorHeight = GetMonitorHeight(0);

    int screenWidth = (int)(monitorWidth * 0.85f);
    int screenHeight = (int)(monitorHeight * 0.85f);

    if (screenWidth < 1200) screenWidth = 1200;
    if (screenHeight < 800) screenHeight = 800;

    InitWindow(screenWidth, screenHeight, "N-Body Simulation - Menu");
    SetTargetFPS(60);

    screenWidth = GetScreenWidth();
    screenHeight = GetScreenHeight();

    // App state
    AppState currentState = MENU;
    std::vector<Scenario> scenarios = getScenarios();
    int selectedScenarioIndex = 2;
    int particleCount = scenarios[selectedScenarioIndex].recommendedParticles;
    bool trailsEnabled = true;
    bool uiVisible = true;
    bool startSimulation = false;
    bool startDemo = false;
    bool startQuadTreeDemo = false;

    // Simulation parameters
    std::vector<Particle> particles;
    std::vector<ParticleTrail> trails;
    double G = 1.0;
    double dt = 0.01;
    SimulationMethod method = BARNES_HUT;
    double theta = 1.2;
    double softening = 0.1;
    int simStepsPerFrame = 1;
    int numStaticParticles = 0;
    double projectileSpeed = 50.0;
    ScenarioType currentScenarioType;
    int currentParticleCount = 0;

    AlgorithmDemoState demoState;

    QuadTreeDemo quadTreeDemo;

    // Camera setup
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 50.0f, 50.0f, 50.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    float cameraAngleH = 0.0f;
    float cameraAngleV = 0.3f;
    float cameraDistance = 70.0f;

    while (!WindowShouldClose()) {
        screenWidth = GetScreenWidth();
        screenHeight = GetScreenHeight();

        // ==== MENU STATE ====
        if (currentState == MENU) {
            if (startSimulation) {
                currentState = SIMULATION;
                startSimulation = false;

                Scenario selectedScenario = scenarios[selectedScenarioIndex];
                createScenario(particles, selectedScenario.type, particleCount);

                G = selectedScenario.recommendedG;
                dt = selectedScenario.recommendedDt;
                numStaticParticles = particles.size();
                currentScenarioType = selectedScenario.type;
                currentParticleCount = particles.size();

                camera.position = (Vector3){ 50.0f, 50.0f, 50.0f };
                camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
                cameraAngleH = 0.0f;
                cameraAngleV = 0.3f;
                cameraDistance = 70.0f;

                trails.clear();
            }

            if (startDemo) {
                currentState = ALGORITHM_DEMO;
                startDemo = false;
                initAlgorithmDemo(demoState);
            }

            if (startQuadTreeDemo) {
                currentState = QUADTREE_DEMO;
                startQuadTreeDemo = false;
                quadTreeDemo.reset();
            }

            BeginDrawing();
            renderMenu(screenWidth, screenHeight, scenarios,
                      selectedScenarioIndex, particleCount,
                      trailsEnabled, softening, startSimulation, startDemo, startQuadTreeDemo);
            EndDrawing();
        }

        // ==== SIMULATION STATE ====
        else if (currentState == SIMULATION) {
            // Physics update
            for (int step = 0; step < simStepsPerFrame; step++) {
                if (method == BRUTE_FORCE) {
                    calculateForces(particles, G, softening);
                } else if (method == BARNES_HUT) {
                    calculateForcesBarnesHut(particles, G, theta, softening);
                }
                updateParticles(particles, dt);
            }

            if (trailsEnabled) {
                updateTrails(particles, trails, numStaticParticles);
            }

        // Camera and input handling
        updateCamera(camera, cameraAngleH, cameraAngleV, cameraDistance);

        if (IsKeyPressed(KEY_SPACE)) {
            shootProjectile(particles, camera, projectileSpeed, 5000.0);
        }

        if (IsKeyPressed(KEY_Q)) {
            currentState = MENU;
            particles.clear();
            trails.clear();
            continue;
        }

        if (IsKeyPressed(KEY_M)) {
            method = (method == BRUTE_FORCE) ? BARNES_HUT : BRUTE_FORCE;
        }

        if (IsKeyPressed(KEY_T)) {
            trailsEnabled = !trailsEnabled;
            if (!trailsEnabled) {
                trails.clear();
            }
        }

        if (IsKeyPressed(KEY_H)) {
            uiVisible = !uiVisible;
        }

        if (IsKeyPressed(KEY_EQUAL) || IsKeyPressed(KEY_KP_ADD)) {
            theta += 0.1;
            if (theta > 2.0) theta = 2.0;
        }

        if (IsKeyPressed(KEY_MINUS) || IsKeyPressed(KEY_KP_SUBTRACT)) {
            theta -= 0.1;
            if (theta < 0.1) theta = 0.1;
        }

        if (IsKeyPressed(KEY_UP)) {
            simStepsPerFrame++;
            if (simStepsPerFrame > 10) simStepsPerFrame = 10;
        }

        if (IsKeyPressed(KEY_DOWN)) {
            simStepsPerFrame--;
            if (simStepsPerFrame < 1) simStepsPerFrame = 1;
        }

        if (IsKeyPressed(KEY_LEFT_BRACKET)) {
            int current = getTreeRebuildInterval();
            current = std::max(1, current - 10);
            setTreeRebuildInterval(current);
        }
        if (IsKeyPressed(KEY_RIGHT_BRACKET)) {
            int current = getTreeRebuildInterval();
            current = std::min(200, current + 10);
            setTreeRebuildInterval(current);
        }

        if (IsKeyPressed(KEY_R)) {
            particles.clear();
            createScenario(particles, currentScenarioType, currentParticleCount);
            numStaticParticles = particles.size();
            camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
            trails.clear();
        }

            BeginDrawing();
            ClearBackground(BLACK);

            BeginMode3D(camera);
            renderParticles(particles, numStaticParticles, trails, camera, trailsEnabled);
            EndMode3D();

            int forceCalcs = 0, traversals = 0;
            if (method == BARNES_HUT) {
                getBarnesHutStats(forceCalcs, traversals);
            }

            const char* methodName;
            if (method == BRUTE_FORCE) {
                methodName = "CPU Brute Force O(n^2)";
            } else {
                methodName = "Barnes-Hut O(n log n)";
            }

            if (uiVisible) {
                renderUI(particles.size(), cameraDistance, screenHeight, methodName, theta, simStepsPerFrame,
                         scenarios[selectedScenarioIndex].name.c_str(), forceCalcs, getTreeRebuildInterval(), softening);
            }

            EndDrawing();
        }  // End SIMULATION state

        // ==== ALGORITHM DEMO STATE ====
        else if (currentState == ALGORITHM_DEMO) {
            updateAlgorithmDemo(demoState);

            if (IsKeyPressed(KEY_Q)) {
                currentState = MENU;
                continue;
            }

            if (IsKeyPressed(KEY_H)) {
                uiVisible = !uiVisible;
            }

            renderAlgorithmDemo(demoState, screenWidth, screenHeight, uiVisible);
        }  // End ALGORITHM_DEMO state

        // ==== QUADTREE DEMO STATE ====
        else if (currentState == QUADTREE_DEMO) {
            quadTreeDemo.update();

            if (IsKeyPressed(KEY_SPACE)) {
                quadTreeDemo.addNextParticle();
            }

            if (IsKeyPressed(KEY_R)) {
                quadTreeDemo.reset();
            }

            if (IsKeyPressed(KEY_L)) {
                quadTreeDemo.toggleLightMode();
            }

            if (IsKeyPressed(KEY_H)) {
                quadTreeDemo.toggleLegend();
            }

            if (IsKeyPressed(KEY_Q)) {
                currentState = MENU;
                continue;
            }

            quadTreeDemo.render(screenWidth, screenHeight);
        }  // End QUADTREE_DEMO state
    }  // End main loop

    CloseWindow();

    return 0;
}
