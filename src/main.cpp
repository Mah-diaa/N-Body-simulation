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

    // Configure window flags before initialization
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    // Get monitor size to fit window nicely
    int monitorWidth = GetMonitorWidth(0);
    int monitorHeight = GetMonitorHeight(0);

    // Use 85% of monitor size for a good fit (not too big, not too small)
    int screenWidth = (int)(monitorWidth * 0.85f);
    int screenHeight = (int)(monitorHeight * 0.85f);

    // Ensure minimum size for readability
    if (screenWidth < 1200) screenWidth = 1200;
    if (screenHeight < 800) screenHeight = 800;

    // Initialize raylib window
    InitWindow(screenWidth, screenHeight, "N-Body Simulation - Menu");
    SetTargetFPS(60);

    // Update to actual screen dimensions after window creation
    screenWidth = GetScreenWidth();
    screenHeight = GetScreenHeight();

    // App state management
    AppState currentState = MENU;
    std::vector<Scenario> scenarios = getScenarios();
    int selectedScenarioIndex = 2;  // Default to Galaxy Disk
    int particleCount = scenarios[selectedScenarioIndex].recommendedParticles;
    bool trailsEnabled = true;
    bool uiVisible = true;  // Toggle for UI overlay visibility
    bool startSimulation = false;
    bool startDemo = false;  // For algorithm demo
    bool startQuadTreeDemo = false;  // For quadtree demo

    // Simulation variables (initialized when simulation starts)
    std::vector<Particle> particles;
    std::vector<Particle> initialParticles;
    std::vector<ParticleTrail> trails;
    double G = 1.0;
    double dt = 0.01;
    SimulationMethod method = BARNES_HUT;
    double theta = 1.2;  // Barnes-Hut accuracy parameter (1.0-1.2 is optimal balance)
    int simStepsPerFrame = 1;
    int numStaticParticles = 0;
    double projectileSpeed = 5.0;
    ScenarioType currentScenarioType;
    int currentParticleCount = 0;

    // Algorithm demo state
    AlgorithmDemoState demoState;

    // QuadTree demo state
    QuadTreeDemo quadTreeDemo;

    // 3D camera
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 50.0f, 50.0f, 50.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Camera control variables
    float cameraAngle = 0.0f;
    float cameraDistance = 70.0f;

    // Main game loop
    while (!WindowShouldClose()) {
        // Update screen dimensions in case window was resized
        screenWidth = GetScreenWidth();
        screenHeight = GetScreenHeight();

        // ==== MENU STATE ====
        if (currentState == MENU) {
            // Handle menu to simulation transition
            if (startSimulation) {
                currentState = SIMULATION;
                startSimulation = false;

                // Get selected scenario
                Scenario selectedScenario = scenarios[selectedScenarioIndex];
                createScenario(particles, selectedScenario.type, particleCount);

                // Initialize simulation parameters
                G = selectedScenario.recommendedG;
                dt = selectedScenario.recommendedDt;
                initialParticles = particles;
                numStaticParticles = particles.size();
                currentScenarioType = selectedScenario.type;
                currentParticleCount = particles.size();

                // Reset camera
                camera.position = (Vector3){ 50.0f, 50.0f, 50.0f };
                camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
                cameraAngle = 0.0f;
                cameraDistance = 70.0f;

                // Clear trails
                trails.clear();

                std::cout << "\n=== Simulation Started ===\n";
                std::cout << "Scenario: " << selectedScenario.name << std::endl;
                std::cout << "Particles: " << particles.size() << std::endl;
                std::cout << "Controls: Q = Return to Menu, R = Reset, M = Toggle Method, T = Toggle Trails, H = Hide UI\n";
            }

            // Handle demo launch
            if (startDemo) {
                currentState = ALGORITHM_DEMO;
                startDemo = false;
                initAlgorithmDemo(demoState);
            }

            // Handle quadtree demo launch
            if (startQuadTreeDemo) {
                currentState = QUADTREE_DEMO;
                startQuadTreeDemo = false;
                quadTreeDemo.reset();
            }

            // Render menu
            BeginDrawing();
            renderMenu(screenWidth, screenHeight, scenarios,
                      selectedScenarioIndex, particleCount,
                      trailsEnabled, startSimulation, startDemo, startQuadTreeDemo);
            EndDrawing();
        }

        // ==== SIMULATION STATE ====
        else if (currentState == SIMULATION) {
            // Physics update - run multiple steps per frame for faster simulation
            for (int step = 0; step < simStepsPerFrame; step++) {
                // Use selected method
                if (method == BRUTE_FORCE) {
                    calculateForces(particles, G);
                } else if (method == BARNES_HUT) {
                    calculateForcesBarnesHut(particles, G, theta);
                }
                updateParticles(particles, dt);
            }

            // Update trails (only once per frame, if enabled)
            if (trailsEnabled) {
                updateTrails(particles, trails, numStaticParticles);
            }

        // Camera update
        updateCamera(camera, cameraAngle, cameraDistance);

        // Input handling
        if (IsKeyPressed(KEY_SPACE)) {
            shootProjectile(particles, camera, projectileSpeed, 5000.0);
        }

        // Return to menu
        if (IsKeyPressed(KEY_Q)) {
            currentState = MENU;
            particles.clear();
            trails.clear();
            std::cout << "\n=== Returned to Menu ===\n";
            continue;  // Skip rest of simulation loop
        }

        // Method switching (toggle between BRUTE_FORCE and BARNES_HUT)
        if (IsKeyPressed(KEY_M)) {
            method = (method == BRUTE_FORCE) ? BARNES_HUT : BRUTE_FORCE;
            std::cout << "Switched to " << (method == BRUTE_FORCE ? "CPU Brute Force" : "Barnes-Hut") << std::endl;
        }

        // Trails toggle
        if (IsKeyPressed(KEY_T)) {
            trailsEnabled = !trailsEnabled;
            if (!trailsEnabled) {
                trails.clear();  // Clear trails when disabling
            }
            std::cout << "Trails: " << (trailsEnabled ? "ON" : "OFF") << std::endl;
        }

        // UI visibility toggle
        if (IsKeyPressed(KEY_H)) {
            uiVisible = !uiVisible;
            std::cout << "UI Overlay: " << (uiVisible ? "VISIBLE" : "HIDDEN") << std::endl;
        }

        // Theta adjustment
        if (IsKeyPressed(KEY_EQUAL) || IsKeyPressed(KEY_KP_ADD)) {
            theta += 0.1;
            if (theta > 2.0) theta = 2.0;
            std::cout << "Theta: " << theta << std::endl;
        }

        if (IsKeyPressed(KEY_MINUS) || IsKeyPressed(KEY_KP_SUBTRACT)) {
            theta -= 0.1;
            if (theta < 0.1) theta = 0.1;
            std::cout << "Theta: " << theta << std::endl;
        }

        // Simulation speed adjustment
        if (IsKeyPressed(KEY_UP)) {
            simStepsPerFrame++;
            if (simStepsPerFrame > 10) simStepsPerFrame = 10;
            std::cout << "Simulation speed: " << simStepsPerFrame << "x" << std::endl;
        }

        if (IsKeyPressed(KEY_DOWN)) {
            simStepsPerFrame--;
            if (simStepsPerFrame < 1) simStepsPerFrame = 1;
            std::cout << "Simulation speed: " << simStepsPerFrame << "x" << std::endl;
        }

        // Tree rebuild interval adjustment (for Barnes-Hut performance tuning)
        if (IsKeyPressed(KEY_LEFT_BRACKET)) {
            int current = getTreeRebuildInterval();
            current = std::max(1, current - 10);
            setTreeRebuildInterval(current);
            std::cout << "Tree rebuild interval: " << current << " steps (more accurate, slower)" << std::endl;
        }
        if (IsKeyPressed(KEY_RIGHT_BRACKET)) {
            int current = getTreeRebuildInterval();
            current = std::min(200, current + 10);
            setTreeRebuildInterval(current);
            std::cout << "Tree rebuild interval: " << current << " steps (less accurate, faster)" << std::endl;
        }

        // Reset simulation (recreate scenario)
        if (IsKeyPressed(KEY_R)) {
            particles.clear();
            createScenario(particles, currentScenarioType, currentParticleCount);
            initialParticles = particles;
            numStaticParticles = particles.size();
            camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
            trails.clear();
            std::cout << "Scenario reset!" << std::endl;
        }

            // Rendering
            BeginDrawing();
            ClearBackground(BLACK);

            BeginMode3D(camera);
            // Grid removed for cleaner look
            renderParticles(particles, numStaticParticles, trails, camera, trailsEnabled);
            EndMode3D();

            // Get Barnes-Hut stats for display
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

            // Only render UI if visible
            if (uiVisible) {
                renderUI(particles.size(), cameraDistance, screenHeight, methodName, theta, simStepsPerFrame,
                         scenarios[selectedScenarioIndex].name.c_str(), forceCalcs, getTreeRebuildInterval());
            }

            EndDrawing();
        }  // End SIMULATION state

        // ==== ALGORITHM DEMO STATE ====
        else if (currentState == ALGORITHM_DEMO) {
            // Update demo
            updateAlgorithmDemo(demoState);

            // Return to menu
            if (IsKeyPressed(KEY_Q)) {
                currentState = MENU;
                std::cout << "\n=== Returned to Menu ===\n";
                continue;
            }

            // UI visibility toggle
            if (IsKeyPressed(KEY_H)) {
                uiVisible = !uiVisible;
                std::cout << "UI Overlay: " << (uiVisible ? "VISIBLE" : "HIDDEN") << std::endl;
            }

            // Render demo
            renderAlgorithmDemo(demoState, screenWidth, screenHeight, uiVisible);
        }  // End ALGORITHM_DEMO state

        // ==== QUADTREE DEMO STATE ====
        else if (currentState == QUADTREE_DEMO) {
            // Update demo
            quadTreeDemo.update();

            // Handle input
            if (IsKeyPressed(KEY_SPACE)) {
                quadTreeDemo.addNextParticle();
            }

            if (IsKeyPressed(KEY_R)) {
                quadTreeDemo.reset();
                std::cout << "QuadTree demo reset\n";
            }

            // Return to menu
            if (IsKeyPressed(KEY_Q)) {
                currentState = MENU;
                std::cout << "\n=== Returned to Menu ===\n";
                continue;
            }

            // Render demo
            quadTreeDemo.render(screenWidth, screenHeight);
        }  // End QUADTREE_DEMO state
    }  // End main loop

    CloseWindow();

    std::cout << "Simulation complete!" << std::endl;
    return 0;
}
