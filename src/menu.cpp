#include "menu.h"
#include <cmath>
#include <cstddef>  // For size_t on Windows/MSVC

// Button implementation
bool Button::isHovered(Vector2 mousePos) const {
    return CheckCollisionPointRec(mousePos, bounds);
}

bool Button::isClicked(Vector2 mousePos) const {
    return isHovered(mousePos) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

void Button::draw() const {
    Vector2 mousePos = GetMousePosition();
    Color color = isHovered(mousePos) ? hoverColor : normalColor;

    DrawRectangleRec(bounds, color);
    DrawRectangleLinesEx(bounds, 2, WHITE);

    // Center text
    int textWidth = MeasureText(text, 20);
    DrawText(text,
             bounds.x + (bounds.width - textWidth) / 2,
             bounds.y + (bounds.height - 20) / 2,
             20, WHITE);
}

// Slider implementation
void Slider::update(Vector2 mousePos) {
    if (!enabled) return;

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
        CheckCollisionPointRec(mousePos, bounds)) {
        float percent = (mousePos.x - bounds.x) / bounds.width;
        percent = fmaxf(0.0f, fminf(1.0f, percent));
        *currentValue = minValue + (int)(percent * (maxValue - minValue));
    }
}

void Slider::draw() const {
    // Draw label
    DrawText(label, bounds.x, bounds.y - 25, 16, WHITE);
    DrawText(TextFormat("%d particles", *currentValue), bounds.x + bounds.width + 10, bounds.y, 16, LIGHTGRAY);

    if (!enabled) {
        DrawText("(Fixed)", bounds.x + bounds.width + 150, bounds.y, 16, GRAY);
    }

    // Draw track
    Color trackColor = enabled ? DARKGRAY : GRAY;
    DrawRectangleRec(bounds, trackColor);

    // Draw handle
    float percent = (float)(*currentValue - minValue) / (maxValue - minValue);
    float handleX = bounds.x + percent * bounds.width;
    Color handleColor = enabled ? SKYBLUE : GRAY;
    DrawCircle(handleX, bounds.y + bounds.height / 2, 8, handleColor);
}

