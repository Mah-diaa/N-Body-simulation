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
        {BINARY_STARS, "Binary Stars", "Two stars orbiting with debris field", 3000, 1.0, 0.005},
        {GLOBULAR_CLUSTER, "Big Bang", "Collapse and explosive expansion", 4000, 1.0, 0.01},
        {STABLE_CLUSTER, "Globular Cluster", "Stable spherical star cluster", 3000, 1.0, 0.01},
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
        case BINARY_STARS:
            createBinaryStars(particles, numParticles);
            break;
        case GLOBULAR_CLUSTER:
            createGlobularCluster(particles, numParticles);
            break;
        case STABLE_CLUSTER:
            createStableCluster(particles, numParticles);
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

void createBinaryStars(std::vector<Particle>& particles, int numParticles) {
    double separation = 40.0;
    double starMass = 500.0;
    double G = 1.0;

    // For circular orbit: v = sqrt(G * M / (2 * separation))
    double orbitalVelocity = std::sqrt(G * starMass / (2.0 * separation));

    // Star 1
    Particle star1;
    star1.x = separation / 2;
    star1.y = 0.0;
    star1.z = 0.0;
    star1.vx = 0.0;
    star1.vy = orbitalVelocity;
    star1.vz = 0.0;
    star1.mass = starMass;
    particles.push_back(star1);

    // Star 2
    Particle star2;
    star2.x = -separation / 2;
    star2.y = 0.0;
    star2.z = 0.0;
    star2.vx = 0.0;
    star2.vy = -orbitalVelocity;
    star2.vz = 0.0;
    star2.mass = starMass;
    particles.push_back(star2);

    // Debris field around both stars
    int particlesPerStar = (numParticles - 2) / 2;

    for (int i = 0; i < particlesPerStar; i++) {
        // Particles around star 1
        Particle p1;
        double r = randomUniform(5.0, 15.0);
        double theta = randomUniform(0.0, 2.0 * M_PI);
        double phi = randomUniform(0.0, 2.0 * M_PI);

        p1.x = separation / 2 + r * std::cos(theta);
        p1.y = r * std::sin(theta) * std::cos(phi);
        p1.z = r * std::sin(theta) * std::sin(phi);

        double G = 1.0;
        double v = std::sqrt(G * starMass / r) * 0.7;
        p1.vx = -v * std::sin(theta);
        p1.vy = orbitalVelocity + v * std::cos(theta) * std::cos(phi);
        p1.vz = v * std::cos(theta) * std::sin(phi);
        p1.mass = 0.001;
        particles.push_back(p1);

        // Particles around star 2
        Particle p2;
        r = randomUniform(5.0, 15.0);
        theta = randomUniform(0.0, 2.0 * M_PI);
        phi = randomUniform(0.0, 2.0 * M_PI);

        p2.x = -separation / 2 + r * std::cos(theta);
        p2.y = r * std::sin(theta) * std::cos(phi);
        p2.z = r * std::sin(theta) * std::sin(phi);

        v = std::sqrt(G * starMass / r) * 0.7;
        p2.vx = -v * std::sin(theta);
        p2.vy = -orbitalVelocity + v * std::cos(theta) * std::cos(phi);
        p2.vz = v * std::cos(theta) * std::sin(phi);
        p2.mass = 0.001;
        particles.push_back(p2);
    }
}

void createGlobularCluster(std::vector<Particle>& particles, int numParticles) {
    double clusterRadius = 50.0;

    for (int i = 0; i < numParticles; i++) {
        Particle p;

        // Plummer sphere distribution - denser at center
        double r = clusterRadius / std::sqrt(std::pow(randomUniform(0.0, 1.0), -2.0/3.0) - 1.0);
        r = std::min(r, clusterRadius);

        // Random direction in 3D
        double theta = std::acos(2.0 * randomUniform(0.0, 1.0) - 1.0);
        double phi = randomUniform(0.0, 2.0 * M_PI);

        p.x = r * std::sin(theta) * std::cos(phi);
        p.y = r * std::sin(theta) * std::sin(phi);
        p.z = r * std::cos(theta);

        // Virial velocities - random motion with magnitude dependent on radius
        double velocityScale = 2.0 / std::sqrt(r + 1.0);
        double vtheta = std::acos(2.0 * randomUniform(0.0, 1.0) - 1.0);
        double vphi = randomUniform(0.0, 2.0 * M_PI);

        p.vx = velocityScale * std::sin(vtheta) * std::cos(vphi);
        p.vy = velocityScale * std::sin(vtheta) * std::sin(vphi);
        p.vz = velocityScale * std::cos(vtheta);

        p.mass = 1.0;
        particles.push_back(p);
    }
}

void createStableCluster(std::vector<Particle>& particles, int numParticles) {
    double plummerRadius = 20.0;  // Plummer scale length
    double particleMass = 1.0;
    double totalMass = numParticles * particleMass;
    double G = 1.0;

    // Virial velocity dispersion: sigma^2 = G * M / (6 * a)
    double velocityDispersion = std::sqrt(G * totalMass / (6.0 * plummerRadius));

    for (int i = 0; i < numParticles; i++) {
        Particle p;

        // Plummer sphere position distribution
        double r = plummerRadius / std::sqrt(std::pow(randomUniform(0.001, 1.0), -2.0/3.0) - 1.0);

        // Random direction in 3D (uniform on sphere)
        double theta = std::acos(2.0 * randomUniform(0.0, 1.0) - 1.0);
        double phi = randomUniform(0.0, 2.0 * M_PI);

        p.x = r * std::sin(theta) * std::cos(phi);
        p.y = r * std::sin(theta) * std::sin(phi);
        p.z = r * std::cos(theta);

        // Maxwell-Boltzmann velocity distribution (virial equilibrium)
        p.vx = randomNormal(0.0, velocityDispersion);
        p.vy = randomNormal(0.0, velocityDispersion);
        p.vz = randomNormal(0.0, velocityDispersion);

        p.mass = particleMass;
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
