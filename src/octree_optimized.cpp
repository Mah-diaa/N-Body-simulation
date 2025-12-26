#include "octree_optimized.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <iostream>

OptimizedOctree::OptimizedOctree()
    : m_particlesRef(nullptr), m_forceCalculations(0), m_treeTraversals(0) {
}

OptimizedOctree::~OptimizedOctree() {
}

// Expand a 10-bit integer into 30 bits by inserting 2 zeros after each bit
uint32_t OptimizedOctree::expandBits(uint32_t v) const {
    v = (v * 0x00010001u) & 0xFF0000FFu;
    v = (v * 0x00000101u) & 0x0F00F00Fu;
    v = (v * 0x00000011u) & 0xC30C30C3u;
    v = (v * 0x00000005u) & 0x49249249u;
    return v;
}

// Calculate Morton code (Z-order) for a 3D point
uint64_t OptimizedOctree::mortonCode(double x, double y, double z, double min, double size) const {
    // Normalize to [0, 1]
    double nx = (x - min) / size;
    double ny = (y - min) / size;
    double nz = (z - min) / size;

    // Clamp to [0, 1]
    nx = std::max(0.0, std::min(1.0, nx));
    ny = std::max(0.0, std::min(1.0, ny));
    nz = std::max(0.0, std::min(1.0, nz));

    // Convert to integers in range [0, 1023]
    uint32_t ix = static_cast<uint32_t>(nx * 1023.0);
    uint32_t iy = static_cast<uint32_t>(ny * 1023.0);
    uint32_t iz = static_cast<uint32_t>(nz * 1023.0);

    // Interleave bits
    uint64_t code = 0;
    code |= expandBits(ix);
    code |= expandBits(iy) << 1;
    code |= expandBits(iz) << 2;

    return code;
}

// Build optimized octree (INCREMENT 2: flat vector + implicit child indexing)
void OptimizedOctree::build(std::vector<Particle>& particles) {
    if (particles.empty()) return;

    m_particlesRef = &particles;
    m_nodes.clear();
    m_nodes.reserve(particles.size() * 2);

    // Find bounding box
    double min_x = particles[0].x, max_x = particles[0].x;
    double min_y = particles[0].y, max_y = particles[0].y;
    double min_z = particles[0].z, max_z = particles[0].z;

    for (const auto& p : particles) {
        min_x = std::min(min_x, p.x);
        max_x = std::max(max_x, p.x);
        min_y = std::min(min_y, p.y);
        max_y = std::max(max_y, p.y);
        min_z = std::min(min_z, p.z);
        max_z = std::max(max_z, p.z);
    }

    double center_x = (min_x + max_x) * 0.5;
    double center_y = (min_y + max_y) * 0.5;
    double center_z = (min_z + max_z) * 0.5;
    double size = std::max({max_x - min_x, max_y - min_y, max_z - min_z}) * 1.1;

    // Create root node
    int rootIdx = m_nodes.size();
    m_nodes.emplace_back();
    m_nodes[rootIdx].x_center = center_x;
    m_nodes[rootIdx].y_center = center_y;
    m_nodes[rootIdx].z_center = center_z;
    m_nodes[rootIdx].size = size;

    // Insert all particles into tree (using insertion method - slower but reliable)
    for (size_t i = 0; i < particles.size(); i++) {
        insertParticle(rootIdx, &particles[i], i);
    }
}

