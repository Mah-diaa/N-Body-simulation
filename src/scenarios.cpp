#include "scenarios.h"
#include <cmath>
#include <random>
#include <algorithm>
#include <map>
#include <cctype>

// Random number generator
static std::random_device rd;
static std::mt19937 gen(rd());

// Helper: Random uniform in range [min, max]
double randomUniform(double min, double max) {
    std::uniform_real_distribution<> dis(min, max);
    return dis(gen);
}

// Helper: Random normal distribution
double randomNormal(double mean, double stddev) {
    std::normal_distribution<> dis(mean, stddev);
    return dis(gen);
}

// Get list of all scenarios
std::vector<Scenario> getScenarios() {
    return {
        {GRID, "Grid", "Simple 3D grid (testing)", 1000, 1.0, 0.01},
        {PLUMMER_SPHERE, "Plummer Sphere", "Globular star cluster", 2000, 0.1, 0.01},
        {GALAXY_DISK, "Galaxy Disk", "Spiral galaxy with rotation", 5000, 1.0, 0.005},
        {KEPLERIAN_DISK, "Keplerian Disk", "Orbiting disk around central mass", 3000, 50.0, 0.002},
        {BINARY_ORBIT, "Binary Stars", "Two stars in circular orbit", 2, 100.0, 0.001},
        {SOLAR_SYSTEM, "Solar System", "Sun with 8 planets", 9, 1000.0, 0.00003},
        {BLACK_HOLE, "Black Hole", "Accretion disk around black hole", 3000, 200.0, 0.001},
        {GALAXY_COLLISION, "Galaxy Collision", "Two galaxies colliding", 4000, 1.0, 0.005},
        {FIGURE_8, "Figure-8 Orbit", "Three-body periodic orbit", 3, 1.0, 0.0001},
        {TEXT, "Text", "Type your own text to shoot at!", 500, 0.1, 0.01}
    };
}

// Main scenario creator
void createScenario(std::vector<Particle>& particles, ScenarioType type, int numParticles) {
    particles.clear();

    switch(type) {
        case GRID:
            createGrid(particles, (int)std::cbrt(numParticles));
            break;
        case PLUMMER_SPHERE:
            createPlummerSphere(particles, numParticles);
            break;
        case GALAXY_DISK:
            createGalaxyDisk(particles, numParticles);
            break;
        case KEPLERIAN_DISK:
            createKeplerianDisk(particles, numParticles);
            break;
        case BINARY_ORBIT:
            createBinaryOrbit(particles);
            break;
        case SOLAR_SYSTEM:
            createSolarSystem(particles);
            break;
        case BLACK_HOLE:
            createBlackHole(particles, numParticles);
            break;
        case GALAXY_COLLISION:
            createGalaxyCollision(particles, numParticles / 2);
            break;
        case FIGURE_8:
            createFigure8(particles);
            break;
        case TEXT:
            // Text is handled separately with createScenarioWithText
            break;
    }
}

// Wrapper for scenarios that need text input
void createScenarioWithText(std::vector<Particle>& particles, ScenarioType type, int numParticles, const std::string& text) {
    particles.clear();
    if (type == TEXT) {
        createText(particles, text);
    } else {
        createScenario(particles, type, numParticles);
    }
}

// Simple grid (original)
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

