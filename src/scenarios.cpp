#include "scenarios.h"
#include <cmath>
#include <random>

static std::random_device rd;
static std::mt19937 gen(rd());

double randomUniform(double min, double max) {
    std::uniform_real_distribution<> dis(min, max);
    return dis(gen);
}

double randomNormal(double mean, double stddev) {
    std::normal_distribution<> dis(mean, stddev);
    return dis(gen);
}

std::vector<Scenario> getScenarios() {
    return {
        {GRID, "Grid", "Simple 3D grid (testing)", 1000, 1.0, 0.01},
        {GALAXY_DISK, "Galaxy Disk", "Spiral galaxy with rotation", 5000, 1.0, 0.005},
        {KEPLERIAN_DISK, "Keplerian Disk", "Orbiting disk around central mass", 3000, 50.0, 0.002},
        {BLACK_HOLE, "Black Hole", "Accretion disk around black hole", 3000, 200.0, 0.001},
        {GALAXY_COLLISION, "Galaxy Collision", "Two galaxies colliding", 4000, 1.0, 0.005}
    };
}

void createScenario(std::vector<Particle>& particles, ScenarioType type, int numParticles) {
    particles.clear();

    switch(type) {
        case GRID:
            createGrid(particles, (int)std::cbrt(numParticles));
            break;
        case GALAXY_DISK:
            createGalaxyDisk(particles, numParticles);
            break;
        case KEPLERIAN_DISK:
            createKeplerianDisk(particles, numParticles);
            break;
        case BLACK_HOLE:
            createBlackHole(particles, numParticles);
            break;
        case GALAXY_COLLISION:
            createGalaxyCollision(particles, numParticles / 2);
            break;
    }
}

void createGrid(std::vector<Particle>& particles, int gridSize) {
    for (int ix = 0; ix < gridSize; ix++) {
        for (int iy = 0; iy < gridSize; iy++) {
            for (int iz = 0; iz < gridSize; iz++) {
                Particle p;
                p.x = (ix - gridSize/2.0) * 5.0;
                p.y = (iy - gridSize/2.0) * 5.0;
                p.z = (iz - gridSize/2.0) * 5.0;
                p.mass = 0.1;
                p.vx = p.vy = p.vz = 0.0;
                particles.push_back(p);
            }
        }
    }
}

void createGalaxyDisk(std::vector<Particle>& particles, int numParticles) {
    double diskRadius = 50.0;
    double diskHeight = 2.0;
    double centralMass = 1000.0;

    Particle center;
    center.x = center.y = center.z = 0.0;
    center.vx = center.vy = center.vz = 0.0;
    center.mass = centralMass;
    particles.push_back(center);

    for (int i = 0; i < numParticles - 1; i++) {
        Particle p;
        double r = -diskRadius * 0.3 * std::log(randomUniform(0.01, 1.0));
        r = std::min(r, diskRadius);
        double theta = randomUniform(0.0, 2.0 * M_PI);

        p.x = r * std::cos(theta);
        p.y = r * std::sin(theta);
        p.z = randomNormal(0.0, diskHeight);

        double G = 1.0;
        double v_circular = std::sqrt(G * centralMass / (r + 1.0));

        p.vx = -v_circular * std::sin(theta);
        p.vy = v_circular * std::cos(theta);
        p.vz = randomNormal(0.0, 0.1);
        p.mass = 0.01;
        particles.push_back(p);
    }
}

