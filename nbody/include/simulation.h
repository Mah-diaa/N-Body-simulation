#include "particle.h"
#include <vector>

void calculateForces(std::vector<Particle>& particles, double G);

void updateParticles(std::vector<Particle>& particles, double dt);

void runSim(std::vector<Particle>& particles, int numsteps, double dt, double G);
