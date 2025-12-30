#include "algorithms/quadtree2d.h"

QuadTreeNode2D::QuadTreeNode2D()
    : x_center(0), y_center(0), size(0),
      mass(0), cm_x(0), cm_y(0),
      particleIndex(-1), isLeaf(true), depth(0), nodeId(-1) {
    for (int i = 0; i < 4; i++) children[i] = -1;
}

QuadTree2D::QuadTree2D(double worldSize)
    : worldSize(worldSize), worldCenterX(0.0), worldCenterY(0.0), nextNodeId(0) {
    createNode(worldCenterX, worldCenterY, worldSize, 0);
}

QuadTree2D::~QuadTree2D() {}

void QuadTree2D::clear() {
    nodes.clear();
    particles.clear();
    nextNodeId = 0;
    createNode(worldCenterX, worldCenterY, worldSize, 0);
}

void QuadTree2D::buildTree(const std::vector<Particle2D>& inputParticles) {
    nodes.clear();
    particles = inputParticles;
    nextNodeId = 0;

    if (particles.empty()) {
        createNode(worldCenterX, worldCenterY, worldSize, 0);
        return;
    }

    createNode(worldCenterX, worldCenterY, worldSize, 0);
    for (int i = 0; i < (int)particles.size(); i++) {
        insertParticle(0, i);
    }
    updateCenterOfMass(0);
}

int QuadTree2D::createNode(double cx, double cy, double size, int depth) {
    QuadTreeNode2D node;
    node.x_center = cx;
    node.y_center = cy;
    node.size = size;
    node.depth = depth;
    node.nodeId = nextNodeId++;
    node.isLeaf = true;
    node.particleIndex = -1;
    node.mass = 0;
    node.cm_x = cx;
    node.cm_y = cy;
    for (int i = 0; i < 4; i++) node.children[i] = -1;

    nodes.push_back(node);
    return nodes.size() - 1;
}

int QuadTree2D::getQuadrant(double px, double py, double cx, double cy) {
    // 0=NW, 1=NE, 2=SW, 3=SE
    if (px < cx) return (py >= cy) ? 0 : 2;
    else return (py >= cy) ? 1 : 3;
}

void QuadTree2D::insertParticle(int nodeIdx, int particleIdx) {
    QuadTreeNode2D& node = nodes[nodeIdx];

    if (node.isLeaf && node.particleIndex == -1) {
        node.particleIndex = particleIdx;
        return;
    }

    if (node.isLeaf && node.particleIndex != -1) {
        int existingParticle = node.particleIndex;
        node.particleIndex = -1;
        node.isLeaf = false;

        double childSize = node.size / 2.0;
        double offset = childSize / 2.0;

        node.children[0] = createNode(node.x_center - offset, node.y_center + offset, childSize, node.depth + 1);
        node.children[1] = createNode(node.x_center + offset, node.y_center + offset, childSize, node.depth + 1);
        node.children[2] = createNode(node.x_center - offset, node.y_center - offset, childSize, node.depth + 1);
        node.children[3] = createNode(node.x_center + offset, node.y_center - offset, childSize, node.depth + 1);

        int quad = getQuadrant(particles[existingParticle].x, particles[existingParticle].y, node.x_center, node.y_center);
        insertParticle(node.children[quad], existingParticle);
    }

    const Particle2D& p = particles[particleIdx];
    int quad = getQuadrant(p.x, p.y, node.x_center, node.y_center);
    insertParticle(node.children[quad], particleIdx);
}

void QuadTree2D::updateCenterOfMass(int nodeIdx) {
    if (nodeIdx < 0 || nodeIdx >= (int)nodes.size()) return;

    QuadTreeNode2D& node = nodes[nodeIdx];

    if (node.isLeaf && node.particleIndex != -1) {
        const Particle2D& p = particles[node.particleIndex];
        node.mass = p.mass;
        node.cm_x = p.x;
        node.cm_y = p.y;
        return;
    }

    double totalMass = 0, cmX = 0, cmY = 0;
    for (int i = 0; i < 4; i++) {
        if (node.children[i] != -1) {
            updateCenterOfMass(node.children[i]);
            QuadTreeNode2D& child = nodes[node.children[i]];
            totalMass += child.mass;
            cmX += child.cm_x * child.mass;
            cmY += child.cm_y * child.mass;
        }
    }

    if (totalMass > 0) {
        node.mass = totalMass;
        node.cm_x = cmX / totalMass;
        node.cm_y = cmY / totalMass;
    }
}
