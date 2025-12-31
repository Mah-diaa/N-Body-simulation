#include "demos/quadtree_demo.h"
#include <cmath>
#include <sstream>
#include <iomanip>
#include <algorithm>

QuadTreeDemo::QuadTreeDemo() : quadTree(100.0), currentParticleIndex(0) {
    allParticles.push_back(Particle2DColored(20, 15, 3.0, RED));
    allParticles.push_back(Particle2DColored(-18, 20, 2.0, BLUE));
    allParticles.push_back(Particle2DColored(-15, -18, 2.5, GREEN));
    allParticles.push_back(Particle2DColored(18, -20, 1.5, YELLOW));
    allParticles.push_back(Particle2DColored(10, 25, 2.0, ORANGE));
    allParticles.push_back(Particle2DColored(25, -10, 1.0, PINK));
    reset();
}

QuadTreeDemo::~QuadTreeDemo() {}

void QuadTreeDemo::reset() {
    currentParticleIndex = 0;
    activeParticles.clear();
    quadTree.clear();
}

void QuadTreeDemo::addNextParticle() {
    if (currentParticleIndex < (int)allParticles.size()) {
        currentParticleIndex++;
        activeParticles.clear();
        std::vector<Particle2D> particles;
        for (int i = 0; i < currentParticleIndex; i++) {
            activeParticles.push_back(allParticles[i]);
            particles.push_back(Particle2D(allParticles[i].x, allParticles[i].y, allParticles[i].mass));
        }
        quadTree.buildTree(particles);
    }
}

void QuadTreeDemo::update() {}

Vector2 QuadTreeDemo::worldToScreen(double wx, double wy, int viewX, int viewY, int viewWidth, int viewHeight) {
    double scale = viewHeight / quadTree.getWorldSize();
    int centerX = viewX + viewWidth / 2;
    int centerY = viewY + viewHeight / 2;
    return {(float)(centerX + (wx - quadTree.getWorldCenterX()) * scale),
            (float)(centerY - (wy - quadTree.getWorldCenterY()) * scale)};
}

void QuadTreeDemo::drawParticles(int viewX, int viewY, int viewWidth, int viewHeight) {
    for (int i = 0; i < (int)activeParticles.size(); i++) {
        const Particle2DColored& p = activeParticles[i];
        Vector2 pos = worldToScreen(p.x, p.y, viewX, viewY, viewWidth, viewHeight);
        float radius = 10.0f + p.mass * 2.5f;

        DrawCircleV(pos, radius + 3, Fade(p.color, 0.3f));
        DrawCircleV(pos, radius, p.color);
        DrawCircleLines(pos.x, pos.y, radius, WHITE);

        std::ostringstream oss;
        oss << "P" << (i + 1);
        int labelWidth = MeasureText(oss.str().c_str(), 14);
        DrawText(oss.str().c_str(), pos.x - labelWidth/2, pos.y - radius - 25, 14, YELLOW);

        oss.str("");
        oss << "m=" << std::fixed << std::setprecision(1) << p.mass;
        int massWidth = MeasureText(oss.str().c_str(), 12);
        DrawText(oss.str().c_str(), pos.x - massWidth/2, pos.y + radius + 8, 12, WHITE);
    }

    if (currentParticleIndex < (int)allParticles.size()) {
        const Particle2DColored& nextP = allParticles[currentParticleIndex];
        Vector2 pos = worldToScreen(nextP.x, nextP.y, viewX, viewY, viewWidth, viewHeight);
        float pulse = (sin(GetTime() * 5.0f) + 1.0f) / 2.0f;
        float radius = 10.0f + nextP.mass * 2.5f;

        DrawCircleV(pos, radius + pulse * 8.0f, Fade(nextP.color, 0.2f));
        DrawCircleV(pos, radius, Fade(nextP.color, 0.5f));

        const char* nextLabel = "NEXT";
        int nextWidth = MeasureText(nextLabel, 16);
        DrawRectangle(pos.x - nextWidth/2 - 5, pos.y + radius + 10, nextWidth + 10, 22, Fade(YELLOW, 0.8f));
        DrawText(nextLabel, pos.x - nextWidth/2, pos.y + radius + 12, 16, BLACK);

        std::ostringstream oss;
        oss << "P" << (currentParticleIndex + 1) << " (m=" << std::fixed << std::setprecision(1) << nextP.mass << ")";
        int infoWidth = MeasureText(oss.str().c_str(), 12);
        DrawText(oss.str().c_str(), pos.x - infoWidth/2, pos.y - radius - 20, 12, YELLOW);
    }
}