// Plummer sphere - realistic globular cluster
void createPlummerSphere(std::vector<Particle>& particles, int numParticles) {
    double plummerRadius = 20.0;  // Scale radius

    for (int i = 0; i < numParticles; i++) {
        Particle p;

        // Sample radius from Plummer distribution using inverse transform
        double r_rand = randomUniform(0.0, 1.0);
        double r = plummerRadius / std::sqrt(std::pow(r_rand, -2.0/3.0) - 1.0);

        // Random direction (uniform on sphere)
        double theta = randomUniform(0.0, M_PI);
        double phi = randomUniform(0.0, 2.0 * M_PI);

        p.x = r * std::sin(theta) * std::cos(phi);
        p.y = r * std::sin(theta) * std::sin(phi);
        p.z = r * std::cos(theta);

        // Velocity: virial equilibrium (simplified)
        double v_mag = std::sqrt(0.1 / (r + plummerRadius));  // Escape velocity scaling
        double v_theta = randomUniform(0.0, M_PI);
        double v_phi = randomUniform(0.0, 2.0 * M_PI);

        p.vx = v_mag * std::sin(v_theta) * std::cos(v_phi);
        p.vy = v_mag * std::sin(v_theta) * std::sin(v_phi);
        p.vz = v_mag * std::cos(v_theta);

        p.mass = 1.0 / numParticles;  // Equal mass particles
        particles.push_back(p);
    }
}

// Galaxy disk with rotation
void createGalaxyDisk(std::vector<Particle>& particles, int numParticles) {
    double diskRadius = 50.0;
    double diskHeight = 2.0;
    double centralMass = 1000.0;

    // Add central bulge
    Particle center;
    center.x = center.y = center.z = 0.0;
    center.vx = center.vy = center.vz = 0.0;
    center.mass = centralMass;
    particles.push_back(center);

    // Add disk particles
    for (int i = 0; i < numParticles - 1; i++) {
        Particle p;

        // Exponential disk profile
        double r = -diskRadius * 0.3 * std::log(randomUniform(0.01, 1.0));
        r = std::min(r, diskRadius);

        double theta = randomUniform(0.0, 2.0 * M_PI);

        p.x = r * std::cos(theta);
        p.y = r * std::sin(theta);
        p.z = randomNormal(0.0, diskHeight);  // Thin disk

        // Circular velocity for stable rotation
        double G = 1.0;  // Must match the scenario's recommended G
        double v_circular = std::sqrt(G * centralMass / (r + 1.0));

        p.vx = -v_circular * std::sin(theta);
        p.vy = v_circular * std::cos(theta);
        p.vz = randomNormal(0.0, 0.1);  // Small vertical motion

        p.mass = 0.01;
        particles.push_back(p);
    }
}

// Keplerian disk around central mass
void createKeplerianDisk(std::vector<Particle>& particles, int numParticles) {
    double innerRadius = 10.0;
    double outerRadius = 50.0;
    double centralMass = 5000.0;

    // Central massive object (black hole / star)
    Particle center;
    center.x = center.y = center.z = 0.0;
    center.vx = center.vy = center.vz = 0.0;
    center.mass = centralMass;
    particles.push_back(center);

    // Orbiting particles
    for (int i = 0; i < numParticles - 1; i++) {
        Particle p;

        // Radius with some distribution
        double r = randomUniform(innerRadius, outerRadius);
        double theta = randomUniform(0.0, 2.0 * M_PI);

        p.x = r * std::cos(theta);
        p.y = r * std::sin(theta);
        p.z = randomNormal(0.0, 0.5);  // Very thin disk

        // Perfect Keplerian velocity: v = sqrt(GM/r)
        double G = 50.0;  // Must match the scenario's recommended G
        double v_orbital = std::sqrt(G * centralMass / r);

        p.vx = -v_orbital * std::sin(theta);
        p.vy = v_orbital * std::cos(theta);
        p.vz = 0.0;

        p.mass = 0.001;  // Very light compared to central mass
        particles.push_back(p);
    }
}