// Main menu rendering
void renderMenu(int screenWidth, int screenHeight,
                const std::vector<Scenario>& scenarios,
                int& selectedScenarioIndex,
                int& particleCount,
                bool& trailsEnabled,
                bool& startSimulation,
                bool& startDemo,
                std::string& textInput) {

    ClearBackground(BLACK);

    // Title
    const char* title = "N-BODY SIMULATION";
    int titleSize = 40;
    int titleWidth = MeasureText(title, titleSize);
    DrawText(title, screenWidth/2 - titleWidth/2, 30, titleSize, WHITE);
    DrawText("==================", screenWidth/2 - 160, 75, titleSize, WHITE);

    // Scenario selection
    DrawText("SELECT SCENARIO:", 50, 120, 20, WHITE);

    Vector2 mousePos = GetMousePosition();

    // Draw scenario list
    int startY = 160;
    int itemHeight = 45;
    for (size_t i = 0; i < scenarios.size(); i++) {
        Rectangle scenarioRect = {50, (float)(startY + i * itemHeight), 700, (float)(itemHeight - 5)};

        bool hovered = CheckCollisionPointRec(mousePos, scenarioRect);
        bool selected = (i == (size_t)selectedScenarioIndex);

        // Background
        Color bgColor = selected ? (Color){30, 60, 140, 255} : (hovered ? (Color){40, 40, 60, 255} : (Color){20, 20, 30, 255});
        DrawRectangleRec(scenarioRect, bgColor);
        DrawRectangleLinesEx(scenarioRect, 1, selected ? YELLOW : DARKGRAY);

        // Text
        Color textColor = selected ? YELLOW : (hovered ? WHITE : LIGHTGRAY);
        DrawText(TextFormat("%d. %s", (int)i + 1, scenarios[i].name.c_str()),
                 scenarioRect.x + 10, scenarioRect.y + 5, 16, textColor);
        DrawText(scenarios[i].description.c_str(),
                 scenarioRect.x + 10, scenarioRect.y + 23, 12, GRAY);

        // Handle click
        if (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            selectedScenarioIndex = i;

            // Update particle count to recommended
            const Scenario& scenario = scenarios[i];
            if (scenario.type != BINARY_ORBIT && scenario.type != FIGURE_8 &&
                scenario.type != SOLAR_SYSTEM && scenario.type != TEXT) {
                particleCount = scenario.recommendedParticles;
            }
        }
    }

    // Particle count slider
    int sliderY = startY + scenarios.size() * itemHeight + 30;
    Slider particleSlider;
    particleSlider.bounds = {50, (float)sliderY, 500, 10};
    particleSlider.minValue = 100;
    particleSlider.maxValue = 10000;
    particleSlider.currentValue = &particleCount;
    particleSlider.label = "PARTICLE COUNT:";

    // Disable slider for fixed scenarios
    const Scenario& selectedScenario = scenarios[selectedScenarioIndex];
    particleSlider.enabled = (selectedScenario.type != BINARY_ORBIT &&
                              selectedScenario.type != FIGURE_8 &&
                              selectedScenario.type != SOLAR_SYSTEM &&
                              selectedScenario.type != TEXT);

    if (!particleSlider.enabled) {
        particleCount = selectedScenario.recommendedParticles;
    }

    particleSlider.update(mousePos);
    particleSlider.draw();

    // Trails toggle
    int toggleY = sliderY + 60;
    DrawText("TRAILS:", 50, toggleY, 20, WHITE);

    Rectangle toggleRect = {150, (float)toggleY, 60, 30};
    bool toggleHovered = CheckCollisionPointRec(mousePos, toggleRect);

    if (toggleHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        trailsEnabled = !trailsEnabled;
    }

    Color toggleColor = toggleHovered ? (Color){50, 50, 70, 255} : (Color){30, 30, 40, 255};
    DrawRectangleRec(toggleRect, toggleColor);
    DrawRectangleLinesEx(toggleRect, 2, trailsEnabled ? GREEN : GRAY);
    DrawText(trailsEnabled ? "ON" : "OFF", toggleRect.x + 15, toggleRect.y + 5, 20,
             trailsEnabled ? GREEN : GRAY);

    // Text input for TEXT scenario
    if (selectedScenario.type == TEXT) {
        int textInputY = toggleY + 60;
        DrawText("TEXT INPUT:", 50, textInputY, 20, WHITE);
        DrawText("(Type in terminal for now)", 50, textInputY + 25, 14, GRAY);
        if (!textInput.empty()) {
            DrawText(TextFormat("Current: %s", textInput.c_str()), 50, textInputY + 45, 16, YELLOW);
        }
    }

    // Start button
    Button startButton;
    startButton.bounds = {(float)screenWidth/2 - 360, (float)screenHeight - 120, 280, 60};
    startButton.text = "START SIMULATION";
    startButton.normalColor = (Color){20, 100, 20, 255};
    startButton.hoverColor = (Color){30, 150, 30, 255};

    startButton.draw();

    if (startButton.isClicked(mousePos)) {
        startSimulation = true;
    }

    // Algorithm Demo button
    Button demoButton;
    demoButton.bounds = {(float)screenWidth/2 + 80, (float)screenHeight - 120, 280, 60};
    demoButton.text = "ALGORITHM DEMO";
    demoButton.normalColor = (Color){100, 20, 100, 255};
    demoButton.hoverColor = (Color){150, 30, 150, 255};

    demoButton.draw();

    if (demoButton.isClicked(mousePos)) {
        startDemo = true;
    }

    // Controls info
    DrawText("CONTROLS: ESC = Exit Program", 20, screenHeight - 30, 14, GRAY);
}
