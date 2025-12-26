#ifndef PARTICLE_H
#define PARTICLE_H

#include "raylib.h"

// Particle structure for N-body simulation
struct Particle {
    // Position
    double x, y, z;

    // Velocity
    double vx, vy, vz;

    // Acceleration
    double ax, ay, az;

    // Mass
    double mass;

    // Rendering properties
    float renderRadius;  // Visual size for rendering
    Color color;         // Particle color

    // Constructor
    Particle() : x(0), y(0), z(0),
                 vx(0), vy(0), vz(0),
                 ax(0), ay(0), az(0),
                 mass(1.0),
                 renderRadius(0.5f),
                 color({200, 250, 255, 255}) {}

    Particle(double px, double py, double pz, double m)
        : x(px), y(py), z(pz),
          vx(0), vy(0), vz(0),
          ax(0), ay(0), az(0),
          mass(m),
          renderRadius(0.5f),
          color({200, 250, 255, 255}) {}
};

#endif // PARTICLE_H