// Binary star system
void createBinaryOrbit(std::vector<Particle>& particles) {
    double separation = 30.0;
    double totalMass = 100.0;
    double mass1 = 50.0;
    double mass2 = 50.0;

    // Center of mass is at origin
    double r1 = separation * mass2 / totalMass;
    double r2 = separation * mass1 / totalMass;

    // Orbital velocity for circular orbit
    double v1 = std::sqrt(mass2 * mass2 / (totalMass * separation));
    double v2 = std::sqrt(mass1 * mass1 / (totalMass * separation));

    Particle star1, star2;

    star1.x = r1;
    star1.y = 0.0;
    star1.z = 0.0;
    star1.vx = 0.0;
    star1.vy = v1;
    star1.vz = 0.0;
    star1.mass = mass1;

    star2.x = -r2;
    star2.y = 0.0;
    star2.z = 0.0;
    star2.vx = 0.0;
    star2.vy = -v2;
    star2.vz = 0.0;
    star2.mass = mass2;

    particles.push_back(star1);
    particles.push_back(star2);
}

// Solar system (realistic)
void createSolarSystem(std::vector<Particle>& particles) {
    double G = 1000.0;  // Must match scenario G
    double sunMass = 1000.0;

    // Sun
    Particle sun;
    sun.x = sun.y = sun.z = 0.0;
    sun.vx = sun.vy = sun.vz = 0.0;
    sun.mass = sunMass;
    sun.renderRadius = 3.0f;  // Large, visible sun
    sun.color = (Color){255, 220, 0, 255};  // Yellow
    particles.push_back(sun);

    // Planet data: {orbitRadius, mass, actualRadius (for rendering), color}
    struct PlanetData {
        double orbitRadius;
        double mass;
        float renderRadius;
        Color color;
        const char* name;
    };

    PlanetData planets[] = {
        {8.0,  5.0,    0.3f, (Color){169, 169, 169, 255}, "Mercury"},  // Gray - boosted mass for sphere rendering
        {12.0, 8.0,    0.5f, (Color){255, 198, 73, 255},  "Venus"},    // Yellow-orange - boosted mass
        {16.0, 10.0,   0.5f, (Color){100, 149, 237, 255}, "Earth"},    // Blue - boosted mass
        {22.0, 5.0,    0.35f, (Color){188, 39, 50, 255},  "Mars"},     // Red - boosted mass
        {40.0, 318.0,  2.0f, (Color){201, 149, 91, 255},  "Jupiter"},  // Orange-brown
        {55.0, 95.0,   1.7f, (Color){238, 203, 173, 255}, "Saturn"},   // Pale gold
        {75.0, 14.5,   0.9f, (Color){79, 208, 231, 255},  "Uranus"},   // Cyan
        {90.0, 17.1,   0.9f, (Color){63, 84, 186, 255},   "Neptune"}   // Deep blue
    };

    for (int i = 0; i < 8; i++) {
        Particle planet;
        double r = planets[i].orbitRadius;
        double theta = randomUniform(0.0, 2.0 * M_PI);

        planet.x = r * std::cos(theta);
        planet.y = r * std::sin(theta);
        planet.z = randomNormal(0.0, 0.1);

        // Correct orbital velocity: v = sqrt(G * M_sun / r)
        double v = std::sqrt(G * sunMass / r);
        planet.vx = -v * std::sin(theta);
        planet.vy = v * std::cos(theta);
        planet.vz = 0.0;

        planet.mass = planets[i].mass;
        planet.renderRadius = planets[i].renderRadius;
        planet.color = planets[i].color;

        particles.push_back(planet);
    }
}

// Black hole accretion disk
void createBlackHole(std::vector<Particle>& particles, int numParticles) {
    double innerRadius = 5.0;   // Inner edge (event horizon region)
    double outerRadius = 40.0;
    double blackHoleMass = 10000.0;

    // Central black hole
    Particle bh;
    bh.x = bh.y = bh.z = 0.0;
    bh.vx = bh.vy = bh.vz = 0.0;
    bh.mass = blackHoleMass;
    particles.push_back(bh);

    // Accretion disk
    for (int i = 0; i < numParticles - 1; i++) {
        Particle p;

        // Power-law distribution (more particles at inner edge)
        double u = randomUniform(0.0, 1.0);
        double r = innerRadius * std::pow(outerRadius / innerRadius, u);
        double theta = randomUniform(0.0, 2.0 * M_PI);

        p.x = r * std::cos(theta);
        p.y = r * std::sin(theta);
        p.z = randomNormal(0.0, 0.3);  // Very thin disk

        // Keplerian + small perturbation for spiral structure
        // Note: G=200 for this scenario, so v = sqrt(G*M/r)
        double G = 200.0;  // Must match the scenario's recommended G
        double v_kep = std::sqrt(G * blackHoleMass / r);
        double v_perturb = randomNormal(0.0, v_kep * 0.05);

        p.vx = -(v_kep + v_perturb) * std::sin(theta);
        p.vy = (v_kep + v_perturb) * std::cos(theta);
        p.vz = randomNormal(0.0, 0.1);

        p.mass = 0.0001;
        particles.push_back(p);
    }
}