// INCREMENT 3: Build a node recursively from sorted particle range
int OptimizedOctree::buildNode(int start, int end, double cx, double cy, double cz, double size) {
    // Create node
    int nodeIdx = m_nodes.size();
    m_nodes.emplace_back();

    // Set bounding box (access by index to avoid invalidation)
    m_nodes[nodeIdx].x_center = cx;
    m_nodes[nodeIdx].y_center = cy;
    m_nodes[nodeIdx].z_center = cz;
    m_nodes[nodeIdx].size = size;

    int count = end - start;

    if (count == 0) {
        return nodeIdx;
    }

    if (count == 1) {
        // Leaf node with single particle
        int pIdx = m_sortedIndices[start];
        // Validate index bounds
        if (pIdx < 0 || pIdx >= (int)m_particlesRef->size()) {
            return nodeIdx; // Invalid index, return empty node
        }
        m_nodes[nodeIdx].isLeaf = true;
        m_nodes[nodeIdx].particleIndex = pIdx;
        m_nodes[nodeIdx].mass = (*m_particlesRef)[pIdx].mass;
        m_nodes[nodeIdx].cm_x = (*m_particlesRef)[pIdx].x;
        m_nodes[nodeIdx].cm_y = (*m_particlesRef)[pIdx].y;
        m_nodes[nodeIdx].cm_z = (*m_particlesRef)[pIdx].z;
        return nodeIdx;
    }

    // Internal node - subdivide
    m_nodes[nodeIdx].isLeaf = false;

    // Children will be stored sequentially (implicit indexing)
    m_nodes[nodeIdx].firstChild = m_nodes.size();

    // Partition particles into 8 octants
    int octantCounts[8] = {0};
    int octantStarts[8];

    // Count particles in each octant
    for (int i = start; i < end; i++) {
        int pIdx = m_sortedIndices[i];
        int octant = 0;
        if ((*m_particlesRef)[pIdx].x >= cx) octant |= 1;
        if ((*m_particlesRef)[pIdx].y >= cy) octant |= 2;
        if ((*m_particlesRef)[pIdx].z >= cz) octant |= 4;
        octantCounts[octant]++;
    }

    // Calculate starting positions
    octantStarts[0] = start;
    for (int i = 1; i < 8; i++) {
        octantStarts[i] = octantStarts[i-1] + octantCounts[i-1];
    }

    // Partition particles (in-place using temporary array)
    std::vector<int> tempIndices(m_sortedIndices.begin() + start,
                                 m_sortedIndices.begin() + end);
    int octantPos[8];
    for (int i = 0; i < 8; i++) octantPos[i] = 0;

    for (int i = 0; i < count; i++) {
        int pIdx = tempIndices[i];
        // Bounds check
        if (pIdx < 0 || pIdx >= (int)m_particlesRef->size()) continue;

        int octant = 0;
        if ((*m_particlesRef)[pIdx].x >= cx) octant |= 1;
        if ((*m_particlesRef)[pIdx].y >= cy) octant |= 2;
        if ((*m_particlesRef)[pIdx].z >= cz) octant |= 4;

        m_sortedIndices[octantStarts[octant] + octantPos[octant]] = pIdx;
        octantPos[octant]++;
    }

    // Create 8 children sequentially
    double quarter = size * 0.25;
    double half = size * 0.5;

    for (int i = 0; i < 8; i++) {
        double child_cx = cx + ((i & 1) ? quarter : -quarter);
        double child_cy = cy + ((i & 2) ? quarter : -quarter);
        double child_cz = cz + ((i & 4) ? quarter : -quarter);

        int childStart = octantStarts[i];
        int childEnd = (i < 7) ? octantStarts[i+1] : end;

        buildNode(childStart, childEnd, child_cx, child_cy, child_cz, half);
    }

    // Update center of mass
    updateCenterOfMass(nodeIdx);

    return nodeIdx;
}