void QuadTreeDemo::drawQuadTreeBounds(int nodeIdx, int viewX, int viewY, int viewWidth, int viewHeight) {
    const auto& nodes = quadTree.getNodes();
    if (nodeIdx < 0 || nodeIdx >= (int)nodes.size()) return;

    const QuadTreeNode2D& node = nodes[nodeIdx];
    double halfSize = node.size / 2.0;
    Vector2 topLeft = worldToScreen(node.x_center - halfSize, node.y_center + halfSize, viewX, viewY, viewWidth, viewHeight);
    Vector2 bottomRight = worldToScreen(node.x_center + halfSize, node.y_center - halfSize, viewX, viewY, viewWidth, viewHeight);

    Color colors[] = {BLUE, GREEN, YELLOW, ORANGE, RED};
    Color color = colors[std::min(node.depth, 4)];

    DrawRectangleLines(topLeft.x, topLeft.y, bottomRight.x - topLeft.x, bottomRight.y - topLeft.y, Fade(color, 0.6f));

    if (node.mass > 0) {
        Vector2 cmPos = worldToScreen(node.cm_x, node.cm_y, viewX, viewY, viewWidth, viewHeight);
        DrawCircleV(cmPos, 5.0f, Fade(YELLOW, 0.5f));
        DrawCircleLines(cmPos.x, cmPos.y, 5.0f, YELLOW);
    }

    if (!node.isLeaf) {
        for (int i = 0; i < 4; i++) {
            if (node.children[i] != -1) {
                drawQuadTreeBounds(node.children[i], viewX, viewY, viewWidth, viewHeight);
            }
        }
    }
}

void QuadTreeDemo::renderSpatialView(int x, int y, int width, int height) {
    DrawRectangle(x, y, width, height, Fade(BLACK, 0.8f));
    DrawRectangleLines(x, y, width, height, WHITE);
    DrawText("2D Space (Quadtree Cells)", x + 10, y + 10, 20, WHITE);

    int viewX = x + 20, viewY = y + 50;
    int viewWidth = width - 40, viewHeight = height - 70;

    Vector2 origin = worldToScreen(0, 0, viewX, viewY, viewWidth, viewHeight);
    Vector2 xAxis = worldToScreen(quadTree.getWorldSize()/2, 0, viewX, viewY, viewWidth, viewHeight);
    Vector2 yAxis = worldToScreen(0, quadTree.getWorldSize()/2, viewX, viewY, viewWidth, viewHeight);

    DrawLineV(origin, xAxis, Fade(WHITE, 0.3f));
    DrawLineV(origin, yAxis, Fade(WHITE, 0.3f));
    DrawText("X", xAxis.x + 5, xAxis.y - 10, 12, WHITE);
    DrawText("Y", yAxis.x - 15, yAxis.y - 5, 12, WHITE);

    if (!quadTree.getNodes().empty()) drawQuadTreeBounds(0, viewX, viewY, viewWidth, viewHeight);
    drawParticles(viewX, viewY, viewWidth, viewHeight);

    int legendY = y + height - 100;
    DrawText("Legend:", x + 10, legendY, 16, WHITE);
    DrawText("Blue boxes = Root level", x + 10, legendY + 20, 14, BLUE);
    DrawText("Green boxes = Level 1", x + 10, legendY + 38, 14, GREEN);
    DrawText("Yellow boxes = Level 2", x + 10, legendY + 56, 14, YELLOW);
    DrawCircle(x + width - 50, legendY + 30, 8, Fade(YELLOW, 0.5f));
    DrawText("= Center of Mass", x + width - 180, legendY + 23, 14, YELLOW);
}