// Two galaxies colliding
void createGalaxyCollision(std::vector<Particle>& particles, int particlesPerGalaxy) {
    double separation = 60.0;  // Reduced from 100 for faster collision
    double velocity = 10.0;     // Increased from 5.0 for more dramatic collision
    double galaxyMass = 500.0;
    double centralMass = 200.0; // Add massive cores for more dramatic interaction

    // First galaxy (right side, moving left)
    // Add central massive object
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

        double r = randomUniform(0.0, 25.0);  // Slightly smaller galaxies
        double theta = randomUniform(0.0, 2.0 * M_PI);

        p.x = separation/2 + r * std::cos(theta);
        p.y = r * std::sin(theta);
        p.z = randomNormal(0.0, 2.0);

        // Rotating + moving toward each other
        double v_rot = std::sqrt(centralMass / (r + 5.0)) * 0.5; // Faster rotation
        p.vx = -velocity - v_rot * std::sin(theta);
        p.vy = v_rot * std::cos(theta);
        p.vz = randomNormal(0.0, 0.1);

        p.mass = 0.5 / particlesPerGalaxy;
        particles.push_back(p);
    }

    // Second galaxy (left side, moving right)
    // Add central massive object
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

        double r = randomUniform(0.0, 25.0);  // Slightly smaller galaxies
        double theta = randomUniform(0.0, 2.0 * M_PI);

        p.x = -separation/2 + r * std::cos(theta);
        p.y = r * std::sin(theta);
        p.z = randomNormal(0.0, 2.0);

        // Rotating + moving toward each other
        double v_rot = std::sqrt(centralMass / (r + 5.0)) * 0.5; // Faster rotation
        p.vx = velocity - v_rot * std::sin(theta);
        p.vy = v_rot * std::cos(theta);
        p.vz = randomNormal(0.0, 0.1);

        p.mass = 0.5 / particlesPerGalaxy;
        particles.push_back(p);
    }
}

// Figure-8 three-body orbit (famous periodic solution)
void createFigure8(std::vector<Particle>& particles) {
    // Initial conditions for figure-8 orbit (Chenciner & Montgomery, 2000)
    Particle p1, p2, p3;

    p1.x = -0.97000436;
    p1.y = 0.24308753;
    p1.z = 0.0;
    p1.vx = 0.4662036850;
    p1.vy = 0.4323657300;
    p1.vz = 0.0;
    p1.mass = 1.0;

    p2.x = 0.97000436;
    p2.y = -0.24308753;
    p2.z = 0.0;
    p2.vx = 0.4662036850;
    p2.vy = 0.4323657300;
    p2.vz = 0.0;
    p2.mass = 1.0;

    p3.x = 0.0;
    p3.y = 0.0;
    p3.z = 0.0;
    p3.vx = -2.0 * 0.4662036850;
    p3.vy = -2.0 * 0.4323657300;
    p3.vz = 0.0;
    p3.mass = 1.0;

    // Scale up for better visualization
    double scale = 20.0;
    p1.x *= scale; p1.y *= scale; p1.z *= scale;
    p2.x *= scale; p2.y *= scale; p2.z *= scale;
    p3.x *= scale; p3.y *= scale; p3.z *= scale;

    particles.push_back(p1);
    particles.push_back(p2);
    particles.push_back(p3);
}

