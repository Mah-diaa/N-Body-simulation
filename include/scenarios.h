#ifndef SCENARIOS_H
#define SCENARIOS_H

#include "particle.h"
#include <vector>
#include <string>

// Scenario types
enum ScenarioType {
    GRID,
    GALAXY_DISK,
    KEPLERIAN_DISK,
    BLACK_HOLE,
    GALAXY_COLLISION
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

// Individual scenario generators
void createGrid(std::vector<Particle>& particles, int gridSize);
void createGalaxyDisk(std::vector<Particle>& particles, int numParticles);
void createKeplerianDisk(std::vector<Particle>& particles, int numParticles);
void createBlackHole(std::vector<Particle>& particles, int numParticles);
void createGalaxyCollision(std::vector<Particle>& particles, int particlesPerGalaxy);

#endif // SCENARIOS_H