void QuadTreeDemo::drawTreeNode(int nodeIdx, float x, float y, float spacing, int viewX, int viewY, int viewWidth, int viewHeight) {
    const auto& nodes = quadTree.getNodes();
    if (nodeIdx < 0 || nodeIdx >= (int)nodes.size()) return;

    const QuadTreeNode2D& node = nodes[nodeIdx];
    if (node.isLeaf && node.particleIndex == -1) return;

    Color nodeColor = (node.isLeaf && node.particleIndex != -1) ? activeParticles[node.particleIndex].color : LIGHTGRAY;
    float nodeWidth = 95, nodeHeight = 60;
    Rectangle nodeRect = {x - nodeWidth/2, y - nodeHeight/2, nodeWidth, nodeHeight};

    DrawRectangleRec(nodeRect, nodeColor);
    DrawRectangleLinesEx(nodeRect, 2, BLACK);

    std::ostringstream oss;
    oss << "N" << node.nodeId;
    DrawText(oss.str().c_str(), x - 18, y - 26, 18, BLACK);

    if (node.mass > 0) {
        oss.str("");
        oss << "M=" << std::fixed << std::setprecision(1) << node.mass;
        DrawText(oss.str().c_str(), x - 26, y - 7, 17, BLACK);

        oss.str("");
        oss << "(" << std::fixed << std::setprecision(0) << node.cm_x << "," << node.cm_y << ")";
        DrawText(oss.str().c_str(), x - 28, y + 11, 16, BLACK);
    }

    if (!node.isLeaf) {
        DrawText("Int", x - 13, y + 25, 15, DARKGRAY);
    }

    if (!node.isLeaf) {
        float childY = y + 130;
        float childSpacing = spacing * 1.4f;
        const char* quadLabels[] = {"NW", "NE", "SW", "SE"};

        bool allChildrenAreLeaves = true;
        int nonEmptyLeafCount = 0;
        for (int i = 0; i < 4; i++) {
            if (node.children[i] != -1) {
                const QuadTreeNode2D& child = nodes[node.children[i]];
                if (!child.isLeaf) allChildrenAreLeaves = false;
                if (child.isLeaf && child.particleIndex != -1) nonEmptyLeafCount++;
            }
        }

        auto drawChildren = [&](float startX, bool useSpread) {
            float totalWidth = spacing * 0.8f;
            float spreadSpacing = (nonEmptyLeafCount > 1) ? totalWidth / (nonEmptyLeafCount - 1) : 0;
            float spreadStartX = x - totalWidth / 2.0f;
            std::vector<float> placedPositions;
            int nonEmptyIndex = 0;

            for (int i = 0; i < 4; i++) {
                if (node.children[i] == -1) continue;
                const QuadTreeNode2D& child = nodes[node.children[i]];
                if (child.isLeaf && child.particleIndex == -1) continue;

                float childX = useSpread && (child.isLeaf && child.particleIndex != -1)
                    ? spreadStartX + nonEmptyIndex++ * spreadSpacing
                    : startX + i * spacing;

                for (float placedX : placedPositions) {
                    float distance = abs(childX - placedX);
                    if (distance < nodeWidth + 5) {
                        childX = (childX < placedX) ? placedX - nodeWidth - 5 : placedX + nodeWidth + 5;
                    }
                }
                placedPositions.push_back(childX);

                DrawLineEx({x, y + nodeHeight/2}, {childX, childY - nodeHeight/2}, 3, DARKGRAY);
                float midX = (x + childX) / 2.0f;
                float midY = (y + nodeHeight/2 + childY - nodeHeight/2) / 2.0f;
                DrawText(quadLabels[i], midX - 14, midY - 10, 16, DARKBLUE);
                drawTreeNode(node.children[i], childX, childY, childSpacing, viewX, viewY, viewWidth, viewHeight);
            }
        };

        if (allChildrenAreLeaves && nonEmptyLeafCount > 0) {
            drawChildren(0, true);
        } else {
            drawChildren(x - spacing * 1.6f, false);
        }
    }
}

void QuadTreeDemo::renderTreeDiagram(int x, int y, int width, int height) {
    DrawRectangle(x, y, width, height, Fade(BLACK, 0.8f));
    DrawRectangleLines(x, y, width, height, WHITE);
    DrawText("Tree Structure (Mass Propagation)", x + 10, y + 10, 20, WHITE);

    if (quadTree.getNodes().empty()) {
        DrawText("No tree yet - press SPACE to add particles", x + width/2 - 200, y + height/2, 16, GRAY);
        return;
    }

    int viewX = x + 20, viewY = y + 50;
    int viewWidth = width - 40, viewHeight = height - 70;
    float rootX = viewX + viewWidth / 2.0f - 50;
    float rootY = viewY + 15;
    float spacing = std::min(130.0f, (float)viewWidth / 9.0f);
    if (spacing < 80) spacing = 80;

    drawTreeNode(0, rootX, rootY, spacing, viewX, viewY, viewWidth, viewHeight);
    DrawText("Format: [ID] | M=mass | (x,y)=COM | P#=particle", x + 10, y + height - 20, 11, LIGHTGRAY);
}

