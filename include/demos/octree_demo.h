#ifndef OCTREE_DEMO_H
#define OCTREE_DEMO_H

#include "../particle.h"
#include "raylib.h"
#include <vector>

enum AlgorithmDemoMode {
    DEMO_BRUTE_FORCE,
    DEMO_OCTREE_BUILD,
    DEMO_OCTREE_STRUCTURE,
    DEMO_BARNES_HUT_CALC
};

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

struct TraversalStep {
    int nodeIndex;          // Which node we're evaluating
    double s;               // Size of the node
    double d;               // Distance to node center/COM
    double ratio;           // s/d
    bool passed;            // Did it pass theta test?
    bool isLeaf;            // Is this a leaf node?
    int particleIndex;      // If leaf, which particle (-1 if none)
    double comX, comY, comZ; // Center of mass position
    double totalMass;       // Total mass in this node
};

struct AlgorithmDemoState {
    AlgorithmDemoMode mode;
    std::vector<Particle> allParticles;
    std::vector<Particle> activeParticles;
    std::vector<VisualOctreeNode> octreeNodes;
    int currentParticleIndex;
    int currentStep;
    double theta;
    Camera3D camera;
    float cameraAngleH;
    float cameraAngleV;
    float cameraDistance;
    bool lightMode;

    std::vector<TraversalStep> traversalSteps;
    int currentTraversalStep;
    bool traversalComplete;

    AlgorithmDemoState();
    void reset();
    void addNextParticle();
    void buildVisualOctree();
    void insertParticleIntoOctree(int nodeIdx, int particleIdx);
    void buildTraversalSteps();  // Build step-by-step traversal
    void toggleLightMode() { lightMode = !lightMode; }
    bool isLightMode() const { return lightMode; }
};

void initAlgorithmDemo(AlgorithmDemoState& state);
void updateAlgorithmDemo(AlgorithmDemoState& state);
void renderAlgorithmDemo(const AlgorithmDemoState& state, int screenWidth, int screenHeight, bool uiVisible = true);

void drawOctreeNodeBox(double x, double y, double z, double size, Color color);

#endif // OCTREE_DEMO_H
