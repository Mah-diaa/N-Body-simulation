#ifndef OCTREE_OPTIMIZED_H
#define OCTREE_OPTIMIZED_H

#include "particle.h"
#include <vector>
#include <cstdint>

// Optimized octree node - stored in flat vector for cache locality
struct OptimizedOctreeNode {
    // Bounding box
    double x_center, y_center, z_center;
    double size;

    // Center of mass data
    double mass;
    double cm_x, cm_y, cm_z;

    // Tree structure - use indices instead of pointers
    // INCREMENT 2: Implicit child indexing - children stored sequentially
    int firstChild;      // Index of first child, children at [firstChild, firstChild+7], -1 if leaf
    int particleIndex;   // Index of particle if leaf, -1 otherwise

    // Leaf flag
    bool isLeaf;

    // Constructor
    OptimizedOctreeNode() : x_center(0), y_center(0), z_center(0), size(0),
                            mass(0), cm_x(0), cm_y(0), cm_z(0),
                            firstChild(-1), particleIndex(-1), isLeaf(true) {
    }
};

// Optimized octree class
class OptimizedOctree {
public:
    OptimizedOctree();
    ~OptimizedOctree();

    // Build tree from particles (optimization #4 - with Morton code sorting)
    void build(std::vector<Particle>& particles);

    // Calculate forces using Barnes-Hut
    void calculateForces(std::vector<Particle>& particles, double G, double theta);

    // Get profiling stats
    void getStats(int& forceCalcs, int& traversals) const {
        forceCalcs = m_forceCalculations;
        traversals = m_treeTraversals;
    }

private:
    std::vector<OptimizedOctreeNode> m_nodes;  // Flat vector storage
    std::vector<Particle>* m_particlesRef;     // Reference to particles for tree building
    std::vector<int> m_sortedIndices;          // INCREMENT 3: Particle indices sorted by Morton code

    // Profiling
    int m_forceCalculations;
    int m_treeTraversals;

    // INCREMENT 3: Morton code functions for spatial sorting
    uint64_t mortonCode(double x, double y, double z, double min, double size) const;
    uint32_t expandBits(uint32_t v) const;

    // Tree building helpers
    void insertParticle(int nodeIdx, Particle* p, int pIdx);
    void updateCenterOfMass(int nodeIdx);
    int buildNode(int start, int end, double cx, double cy, double cz, double size);

    // Force calculation
    void calculateForceOnParticle(Particle& p, size_t particleIdx, int nodeIdx,
                                  const std::vector<Particle>& particles,
                                  double G, double theta);
};

// ========== Global Interface Functions ==========
// Simple interface for Barnes-Hut simulation

// Calculate forces using Barnes-Hut algorithm
void calculateForcesBarnesHut(std::vector<Particle>& particles, double G, double theta);

// Profiling: Get Barnes-Hut statistics
void getBarnesHutStats(int& forceCalcs, int& traversals);

// Performance tuning: Set how often to rebuild the octree
void setTreeRebuildInterval(int interval);
int getTreeRebuildInterval();

#endif // OCTREE_OPTIMIZED_H
