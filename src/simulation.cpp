#include "simulation.h"
#include "particle.h"
#include <cmath>
#include <cstddef>  // For size_t on Windows/MSVC


void calculateForces(std::vector<Particle> &particles, double G, double softening){
	    size_t n = particles.size();
    for (size_t i = 0; i < n; ++i) {
	double fx= 0.0;
	double fy = 0.0;
	double fz = 0.0;
	for (size_t j = 0; j < n; ++j) {
	    if (i != j) {
		// Skip text-text interactions (both masses 5-100 = text particles)
		if (particles[i].mass >= 5.0 && particles[i].mass < 100.0 &&
		    particles[j].mass >= 5.0 && particles[j].mass < 100.0) {
		    continue;  // Text particles don't attract each other
		}

		double dx = particles[j].x - particles[i].x;
		double dy = particles[j].y - particles[i].y;
		double dz = particles[j].z - particles[i].z;
		double distSqr = dx * dx + dy * dy + dz * dz + softening; // Softening factor to avoid singularity
		double invDist = 1.0 / sqrt(distSqr);
		double invDist3 = invDist * invDist * invDist;
		double force = G * particles[i].mass * particles [j].mass * invDist3;
		fx += force * dx;
		fy += force * dy;
		fz += force * dz;
	}
	    }
	particles[i].ax = fx / particles[i].mass;
	particles[i].ay = fy / particles[i].mass;
	particles[i].az = fz / particles[i].mass;
}
}

void updateParticles(std::vector<Particle> &particles, double dt){
    for (auto &p : particles) {
	p.vx += p.ax * dt;
	p.vy += p.ay * dt;
	p.vz += p.az * dt;
	p.x += p.vx * dt;
	p.y += p.vy * dt;
	p.z += p.vz * dt;
    }
}

void runSim(std::vector<Particle> &particles, int numsteps, double dt, double G){
    for (int step = 0; step < numsteps; ++step) {
	calculateForces(particles, G);
	updateParticles(particles, dt);
    }
}

