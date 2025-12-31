#include "renderer.h"
#include <cmath>
#include <iostream>
#include <cstddef>  // For size_t on Windows/MSVC

void updateCamera(Camera3D& camera, float& cameraAngleH, float& cameraAngleV, float& cameraDistance) {
    // EXACT COPY from octree demo - rotation
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        Vector2 mouseDelta = GetMouseDelta();
        cameraAngleH -= mouseDelta.x * 0.01f;
        cameraAngleV += mouseDelta.y * 0.01f;

        if (cameraAngleV < -M_PI/2.0f + 0.1f) cameraAngleV = -M_PI/2.0f + 0.1f;
        if (cameraAngleV > M_PI/2.0f - 0.1f) cameraAngleV = M_PI/2.0f - 0.1f;
    }

    // EXACT COPY from octree demo - zoom
    cameraDistance -= GetMouseWheelMove() * 5.0f;
    if (cameraDistance < 30.0f) cameraDistance = 30.0f;
    if (cameraDistance > 800.0f) cameraDistance = 800.0f;

    // EXACT COPY from octree demo - camera position calculation
    camera.position.x = camera.target.x + cameraDistance * cos(cameraAngleV) * cos(cameraAngleH);
    camera.position.z = camera.target.z + cameraDistance * cos(cameraAngleV) * sin(cameraAngleH);
    camera.position.y = camera.target.y + cameraDistance * sin(cameraAngleV);
}

void shootProjectile(std::vector<Particle>& particles, const Camera3D& camera,
                     double speed, double mass) {
    Particle projectile;

    // Start at camera position
    projectile.x = camera.position.x;
    projectile.y = camera.position.y;
    projectile.z = camera.position.z;
    projectile.mass = mass;

    // Calculate direction from camera to target
    Vector3 direction = {
        camera.target.x - camera.position.x,
        camera.target.y - camera.position.y,
        camera.target.z - camera.position.z
    };

    // Normalize direction
    float length = sqrt(direction.x * direction.x +
                      direction.y * direction.y +
                      direction.z * direction.z);
    direction.x /= length;
    direction.y /= length;
    direction.z /= length;

    // Set velocity in that direction
    projectile.vx = direction.x * speed;
    projectile.vy = direction.y * speed;
    projectile.vz = direction.z * speed;

    particles.push_back(projectile);
    std::cout << "Projectile launched! Total particles: " << particles.size() << std::endl;
}

void updateTrails(const std::vector<Particle>& particles, std::vector<ParticleTrail>& trails,
                 int numStaticParticles) {
    // Only track trails for static particles (not projectiles)
    for (int i = 0; i < numStaticParticles && i < (int)particles.size(); i++) {
        if (i >= (int)trails.size()) {
            trails.push_back(ParticleTrail(50));
        }
        trails[i].addPosition(particles[i].x, particles[i].y, particles[i].z);
    }
}

// Helper function to get color based on position
Color getParticleColor(Vector3 pos, const Particle& particle, size_t particleIndex, int numStaticParticles) {
    if (particleIndex >= (size_t)numStaticParticles) {
        // Projectiles - super bright pink/magenta
        return (Color){255, 100, 255, 255};
    } else if (particle.mass >= 5.0) {
        // Heavy particles (text, planets) - use custom color
        return particle.color;
    } else {
        // Light particles - colorful distance-based gradient
        double distance = std::sqrt(pos.x*pos.x + pos.y*pos.y + pos.z*pos.z);
        float distNorm = std::min(1.0f, (float)distance / 60.0f);

        if (distNorm < 0.15f) {
            float t = distNorm / 0.15f;
            return (Color){255, 255, (unsigned char)(255 - t * 155), 255};
        } else if (distNorm < 0.4f) {
            float t = (distNorm - 0.15f) / 0.25f;
            return (Color){255, (unsigned char)(255 - t * 90), (unsigned char)(100 - t * 100), 255};
        } else if (distNorm < 0.7f) {
            float t = (distNorm - 0.4f) / 0.3f;
            return (Color){(unsigned char)(255 - t * 105), (unsigned char)(165 + t * 90), (unsigned char)(0 + t * 255), 255};
        } else {
            float t = (distNorm - 0.7f) / 0.3f;
            return (Color){(unsigned char)(150 - t * 70), (unsigned char)(255 - t * 105), 255, 255};
        }
    }
}

