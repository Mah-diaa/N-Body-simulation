#ifndef OCTREE_DEMO_H
#define OCTREE_DEMO_H

#include "../particle.h"
#include "raylib.h"
#include <vector>

// Demo visualization modes
enum AlgorithmDemoMode {
    DEMO_BRUTE_FORCE,
    DEMO_OCTREE_BUILD,
    DEMO_OCTREE_STRUCTURE,
    DEMO_BARNES_HUT_CALC
};

// Simple octree node for visualization
struct VisualOctreeNode {
    double x_center, y_center, z_center;
    double size;
    int particleIndex;  // -1 if internal node
    bool isLeaf;
    int children[8];    // Indices to child nodes, -1 if no child
    int depth;          // Depth in tree for coloring

    VisualOctreeNode() : x_center(0), y_center(0), z_center(0), size(0),
                         particleIndex(-1), isLeaf(true), depth(0) {
        for (int i = 0; i < 8; i++) children[i] = -1;
    }
};

// Demo state structure
struct AlgorithmDemoState {
    AlgorithmDemoMode mode;
    std::vector<Particle> allParticles;
    std::vector<Particle> activeParticles;
    std::vector<VisualOctreeNode> octreeNodes;
    int currentParticleIndex;
    int currentStep;
    int currentParticle;
    double theta;
    Camera3D camera;
    float cameraAngleH;
    float cameraAngleV;
    float cameraDistance;

    AlgorithmDemoState();
    void reset();
    void addNextParticle();
    void buildVisualOctree();
    void insertParticleIntoOctree(int nodeIdx, int particleIdx);
};

// Demo control functions
void initAlgorithmDemo(AlgorithmDemoState& state);
void updateAlgorithmDemo(AlgorithmDemoState& state);
void renderAlgorithmDemo(const AlgorithmDemoState& state, int screenWidth, int screenHeight, bool uiVisible = true);

// Helper visualization functions
void drawOctreeNodeBox(double x, double y, double z, double size, Color color);

#endif // OCTREE_DEMO_H