// Insert a particle into the tree (OLD METHOD - kept for reference but not used with Morton sorting)
void OptimizedOctree::insertParticle(int nodeIdx, Particle* p, int pIdx) {
    // Access by index to avoid invalidation issues
    if (m_nodes[nodeIdx].isLeaf) {
        if (m_nodes[nodeIdx].particleIndex == -1) {
            // Empty leaf - just place the particle here
            m_nodes[nodeIdx].particleIndex = pIdx;
            m_nodes[nodeIdx].mass = p->mass;
            m_nodes[nodeIdx].cm_x = p->x;
            m_nodes[nodeIdx].cm_y = p->y;
            m_nodes[nodeIdx].cm_z = p->z;
        } else {
            // Leaf already has a particle - need to subdivide
            int existingPIdx = m_nodes[nodeIdx].particleIndex;
            m_nodes[nodeIdx].particleIndex = -1;
            m_nodes[nodeIdx].isLeaf = false;

            // INCREMENT 2: Create 8 children sequentially for implicit indexing
            double quarter = m_nodes[nodeIdx].size / 4.0;
            double half = m_nodes[nodeIdx].size / 2.0;

            // Store first child index before adding children
            int firstChildIdx = m_nodes.size();
            m_nodes[nodeIdx].firstChild = firstChildIdx;

            // Add all 8 children sequentially
            for (int i = 0; i < 8; i++) {
                double x_offset = (i & 1) ? quarter : -quarter;
                double y_offset = (i & 2) ? quarter : -quarter;
                double z_offset = (i & 4) ? quarter : -quarter;

                m_nodes.emplace_back();
                int childIdx = firstChildIdx + i;

                m_nodes[childIdx].x_center = m_nodes[nodeIdx].x_center + x_offset;
                m_nodes[childIdx].y_center = m_nodes[nodeIdx].y_center + y_offset;
                m_nodes[childIdx].z_center = m_nodes[nodeIdx].z_center + z_offset;
                m_nodes[childIdx].size = half;
            }

            // Re-insert existing particle and new particle
            Particle* existingP = &((*m_particlesRef)[existingPIdx]);
            insertParticle(nodeIdx, existingP, existingPIdx);
            insertParticle(nodeIdx, p, pIdx);

            // Update center of mass
            updateCenterOfMass(nodeIdx);
        }
    } else {
        // Internal node - find appropriate octant and recurse
        int octant = 0;
        if (p->x >= m_nodes[nodeIdx].x_center) octant |= 1;
        if (p->y >= m_nodes[nodeIdx].y_center) octant |= 2;
        if (p->z >= m_nodes[nodeIdx].z_center) octant |= 4;

        // INCREMENT 2: Use implicit indexing
        insertParticle(m_nodes[nodeIdx].firstChild + octant, p, pIdx);

        // Update center of mass after insertion
        updateCenterOfMass(nodeIdx);
    }
}

// Update center of mass for a node
void OptimizedOctree::updateCenterOfMass(int nodeIdx) {
    if (m_nodes[nodeIdx].isLeaf) {
        int pIdx = m_nodes[nodeIdx].particleIndex;
        if (pIdx >= 0) {
            m_nodes[nodeIdx].mass = (*m_particlesRef)[pIdx].mass;
            m_nodes[nodeIdx].cm_x = (*m_particlesRef)[pIdx].x;
            m_nodes[nodeIdx].cm_y = (*m_particlesRef)[pIdx].y;
            m_nodes[nodeIdx].cm_z = (*m_particlesRef)[pIdx].z;
        }
    } else {
        double total_mass = 0.0;
        double weighted_x = 0.0;
        double weighted_y = 0.0;
        double weighted_z = 0.0;

        // INCREMENT 2: Sum contributions from 8 children using implicit indexing
        int firstChildIdx = m_nodes[nodeIdx].firstChild;
        for (int i = 0; i < 8; i++) {
            int childIdx = firstChildIdx + i;
            if (childIdx < (int)m_nodes.size() && m_nodes[childIdx].mass > 0.0) {
                double child_mass = m_nodes[childIdx].mass;
                total_mass += child_mass;
                weighted_x += child_mass * m_nodes[childIdx].cm_x;
                weighted_y += child_mass * m_nodes[childIdx].cm_y;
                weighted_z += child_mass * m_nodes[childIdx].cm_z;
            }
        }

        m_nodes[nodeIdx].mass = total_mass;
        if (total_mass > 0.0) {
            m_nodes[nodeIdx].cm_x = weighted_x / total_mass;
            m_nodes[nodeIdx].cm_y = weighted_y / total_mass;
            m_nodes[nodeIdx].cm_z = weighted_z / total_mass;
        }
    }
}

// Calculate forces using optimized Barnes-Hut
void OptimizedOctree::calculateForces(std::vector<Particle>& particles, double G, double theta) {
    m_forceCalculations = 0;
    m_treeTraversals = 0;

    // Reset accelerations
    for (auto& p : particles) {
        p.ax = 0.0;
        p.ay = 0.0;
        p.az = 0.0;
    }

    if (m_nodes.empty()) return;

    // Calculate forces for each particle (pass index explicitly to avoid pointer arithmetic issues)
    for (size_t i = 0; i < particles.size(); i++) {
        calculateForceOnParticle(particles[i], i, 0, particles, G, theta);
    }
}

