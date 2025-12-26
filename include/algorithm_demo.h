#ifndef ALGORITHM_DEMO_H
#define ALGORITHM_DEMO_H

#include "particle.h"
#include "raylib.h"
#include <vector>

// Demo visualization modes
enum AlgorithmDemoMode {
    DEMO_BRUTE_FORCE,      // Show all pairwise interactions
    DEMO_OCTREE_BUILD,     // Show octree construction step-by-step
    DEMO_OCTREE_STRUCTURE, // Show complete octree structure
    DEMO_BARNES_HUT_CALC   // Show Barnes-Hut calculation step-by-step
};

// Demo state structure
struct AlgorithmDemoState {
    AlgorithmDemoMode mode;
    std::vector<Particle> particles;
    int currentStep;
    int currentParticle;
    double theta;
    bool isPaused;
    Camera3D camera;
    float cameraAngleH;      // Horizontal rotation
    float cameraAngleV;      // Vertical rotation
    float cameraDistance;

    AlgorithmDemoState();
    void reset();
};

// Demo control functions
void initAlgorithmDemo(AlgorithmDemoState& state);
void updateAlgorithmDemo(AlgorithmDemoState& state);
void renderAlgorithmDemo(const AlgorithmDemoState& state, int screenWidth, int screenHeight, bool uiVisible = true);

// Helper visualization functions
void drawOctreeNodeBox(double x, double y, double z, double size, Color color);
void drawForceInteraction(Vector3 p1, Vector3 p2, Color color, float thickness = 1.0f);
void drawParticleWithLabel(const Particle& p, int index, Color color, float size);

#endif // ALGORITHM_DEMO_H
