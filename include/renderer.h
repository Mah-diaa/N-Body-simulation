#ifndef RENDERER_H
#define RENDERER_H

#include <vector>
#include <deque>
#include <cstddef>
#include "particle.h"
#include "raylib.h"

struct ParticleTrail {
    std::deque<Vector3> positions;
    int maxLength;

    ParticleTrail(int maxLen = 20) : maxLength(maxLen) {}

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

void updateCamera(Camera3D& camera, float& cameraAngleH, float& cameraAngleV, float& cameraDistance);

void shootProjectile(std::vector<Particle>& particles, const Camera3D& camera,
                     double speed, double mass);

void renderParticles(const std::vector<Particle>& particles, int numStaticParticles,
                    const std::vector<ParticleTrail>& trails, const Camera3D& camera, bool trailsEnabled = true);
void renderUI(int particleCount, float cameraDistance, int screenHeight,
              const char* methodName, double theta, int simSpeed, const char* scenarioName,
              int forceCalcs, int rebuildInterval, double softening);

void updateTrails(const std::vector<Particle>& particles, std::vector<ParticleTrail>& trails,
                 int numStaticParticles);

#endif // RENDERER_H