std::string QuadTreeDemo::getCalculationExplanation() {
    if (activeParticles.empty() || quadTree.getNodes().empty()) {
        return "Add particles to see center of mass calculations...";
    }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1);
    const QuadTreeNode2D& root = quadTree.getRoot();

    if (currentParticleIndex == 1) {
        const Particle2DColored& p = activeParticles[0];
        oss << "Single particle: M=" << p.mass << ", CM=(" << p.x << ", " << p.y << ")";
    } else {
        oss << "Center of Mass Formula:  CM_x = (m1*x1 + m2*x2 + ...) / (m1 + m2 + ...)\n\n";
        oss << "Root Node Calculation:\nTotal Mass = ";
        for (int i = 0; i < (int)activeParticles.size(); i++) {
            if (i > 0) oss << " + ";
            oss << activeParticles[i].mass;
        }
        oss << " = " << root.mass << "\n\n";

        oss << "CM_x = (";
        for (int i = 0; i < (int)activeParticles.size(); i++) {
            if (i > 0) oss << " + ";
            oss << activeParticles[i].mass << "*" << activeParticles[i].x;
        }
        oss << ") / " << root.mass << " = " << root.cm_x << "\n\n";

        oss << "CM_y = (";
        for (int i = 0; i < (int)activeParticles.size(); i++) {
            if (i > 0) oss << " + ";
            oss << activeParticles[i].mass << "*" << activeParticles[i].y;
        }
        oss << ") / " << root.mass << " = " << root.cm_y;
    }

    return oss.str();
}

void QuadTreeDemo::renderCalculationPanel(int x, int y, int width, int height) {
    DrawRectangle(x, y, width, height, Fade(BLACK, 0.9f));
    DrawRectangleLines(x, y, width, height, YELLOW);
    DrawText("CENTER OF MASS CALCULATION", x + 10, y + 10, 20, YELLOW);

    std::string explanation = getCalculationExplanation();
    int textY = y + 38;
    std::istringstream stream(explanation);
    std::string line;

    while (std::getline(stream, line)) {
        if (textY + 20 > y + height - 10) break;  // Stop if text would go outside box
        DrawText(line.c_str(), x + 15, textY, 16, WHITE);
        textY += 20;
    }

    if (!activeParticles.empty()) {
        int legendX = x + width - 350;
        int legendY = y + 38;
        DrawText("Particles:", legendX, legendY, 18, LIGHTGRAY);

        for (int i = 0; i < (int)activeParticles.size(); i++) {
            const Particle2DColored& p = activeParticles[i];
            int itemY = legendY + 26 + i * 24;
            DrawCircle(legendX + 10, itemY + 8, 8, p.color);

            std::ostringstream oss;
            oss << std::fixed << std::setprecision(1);
            oss << "P" << (i+1) << ": m=" << p.mass << " (" << p.x << "," << p.y << ")";
            DrawText(oss.str().c_str(), legendX + 26, itemY, 15, WHITE);
        }
    }
}

void QuadTreeDemo::render(int screenWidth, int screenHeight) {
    BeginDrawing();
    ClearBackground(DARKGRAY);

    DrawText("QuadTree Construction Demo - 2D Step-by-Step", 20, 20, 28, WHITE);

    std::ostringstream oss;
    oss << "Particles Added: " << currentParticleIndex << " / " << allParticles.size();
    DrawText(oss.str().c_str(), 20, 60, 20, YELLOW);
    DrawText("SPACE: Add Next Particle | R: Reset | Q: Return to Menu | ESC: Exit", 20, screenHeight - 30, 18, LIGHTGRAY);

    int padding = 15;
    int calcPanelHeight = std::min(280, screenHeight / 4);
    int topSectionHeight = screenHeight - calcPanelHeight - 130;
    int viewWidth = (screenWidth - 3 * padding) / 2;

    if (topSectionHeight < 400) topSectionHeight = 400;
    if (viewWidth < 500) viewWidth = (screenWidth - 40) / 2;

    renderSpatialView(padding, 100, viewWidth, topSectionHeight);
    renderTreeDiagram(viewWidth + 2 * padding, 100, viewWidth, topSectionHeight);
    renderCalculationPanel(padding, 100 + topSectionHeight + padding, screenWidth - 2 * padding, calcPanelHeight);

    EndDrawing();
}