void renderParticles(const std::vector<Particle>& particles, int numStaticParticles,
                    const std::vector<ParticleTrail>& trails, const Camera3D& camera, bool trailsEnabled) {
    // Draw trails when enabled - colorful trails that match particle colors!
    if (trailsEnabled) {
        for (size_t i = 0; i < (size_t)numStaticParticles && i < particles.size(); i++) {
            if (i < trails.size() && trails[i].positions.size() > 1) {
                // Draw trail as connected line segments with gradient
                for (size_t j = 1; j < trails[i].positions.size(); j++) {
                    // Calculate color based on trail position for rainbow effect
                    Color baseColor = getParticleColor(trails[i].positions[j], particles[i], i, numStaticParticles);

                    // Fade from transparent to full opacity along the trail
                    float alpha = (float)j / trails[i].positions.size();
                    Color trailColor = ColorAlpha(baseColor, alpha * 0.5f);

                    DrawLine3D(trails[i].positions[j-1], trails[i].positions[j], trailColor);
                }
            }
        }
    }

    // Draw particles using their custom colors and sizes
    for (size_t i = 0; i < particles.size(); i++) {
        Vector3 position = {
            (float)particles[i].x,
            (float)particles[i].y,
            (float)particles[i].z
        };

        // Get particle color using helper function
        Color particleColor = getParticleColor(position, particles[i], i, numStaticParticles);

        // Determine rendering style
        float radius;
        bool renderAsSphere = false;

        if (i >= (size_t)numStaticParticles) {
            // Projectiles
            radius = 1.2f;
            renderAsSphere = true;
        } else if (particles[i].mass >= 5.0) {
            // Heavy particles (text, planets)
            renderAsSphere = true;
            radius = particles[i].renderRadius;
        }

        // Draw based on particle type
        if (renderAsSphere) {
            // Spheres for heavy particles (planets, text, projectiles)
            DrawSphere(position, radius, particleColor);
        } else {
            // Points for light particles - fast rendering
            DrawPoint3D(position, particleColor);
        }
    }
}

void renderUI(int particleCount, float cameraDistance, int screenHeight,
              const char* methodName, double theta, int simSpeed, const char* scenarioName,
              int forceCalcs, int rebuildInterval, double softening) {
    DrawText("N-Body Simulation", 10, 10, 20, WHITE);
    DrawText(TextFormat("Scenario: %s", scenarioName), 10, 35, 16, SKYBLUE);
    DrawText(TextFormat("Particles: %d", particleCount), 10, 55, 16, LIGHTGRAY);
    DrawText(TextFormat("Method: %s", methodName), 10, 75, 16, YELLOW);
    DrawText(TextFormat("Theta: %.2f", theta), 10, 95, 16, LIGHTGRAY);
    DrawText(TextFormat("Softening: %.1f  (set in menu - prevents particle slingshots)", softening), 10, 115, 14, LIGHTGRAY);
    DrawText(TextFormat("Sim Speed: %dx", simSpeed), 10, 135, 16, LIGHTGRAY);
    DrawText(TextFormat("Tree Rebuild: every %d steps", rebuildInterval), 10, 155, 16, ORANGE);
    DrawText(TextFormat("FPS: %d", GetFPS()), 10, 175, 16, GREEN);

    // Show force calculations vs brute force comparison
    int bruteForceCalcs = particleCount * particleCount;
    if (forceCalcs > 0) {
        float savings = 100.0f * (1.0f - (float)forceCalcs / bruteForceCalcs);
        DrawText(TextFormat("Force Calcs: %d (%.1f%% saved)", forceCalcs, savings), 10, 195, 14, ORANGE);
        DrawText(TextFormat("vs Brute Force: %d", bruteForceCalcs), 10, 210, 12, GRAY);
    }

    DrawText("Controls:", 10, screenHeight - 200, 14, GRAY);
    DrawText("Mouse Drag: Rotate", 10, screenHeight - 180, 12, GRAY);
    DrawText("Mouse Wheel: Zoom", 10, screenHeight - 165, 12, GRAY);
    DrawText("M: Toggle method", 10, screenHeight - 150, 12, GRAY);
    DrawText("T: Toggle trails", 10, screenHeight - 135, 12, GRAY);
    DrawText("H: Hide UI overlay", 10, screenHeight - 120, 12, GRAY);
    DrawText("UP/DOWN: Sim speed", 10, screenHeight - 105, 12, GRAY);
    DrawText("+/-: Adjust theta", 10, screenHeight - 90, 12, GRAY);
    DrawText("[/]: Tree rebuild interval", 10, screenHeight - 75, 12, GRAY);
    DrawText("R: Reset", 10, screenHeight - 60, 12, GRAY);
    DrawText("ESC: Exit", 10, screenHeight - 45, 12, GRAY);
}

void createStaticParticles(std::vector<Particle>& particles, int gridSize,
                          double spacing, double mass) {
    for (int ix = 0; ix < gridSize; ix++) {
        for (int iy = 0; iy < gridSize; iy++) {
            for (int iz = 0; iz < gridSize; iz++) {
                Particle p;
                p.x = (ix - gridSize/2.0) * spacing;
                p.y = (iy - gridSize/2.0) * spacing;
                p.z = (iz - gridSize/2.0) * spacing;
                p.mass = mass;
                p.vx = p.vy = p.vz = 0.0;
                particles.push_back(p);
            }
        }
    }
}
