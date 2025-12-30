#ifndef QUADTREE_DEMO_H
#define QUADTREE_DEMO_H

#include "raylib.h"
#include "algorithms/quadtree2d.h"
#include <vector>
#include <string>

// Extended Particle2D with color for visualization
struct Particle2DColored : public Particle2D {
    Color color;

    Particle2DColored() : Particle2D(), color(WHITE) {}
    Particle2DColored(double px, double py, double m, Color c)
        : Particle2D(px, py, m), color(c) {}
};

// Demo wrapper for QuadTree visualization
class QuadTreeDemo {
public:
    QuadTreeDemo();
    ~QuadTreeDemo();

    void reset();
    void addNextParticle();
    void update();
    void render(int screenWidth, int screenHeight);

    bool allParticlesAdded() const { return currentParticleIndex >= (int)allParticles.size(); }
    int getCurrentStep() const { return currentParticleIndex; }
    int getTotalSteps() const { return allParticles.size(); }

private:
    QuadTree2D quadTree;
    std::vector<Particle2DColored> allParticles;
    std::vector<Particle2DColored> activeParticles;
    int currentParticleIndex;

    void renderSpatialView(int x, int y, int width, int height);
    void renderTreeDiagram(int x, int y, int width, int height);
    void renderCalculationPanel(int x, int y, int width, int height);

    void drawParticles(int viewX, int viewY, int viewWidth, int viewHeight);
    void drawQuadTreeBounds(int nodeIdx, int viewX, int viewY, int viewWidth, int viewHeight);
    void drawTreeNode(int nodeIdx, float x, float y, float spacing, int viewX, int viewY, int viewWidth, int viewHeight);

    Vector2 worldToScreen(double wx, double wy, int viewX, int viewY, int viewWidth, int viewHeight);
    std::string getCalculationExplanation();
};

#endif