void createKeplerianDisk(std::vector<Particle>& particles, int numParticles) {
    double innerRadius = 10.0;
    double outerRadius = 50.0;
    double centralMass = 5000.0;

    Particle center;
    center.x = center.y = center.z = 0.0;
    center.vx = center.vy = center.vz = 0.0;
    center.mass = centralMass;
    particles.push_back(center);

    for (int i = 0; i < numParticles - 1; i++) {
        Particle p;
        double r = randomUniform(innerRadius, outerRadius);
        double theta = randomUniform(0.0, 2.0 * M_PI);

        p.x = r * std::cos(theta);
        p.y = r * std::sin(theta);
        p.z = randomNormal(0.0, 0.5);

        double G = 50.0;
        double v_orbital = std::sqrt(G * centralMass / r);

        p.vx = -v_orbital * std::sin(theta);
        p.vy = v_orbital * std::cos(theta);
        p.vz = 0.0;
        p.mass = 0.001;
        particles.push_back(p);
    }
}

void createBlackHole(std::vector<Particle>& particles, int numParticles) {
    double innerRadius = 5.0;
    double outerRadius = 40.0;
    double blackHoleMass = 10000.0;

    Particle bh;
    bh.x = bh.y = bh.z = 0.0;
    bh.vx = bh.vy = bh.vz = 0.0;
    bh.mass = blackHoleMass;
    particles.push_back(bh);

    for (int i = 0; i < numParticles - 1; i++) {
        Particle p;
        double u = randomUniform(0.0, 1.0);
        double r = innerRadius * std::pow(outerRadius / innerRadius, u);
        double theta = randomUniform(0.0, 2.0 * M_PI);

        p.x = r * std::cos(theta);
        p.y = r * std::sin(theta);
        p.z = randomNormal(0.0, 0.3);

        double G = 200.0;
        double v_kep = std::sqrt(G * blackHoleMass / r);
        double v_perturb = randomNormal(0.0, v_kep * 0.05);

        p.vx = -(v_kep + v_perturb) * std::sin(theta);
        p.vy = (v_kep + v_perturb) * std::cos(theta);
        p.vz = randomNormal(0.0, 0.1);
        p.mass = 0.0001;
        particles.push_back(p);
    }
}

void createGalaxyCollision(std::vector<Particle>& particles, int particlesPerGalaxy) {
    double separation = 60.0;
    double velocity = 10.0;
    double centralMass = 200.0;

    Particle core1;
    core1.x = separation/2;
    core1.y = 0.0;
    core1.z = 0.0;
    core1.vx = -velocity;
    core1.vy = 0.0;
    core1.vz = 0.0;
    core1.mass = centralMass;
    particles.push_back(core1);

    for (int i = 0; i < particlesPerGalaxy; i++) {
        Particle p;
        double r = randomUniform(0.0, 25.0);
        double theta = randomUniform(0.0, 2.0 * M_PI);

        p.x = separation/2 + r * std::cos(theta);
        p.y = r * std::sin(theta);
        p.z = randomNormal(0.0, 2.0);

        double v_rot = std::sqrt(centralMass / (r + 5.0)) * 0.5;
        p.vx = -velocity - v_rot * std::sin(theta);
        p.vy = v_rot * std::cos(theta);
        p.vz = randomNormal(0.0, 0.1);
        p.mass = 0.5 / particlesPerGalaxy;
        particles.push_back(p);
    }

    Particle core2;
    core2.x = -separation/2;
    core2.y = 0.0;
    core2.z = 0.0;
    core2.vx = velocity;
    core2.vy = 0.0;
    core2.vz = 0.0;
    core2.mass = centralMass;
    particles.push_back(core2);

    for (int i = 0; i < particlesPerGalaxy; i++) {
        Particle p;
        double r = randomUniform(0.0, 25.0);
        double theta = randomUniform(0.0, 2.0 * M_PI);

        p.x = -separation/2 + r * std::cos(theta);
        p.y = r * std::sin(theta);
        p.z = randomNormal(0.0, 2.0);

        double v_rot = std::sqrt(centralMass / (r + 5.0)) * 0.5;
        p.vx = velocity - v_rot * std::sin(theta);
        p.vy = v_rot * std::cos(theta);
        p.vz = randomNormal(0.0, 0.1);
        p.mass = 0.5 / particlesPerGalaxy;
        particles.push_back(p);
    }
}
