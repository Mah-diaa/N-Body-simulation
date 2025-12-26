#ifndef SCENARIOS_H
#define SCENARIOS_H

#include "particle.h"
#include <vector>
#include <string>

// Scenario types
enum ScenarioType {
    GRID,              // Simple grid (for testing)
    PLUMMER_SPHERE,    // Globular cluster (Plummer model)
    GALAXY_DISK,       // Spiral galaxy disk
    KEPLERIAN_DISK,    // Particles orbiting central mass
    BINARY_ORBIT,      // Two stars orbiting each other
    SOLAR_SYSTEM,      // Sun with planets
    BLACK_HOLE,        // Black hole with accretion disk
    GALAXY_COLLISION,  // Two galaxies colliding
    FIGURE_8,          // Three-body figure-8 orbit
    TEXT               // Custom text from particles
};

// Scenario information
struct Scenario {
    ScenarioType type;
    std::string name;
    std::string description;
    int recommendedParticles;
    double recommendedG;
    double recommendedDt;
};

// Get list of all scenarios
std::vector<Scenario> getScenarios();

// Create particles for a specific scenario
void createScenario(std::vector<Particle>& particles, ScenarioType type, int numParticles);
void createScenarioWithText(std::vector<Particle>& particles, ScenarioType type, int numParticles, const std::string& text);

// Individual scenario generators
void createGrid(std::vector<Particle>& particles, int gridSize);
void createPlummerSphere(std::vector<Particle>& particles, int numParticles);
void createGalaxyDisk(std::vector<Particle>& particles, int numParticles);
void createKeplerianDisk(std::vector<Particle>& particles, int numParticles);
void createBinaryOrbit(std::vector<Particle>& particles);
void createSolarSystem(std::vector<Particle>& particles);
void createBlackHole(std::vector<Particle>& particles, int numParticles);
void createGalaxyCollision(std::vector<Particle>& particles, int particlesPerGalaxy);
void createFigure8(std::vector<Particle>& particles);
void createText(std::vector<Particle>& particles, const std::string& text);

#endif // SCENARIOS_H