// Text scenario - create particles forming letters
void createText(std::vector<Particle>& particles, const std::string& text) {
    // 5x7 bitmap font (1 = particle, 0 = empty)
    std::map<char, std::vector<std::vector<int>>> font = {
        {'A', {{0,1,1,1,0}, {1,0,0,0,1}, {1,0,0,0,1}, {1,1,1,1,1}, {1,0,0,0,1}, {1,0,0,0,1}, {1,0,0,0,1}}},
        {'B', {{1,1,1,1,0}, {1,0,0,0,1}, {1,0,0,0,1}, {1,1,1,1,0}, {1,0,0,0,1}, {1,0,0,0,1}, {1,1,1,1,0}}},
        {'C', {{0,1,1,1,0}, {1,0,0,0,1}, {1,0,0,0,0}, {1,0,0,0,0}, {1,0,0,0,0}, {1,0,0,0,1}, {0,1,1,1,0}}},
        {'D', {{1,1,1,1,0}, {1,0,0,0,1}, {1,0,0,0,1}, {1,0,0,0,1}, {1,0,0,0,1}, {1,0,0,0,1}, {1,1,1,1,0}}},
        {'E', {{1,1,1,1,1}, {1,0,0,0,0}, {1,0,0,0,0}, {1,1,1,1,0}, {1,0,0,0,0}, {1,0,0,0,0}, {1,1,1,1,1}}},
        {'F', {{1,1,1,1,1}, {1,0,0,0,0}, {1,0,0,0,0}, {1,1,1,1,0}, {1,0,0,0,0}, {1,0,0,0,0}, {1,0,0,0,0}}},
        {'G', {{0,1,1,1,0}, {1,0,0,0,1}, {1,0,0,0,0}, {1,0,1,1,1}, {1,0,0,0,1}, {1,0,0,0,1}, {0,1,1,1,0}}},
        {'H', {{1,0,0,0,1}, {1,0,0,0,1}, {1,0,0,0,1}, {1,1,1,1,1}, {1,0,0,0,1}, {1,0,0,0,1}, {1,0,0,0,1}}},
        {'I', {{1,1,1,1,1}, {0,0,1,0,0}, {0,0,1,0,0}, {0,0,1,0,0}, {0,0,1,0,0}, {0,0,1,0,0}, {1,1,1,1,1}}},
        {'J', {{0,0,0,0,1}, {0,0,0,0,1}, {0,0,0,0,1}, {0,0,0,0,1}, {0,0,0,0,1}, {1,0,0,0,1}, {0,1,1,1,0}}},
        {'K', {{1,0,0,0,1}, {1,0,0,1,0}, {1,0,1,0,0}, {1,1,0,0,0}, {1,0,1,0,0}, {1,0,0,1,0}, {1,0,0,0,1}}},
        {'L', {{1,0,0,0,0}, {1,0,0,0,0}, {1,0,0,0,0}, {1,0,0,0,0}, {1,0,0,0,0}, {1,0,0,0,0}, {1,1,1,1,1}}},
        {'M', {{1,0,0,0,1}, {1,1,0,1,1}, {1,0,1,0,1}, {1,0,0,0,1}, {1,0,0,0,1}, {1,0,0,0,1}, {1,0,0,0,1}}},
        {'N', {{1,0,0,0,1}, {1,1,0,0,1}, {1,0,1,0,1}, {1,0,0,1,1}, {1,0,0,0,1}, {1,0,0,0,1}, {1,0,0,0,1}}},
        {'O', {{0,1,1,1,0}, {1,0,0,0,1}, {1,0,0,0,1}, {1,0,0,0,1}, {1,0,0,0,1}, {1,0,0,0,1}, {0,1,1,1,0}}},
        {'P', {{1,1,1,1,0}, {1,0,0,0,1}, {1,0,0,0,1}, {1,1,1,1,0}, {1,0,0,0,0}, {1,0,0,0,0}, {1,0,0,0,0}}},
        {'Q', {{0,1,1,1,0}, {1,0,0,0,1}, {1,0,0,0,1}, {1,0,0,0,1}, {1,0,1,0,1}, {1,0,0,1,0}, {0,1,1,0,1}}},
        {'R', {{1,1,1,1,0}, {1,0,0,0,1}, {1,0,0,0,1}, {1,1,1,1,0}, {1,0,1,0,0}, {1,0,0,1,0}, {1,0,0,0,1}}},
        {'S', {{0,1,1,1,1}, {1,0,0,0,0}, {1,0,0,0,0}, {0,1,1,1,0}, {0,0,0,0,1}, {0,0,0,0,1}, {1,1,1,1,0}}},
        {'T', {{1,1,1,1,1}, {0,0,1,0,0}, {0,0,1,0,0}, {0,0,1,0,0}, {0,0,1,0,0}, {0,0,1,0,0}, {0,0,1,0,0}}},
        {'U', {{1,0,0,0,1}, {1,0,0,0,1}, {1,0,0,0,1}, {1,0,0,0,1}, {1,0,0,0,1}, {1,0,0,0,1}, {0,1,1,1,0}}},
        {'V', {{1,0,0,0,1}, {1,0,0,0,1}, {1,0,0,0,1}, {1,0,0,0,1}, {1,0,0,0,1}, {0,1,0,1,0}, {0,0,1,0,0}}},
        {'W', {{1,0,0,0,1}, {1,0,0,0,1}, {1,0,0,0,1}, {1,0,0,0,1}, {1,0,1,0,1}, {1,1,0,1,1}, {1,0,0,0,1}}},
        {'X', {{1,0,0,0,1}, {1,0,0,0,1}, {0,1,0,1,0}, {0,0,1,0,0}, {0,1,0,1,0}, {1,0,0,0,1}, {1,0,0,0,1}}},
        {'Y', {{1,0,0,0,1}, {1,0,0,0,1}, {0,1,0,1,0}, {0,0,1,0,0}, {0,0,1,0,0}, {0,0,1,0,0}, {0,0,1,0,0}}},
        {'Z', {{1,1,1,1,1}, {0,0,0,0,1}, {0,0,0,1,0}, {0,0,1,0,0}, {0,1,0,0,0}, {1,0,0,0,0}, {1,1,1,1,1}}},
        {' ', {{0,0,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0}}},
        {'!', {{0,0,1,0,0}, {0,0,1,0,0}, {0,0,1,0,0}, {0,0,1,0,0}, {0,0,1,0,0}, {0,0,0,0,0}, {0,0,1,0,0}}}
    };

    double spacing = 2.5;  // Space between particles within a letter
    double letterSpacing = 15.0;  // Space between letters (5 pixels * 2.5 + gap)
    double offsetX = -((double)text.length() * letterSpacing) / 2.0;  // Center the text
    
    for (size_t i = 0; i < text.length(); i++) {
        char c = std::toupper(text[i]);  // Convert to uppercase
        
        if (font.find(c) == font.end()) {
            continue;  // Skip unknown characters
        }
        
        auto& pattern = font[c];
        double baseX = offsetX + i * letterSpacing;
        
        // Create particles for this letter
        for (size_t row = 0; row < pattern.size(); row++) {
            for (size_t col = 0; col < pattern[row].size(); col++) {
                if (pattern[row][col] == 1) {
                    Particle p;
                    p.x = baseX + col * spacing;
                    p.y = (3.5 - row) * spacing;  // Center vertically (7 rows / 2)
                    p.z = 0.0;
                    p.vx = p.vy = p.vz = 0.0;
                    p.mass = 10.0;  // Heavier particles so projectiles affect them
                    particles.push_back(p);
                }
            }
        }
    }
}