// Calculate force on a single particle using optimized traversal
void OptimizedOctree::calculateForceOnParticle(Particle& p, size_t particleIdx, int nodeIdx,
                                               const std::vector<Particle>& particles,
                                               double G, double theta) {
    m_treeTraversals++;

    if (nodeIdx >= (int)m_nodes.size()) return;

    const OptimizedOctreeNode& node = m_nodes[nodeIdx];

    if (node.mass == 0.0) return;

    // Calculate distance to center of mass
    double dx = node.cm_x - p.x;
    double dy = node.cm_y - p.y;
    double dz = node.cm_z - p.z;
    double distSqr = dx * dx + dy * dy + dz * dz;

    // Check if this is the particle itself (compare indices directly)
    if (node.isLeaf && node.particleIndex >= 0) {
        if (particleIdx == (size_t)node.particleIndex) {
            return;  // Don't calculate force on itself
        }

        // Skip text-text interactions (both masses 5-100 = text particles)
        const Particle& other = particles[node.particleIndex];
        if (p.mass >= 5.0 && p.mass < 100.0 &&
            other.mass >= 5.0 && other.mass < 100.0) {
            return;  // Text particles don't attract each other
        }
    }

    // Barnes-Hut criterion: size/distance < theta
    if (node.isLeaf || (node.size / std::sqrt(distSqr) < theta)) {
        // Use this node as a single body
        m_forceCalculations++;

        double dist = std::sqrt(distSqr + 1.0); // Softening
        double invDist3 = 1.0 / (dist * dist * dist);
        double force = G * node.mass * invDist3;

        p.ax += force * dx;
        p.ay += force * dy;
        p.az += force * dz;
    } else {
        // INCREMENT 2: Traverse children using implicit indexing (cache-friendly!)
        int firstChildIdx = node.firstChild;
        for (int i = 0; i < 8; i++) {
            int childIdx = firstChildIdx + i;
            if (childIdx < (int)m_nodes.size()) {
                calculateForceOnParticle(p, particleIdx, childIdx, particles, G, theta);
            }
        }
    }
}

// ========== Global Interface Functions ==========
// These provide a simple interface for the simulation code

// Global optimized octree instance
static OptimizedOctree g_optimizedTree;

// Tree rebuild optimization - rebuild every N steps instead of every step
static int g_stepsSinceRebuild = 0;
static int g_rebuildInterval = 1;  // Rebuild every step (standard practice, adjustable with [/] keys)

// Profiling statistics
static int g_forceCalculations = 0;
static int g_treeTraversals = 0;

// Calculate forces for all particles using Barnes-Hut (OPTIMIZED)
void calculateForcesBarnesHut(std::vector<Particle>& particles, double G, double theta) {
    // PERFORMANCE OPTIMIZATION: Only rebuild tree every N steps
    // This trades some accuracy for much better performance
    if (g_stepsSinceRebuild == 0) {
        g_optimizedTree.build(particles);
    }
    g_stepsSinceRebuild++;
    if (g_stepsSinceRebuild >= g_rebuildInterval) {
        g_stepsSinceRebuild = 0;
    }

    g_optimizedTree.calculateForces(particles, G, theta);

    // Update profiling stats
    g_optimizedTree.getStats(g_forceCalculations, g_treeTraversals);
}

// Set tree rebuild interval (how many steps between rebuilds)
void setTreeRebuildInterval(int interval) {
    g_rebuildInterval = interval;
    g_stepsSinceRebuild = 0;  // Reset counter when interval changes
}

// Get tree rebuild interval
int getTreeRebuildInterval() {
    return g_rebuildInterval;
}

// Get Barnes-Hut profiling statistics
void getBarnesHutStats(int& forceCalcs, int& traversals) {
    forceCalcs = g_forceCalculations;
    traversals = g_treeTraversals;
}
