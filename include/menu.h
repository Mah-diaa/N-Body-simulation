#ifndef MENU_H
#define MENU_H

#include "raylib.h"
#include "scenarios.h"
#include <vector>

// Simple button structure
struct Button {
    Rectangle bounds;
    const char* text;
    Color normalColor;
    Color hoverColor;

    bool isHovered(Vector2 mousePos) const;
    bool isClicked(Vector2 mousePos) const;
    void draw() const;
};

// Slider for particle count
struct Slider {
    Rectangle bounds;
    int minValue;
    int maxValue;
    int* currentValue;
    const char* label;
    bool enabled;

    void update(Vector2 mousePos);
    void draw() const;
};

// Menu rendering function
void renderMenu(int screenWidth, int screenHeight,
                const std::vector<Scenario>& scenarios,
                int& selectedScenarioIndex,
                int& particleCount,
                bool& trailsEnabled,
                bool& startSimulation,
                bool& startDemo,
                bool& startQuadTreeDemo);

#endif // MENU_H
