#include "particle.h"
#include <vector>

void calculateForces(std::vector<Particle>& particles, double G, double softening = 0.1);

void updateParticles(std::vector<Particle>& particles, double dt);
