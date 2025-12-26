#ifndef RENDERER_H
#define RENDERER_H

#include <vector>
#include <deque>
#include <cstddef>  // For size_t on Windows/MSVC
#include "particle.h"
#include "raylib.h"

// Trail system
struct ParticleTrail {
    std::deque<Vector3> positions;
    int maxLength;

    ParticleTrail(int maxLen = 20) : maxLength(maxLen) {}  // Reduced from 50 to 20 for performance

    void addPosition(float x, float y, float z) {
        positions.push_back({x, y, z});
        if (positions.size() > (size_t)maxLength) {
            positions.pop_front();
        }
    }

    void clear() {
        positions.clear();
    }
};

// Camera control
void updateCamera(Camera3D& camera, float& cameraAngle, float& cameraDistance);

// Particle shooting
void shootProjectile(std::vector<Particle>& particles, const Camera3D& camera,
                     double speed, double mass);

// Rendering functions
void renderParticles(const std::vector<Particle>& particles, int numStaticParticles,
                    const std::vector<ParticleTrail>& trails, const Camera3D& camera, bool trailsEnabled = true);
void renderUI(int particleCount, float cameraDistance, int screenHeight,
              const char* methodName, double theta, int simSpeed, const char* scenarioName,
              int forceCalcs, int rebuildInterval);

// Trail management
void updateTrails(const std::vector<Particle>& particles, std::vector<ParticleTrail>& trails,
                 int numStaticParticles);

// Particle creation
void createStaticParticles(std::vector<Particle>& particles, int gridSize,
                          double spacing, double mass);

#endif // RENDERER_H
