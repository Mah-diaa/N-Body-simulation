#ifndef QUADTREE2D_H
#define QUADTREE2D_H

#include <vector>

// Pure 2D QuadTree implementation - no rendering dependencies
struct Particle2D {
    double x, y, mass;

    Particle2D() : x(0), y(0), mass(1.0) {}
    Particle2D(double px, double py, double m) : x(px), y(py), mass(m) {}
};

struct QuadTreeNode2D {
    double x_center, y_center, size;
    double mass, cm_x, cm_y;

    int children[4];  // 0=NW, 1=NE, 2=SW, 3=SE
    int particleIndex;
    bool isLeaf;
    int depth, nodeId;

    QuadTreeNode2D();
};

class QuadTree2D {
public:
    QuadTree2D(double worldSize = 100.0);
    ~QuadTree2D();

    void clear();
    void buildTree(const std::vector<Particle2D>& particles);

    // Getters
    const std::vector<QuadTreeNode2D>& getNodes() const { return nodes; }
    const QuadTreeNode2D& getRoot() const { return nodes[0]; }
    double getWorldSize() const { return worldSize; }
    double getWorldCenterX() const { return worldCenterX; }
    double getWorldCenterY() const { return worldCenterY; }

private:
    std::vector<QuadTreeNode2D> nodes;
    std::vector<Particle2D> particles;

    double worldSize;
    double worldCenterX, worldCenterY;
    int nextNodeId;

    int createNode(double cx, double cy, double size, int depth);
    int getQuadrant(double px, double py, double cx, double cy);
    void insertParticle(int nodeIdx, int particleIdx);
    void updateCenterOfMass(int nodeIdx);
};

#endif
