#ifndef QUADTREE_DEMO_H
#define QUADTREE_DEMO_H

#include "raylib.h"
#include <vector>
#include <string>

// 2D Particle structure for quadtree demo
struct Particle2D {
    double x, y;
    double mass;
    Color color;

    Particle2D() : x(0), y(0), mass(1.0), color(WHITE) {}
    Particle2D(double px, double py, double m, Color c = WHITE)
        : x(px), y(py), mass(m), color(c) {}
};

// Quadtree node structure
struct QuadTreeNode {
    // Bounding box
    double x_center, y_center;
    double size;

    // Center of mass data
    double mass;
    double cm_x, cm_y;

    // Tree structure
    int children[4];  // Indices: 0=NW, 1=NE, 2=SW, 3=SE (-1 if empty)
    int particleIndex;  // Index of particle if leaf (-1 if not)
    bool isLeaf;

    // For visualization
    int depth;
    int nodeId;  // Unique ID for this node

    QuadTreeNode() : x_center(0), y_center(0), size(0),
                     mass(0), cm_x(0), cm_y(0),
                     particleIndex(-1), isLeaf(true), depth(0), nodeId(-1) {
        for (int i = 0; i < 4; i++) children[i] = -1;
    }
};

// Quadtree demo state
class QuadTreeDemo {
public:
    QuadTreeDemo();
    ~QuadTreeDemo();

    // Control functions
    void reset();
    void addNextParticle();  // Add one particle and rebuild tree
    void update();
    void render(int screenWidth, int screenHeight);

    // Tree building
    void buildTree();
    void insertParticle(int nodeIdx, int particleIdx);
    void updateCenterOfMass(int nodeIdx);
    int createNode(double cx, double cy, double size, int depth);

    // Getters
    bool allParticlesAdded() const { return currentParticleIndex >= (int)allParticles.size(); }
    int getCurrentStep() const { return currentParticleIndex; }
    int getTotalSteps() const { return allParticles.size(); }

private:
    // Data
    std::vector<Particle2D> allParticles;      // All particles to add
    std::vector<Particle2D> activeParticles;   // Particles currently in tree
    std::vector<QuadTreeNode> nodes;           // Tree nodes (flat storage)

    int currentParticleIndex;  // How many particles have been added
    int nextNodeId;            // For assigning unique IDs to nodes

    // Tree parameters
    double worldSize;
    double worldCenterX;
    double worldCenterY;

    // Visualization helpers
    void renderSpatialView(int x, int y, int width, int height);
    void renderTreeDiagram(int x, int y, int width, int height);
    void renderCalculationPanel(int x, int y, int width, int height);
    void drawQuadTreeBounds(int nodeIdx, int viewX, int viewY, int viewWidth, int viewHeight);
    void drawTreeNode(int nodeIdx, float x, float y, float spacing, int viewX, int viewY, int viewWidth, int viewHeight);
    void drawParticles(int viewX, int viewY, int viewWidth, int viewHeight);
    std::string getCalculationExplanation();

    // Helper to convert world coords to screen coords
    Vector2 worldToScreen(double wx, double wy, int viewX, int viewY, int viewWidth, int viewHeight);

    // Helper to get quadrant (0=NW, 1=NE, 2=SW, 3=SE)
    int getQuadrant(double px, double py, double cx, double cy);

    // Calculate tree layout positions
    struct TreeNodeLayout {
        float x, y;
        int depth;
    };
    void calculateTreeLayout(std::vector<TreeNodeLayout>& layout, int nodeIdx, float x, float y, float hSpacing, int depth);
};

#endif // QUADTREE_DEMO_H
