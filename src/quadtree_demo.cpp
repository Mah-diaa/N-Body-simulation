#include "quadtree_demo.h"
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>

QuadTreeDemo::QuadTreeDemo() {
    worldSize = 100.0;
    worldCenterX = 0.0;
    worldCenterY = 0.0;
    currentParticleIndex = 0;
    nextNodeId = 0;

    // Carefully chosen particles for optimal educational demonstration
    // Positioned to create a clear tree structure showing all 4 quadrants

    // Particle 1: NE quadrant (top-right) - Red
    allParticles.push_back(Particle2D(20, 15, 3.0, RED));

    // Particle 2: NW quadrant (top-left) - Blue
    allParticles.push_back(Particle2D(-18, 20, 2.0, BLUE));

    // Particle 3: SW quadrant (bottom-left) - Green
    allParticles.push_back(Particle2D(-15, -18, 2.5, GREEN));

    // Particle 4: SE quadrant (bottom-right) - Yellow
    allParticles.push_back(Particle2D(18, -20, 1.5, YELLOW));

    // Particle 5: NE quadrant again (will cause subdivision) - Orange
    allParticles.push_back(Particle2D(10, 25, 2.0, ORANGE));

    // Particle 6: SE quadrant again (will cause subdivision) - Pink
    allParticles.push_back(Particle2D(25, -10, 1.0, PINK));

    reset();
}

QuadTreeDemo::~QuadTreeDemo() {
}

void QuadTreeDemo::reset() {
    currentParticleIndex = 0;
    nextNodeId = 0;
    activeParticles.clear();
    nodes.clear();

    // Create root node
    createNode(worldCenterX, worldCenterY, worldSize, 0);
}

void QuadTreeDemo::addNextParticle() {
    if (currentParticleIndex < (int)allParticles.size()) {
        currentParticleIndex++;
        buildTree();  // Rebuild entire tree with all active particles
    }
}

void QuadTreeDemo::update() {
    // Nothing to update in this demo
}

int QuadTreeDemo::createNode(double cx, double cy, double size, int depth) {
    QuadTreeNode node;
    node.x_center = cx;
    node.y_center = cy;
    node.size = size;
    node.depth = depth;
    node.nodeId = nextNodeId++;
    node.isLeaf = true;
    node.particleIndex = -1;
    node.mass = 0;
    node.cm_x = cx;
    node.cm_y = cy;
    for (int i = 0; i < 4; i++) {
        node.children[i] = -1;
    }

    nodes.push_back(node);
    return nodes.size() - 1;
}

int QuadTreeDemo::getQuadrant(double px, double py, double cx, double cy) {
    // 0 = NW (x < cx, y >= cy)
    // 1 = NE (x >= cx, y >= cy)
    // 2 = SW (x < cx, y < cy)
    // 3 = SE (x >= cx, y < cy)
    if (px < cx) {
        return (py >= cy) ? 0 : 2;
    } else {
        return (py >= cy) ? 1 : 3;
    }
}

void QuadTreeDemo::buildTree() {
    // Rebuild tree from scratch with current particles
    nodes.clear();
    nextNodeId = 0;
    activeParticles.clear();

    // Add particles up to current index
    for (int i = 0; i < currentParticleIndex; i++) {
        activeParticles.push_back(allParticles[i]);
    }

    if (activeParticles.empty()) {
        createNode(worldCenterX, worldCenterY, worldSize, 0);
        return;
    }

    // Create root
    createNode(worldCenterX, worldCenterY, worldSize, 0);

    // Insert each particle
    for (int i = 0; i < (int)activeParticles.size(); i++) {
        insertParticle(0, i);
    }

    // Update center of mass for all nodes
    updateCenterOfMass(0);
}

void QuadTreeDemo::insertParticle(int nodeIdx, int particleIdx) {
    QuadTreeNode& node = nodes[nodeIdx];

    // If this is an empty leaf, place particle here
    if (node.isLeaf && node.particleIndex == -1) {
        node.particleIndex = particleIdx;
        return;
    }

    // If this leaf already has a particle, we need to subdivide
    if (node.isLeaf && node.particleIndex != -1) {
        int existingParticle = node.particleIndex;
        node.particleIndex = -1;
        node.isLeaf = false;

        // Create 4 children
        double childSize = node.size / 2.0;
        double offset = childSize / 2.0;

        // NW (0)
        node.children[0] = createNode(node.x_center - offset, node.y_center + offset, childSize, node.depth + 1);
        // NE (1)
        node.children[1] = createNode(node.x_center + offset, node.y_center + offset, childSize, node.depth + 1);
        // SW (2)
        node.children[2] = createNode(node.x_center - offset, node.y_center - offset, childSize, node.depth + 1);
        // SE (3)
        node.children[3] = createNode(node.x_center + offset, node.y_center - offset, childSize, node.depth + 1);

        // Reinsert existing particle
        int quad = getQuadrant(activeParticles[existingParticle].x,
                               activeParticles[existingParticle].y,
                               node.x_center, node.y_center);
        insertParticle(node.children[quad], existingParticle);
    }

    // Insert new particle into appropriate child
    const Particle2D& p = activeParticles[particleIdx];
    int quad = getQuadrant(p.x, p.y, node.x_center, node.y_center);
    insertParticle(node.children[quad], particleIdx);
}

void QuadTreeDemo::updateCenterOfMass(int nodeIdx) {
    if (nodeIdx < 0 || nodeIdx >= (int)nodes.size()) return;

    QuadTreeNode& node = nodes[nodeIdx];

    // If leaf with particle, set center of mass to particle
    if (node.isLeaf && node.particleIndex != -1) {
        const Particle2D& p = activeParticles[node.particleIndex];
        node.mass = p.mass;
        node.cm_x = p.x;
        node.cm_y = p.y;
        return;
    }

    // If internal node, aggregate from children
    double totalMass = 0;
    double cmX = 0;
    double cmY = 0;

    for (int i = 0; i < 4; i++) {
        if (node.children[i] != -1) {
            updateCenterOfMass(node.children[i]);
            QuadTreeNode& child = nodes[node.children[i]];
            totalMass += child.mass;
            cmX += child.cm_x * child.mass;
            cmY += child.cm_y * child.mass;
        }
    }

    if (totalMass > 0) {
        node.mass = totalMass;
        node.cm_x = cmX / totalMass;
        node.cm_y = cmY / totalMass;
    }
}

Vector2 QuadTreeDemo::worldToScreen(double wx, double wy, int viewX, int viewY, int viewWidth, int viewHeight) {
    // Convert world coordinates [-worldSize/2, worldSize/2] to screen coordinates
    double scale = viewHeight / worldSize;  // Use height for square aspect
    int centerX = viewX + viewWidth / 2;
    int centerY = viewY + viewHeight / 2;

    float sx = centerX + (wx - worldCenterX) * scale;
    float sy = centerY - (wy - worldCenterY) * scale;  // Y is inverted in screen coords

    return {sx, sy};
}

void QuadTreeDemo::drawParticles(int viewX, int viewY, int viewWidth, int viewHeight) {
    // Draw all active particles
    for (int i = 0; i < (int)activeParticles.size(); i++) {
        const Particle2D& p = activeParticles[i];
        Vector2 screenPos = worldToScreen(p.x, p.y, viewX, viewY, viewWidth, viewHeight);

        // Particle circle with glow effect
        float radius = 10.0f + p.mass * 2.5f;
        DrawCircleV(screenPos, radius + 3, Fade(p.color, 0.3f));  // Glow
        DrawCircleV(screenPos, radius, p.color);
        DrawCircleLines(screenPos.x, screenPos.y, radius, WHITE);

        // Particle number label (above)
        std::ostringstream numLabel;
        numLabel << "P" << (i + 1);
        int labelWidth = MeasureText(numLabel.str().c_str(), 14);
        DrawText(numLabel.str().c_str(), screenPos.x - labelWidth/2, screenPos.y - radius - 25, 14, YELLOW);

        // Mass label (below)
        std::ostringstream massLabel;
        massLabel << "m=" << std::fixed << std::setprecision(1) << p.mass;
        int massWidth = MeasureText(massLabel.str().c_str(), 12);
        DrawText(massLabel.str().c_str(), screenPos.x - massWidth/2, screenPos.y + radius + 8, 12, WHITE);
    }

    // Draw the particle that's about to be added (if any)
    if (currentParticleIndex < (int)allParticles.size()) {
        const Particle2D& nextP = allParticles[currentParticleIndex];
        Vector2 screenPos = worldToScreen(nextP.x, nextP.y, viewX, viewY, viewWidth, viewHeight);

        // Pulsing effect
        float pulse = (sin(GetTime() * 5.0f) + 1.0f) / 2.0f;
        float radius = 10.0f + nextP.mass * 2.5f;
        DrawCircleV(screenPos, radius + pulse * 8.0f, Fade(nextP.color, 0.2f));
        DrawCircleV(screenPos, radius, Fade(nextP.color, 0.5f));

        // "NEXT" label
        const char* nextLabel = "NEXT";
        int nextWidth = MeasureText(nextLabel, 16);
        DrawRectangle(screenPos.x - nextWidth/2 - 5, screenPos.y + radius + 10, nextWidth + 10, 22, Fade(YELLOW, 0.8f));
        DrawText(nextLabel, screenPos.x - nextWidth/2, screenPos.y + radius + 12, 16, BLACK);

        // Particle info
        std::ostringstream nextInfo;
        nextInfo << "P" << (currentParticleIndex + 1) << " (m=" << std::fixed << std::setprecision(1) << nextP.mass << ")";
        int infoWidth = MeasureText(nextInfo.str().c_str(), 12);
        DrawText(nextInfo.str().c_str(), screenPos.x - infoWidth/2, screenPos.y - radius - 20, 12, YELLOW);
    }
}

void QuadTreeDemo::drawQuadTreeBounds(int nodeIdx, int viewX, int viewY, int viewWidth, int viewHeight) {
    if (nodeIdx < 0 || nodeIdx >= (int)nodes.size()) return;

    const QuadTreeNode& node = nodes[nodeIdx];

    // Calculate corners
    double halfSize = node.size / 2.0;
    Vector2 topLeft = worldToScreen(node.x_center - halfSize, node.y_center + halfSize, viewX, viewY, viewWidth, viewHeight);
    Vector2 bottomRight = worldToScreen(node.x_center + halfSize, node.y_center - halfSize, viewX, viewY, viewWidth, viewHeight);

    // Color based on depth
    Color color;
    switch (node.depth) {
        case 0: color = BLUE; break;
        case 1: color = GREEN; break;
        case 2: color = YELLOW; break;
        case 3: color = ORANGE; break;
        default: color = RED; break;
    }

    // Draw bounding box
    float width = bottomRight.x - topLeft.x;
    float height = bottomRight.y - topLeft.y;
    DrawRectangleLines(topLeft.x, topLeft.y, width, height, Fade(color, 0.6f));

    // Draw center of mass if node has mass
    if (node.mass > 0) {
        Vector2 cmPos = worldToScreen(node.cm_x, node.cm_y, viewX, viewY, viewWidth, viewHeight);
        DrawCircleV(cmPos, 5.0f, Fade(YELLOW, 0.5f));
        DrawCircleLines(cmPos.x, cmPos.y, 5.0f, YELLOW);
    }

    // Recursively draw children
    if (!node.isLeaf) {
        for (int i = 0; i < 4; i++) {
            if (node.children[i] != -1) {
                drawQuadTreeBounds(node.children[i], viewX, viewY, viewWidth, viewHeight);
            }
        }
    }
}

void QuadTreeDemo::renderSpatialView(int x, int y, int width, int height) {
    // Draw background
    DrawRectangle(x, y, width, height, Fade(BLACK, 0.8f));
    DrawRectangleLines(x, y, width, height, WHITE);

    // Draw title
    DrawText("2D Space (Quadtree Cells)", x + 10, y + 10, 20, WHITE);

    // Draw axes
    int viewX = x + 20;
    int viewY = y + 50;
    int viewWidth = width - 40;
    int viewHeight = height - 70;

    Vector2 originScreen = worldToScreen(0, 0, viewX, viewY, viewWidth, viewHeight);
    Vector2 xAxisEnd = worldToScreen(worldSize/2, 0, viewX, viewY, viewWidth, viewHeight);
    Vector2 yAxisEnd = worldToScreen(0, worldSize/2, viewX, viewY, viewWidth, viewHeight);

    DrawLineV(originScreen, xAxisEnd, Fade(WHITE, 0.3f));
    DrawLineV(originScreen, yAxisEnd, Fade(WHITE, 0.3f));
    DrawText("X", xAxisEnd.x + 5, xAxisEnd.y - 10, 12, WHITE);
    DrawText("Y", yAxisEnd.x - 15, yAxisEnd.y - 5, 12, WHITE);

    // Draw quadtree bounds
    if (!nodes.empty()) {
        drawQuadTreeBounds(0, viewX, viewY, viewWidth, viewHeight);
    }

    // Draw particles
    drawParticles(viewX, viewY, viewWidth, viewHeight);

    // Draw legend
    int legendY = y + height - 100;
    DrawText("Legend:", x + 10, legendY, 16, WHITE);
    DrawText("Blue boxes = Root level", x + 10, legendY + 20, 14, BLUE);
    DrawText("Green boxes = Level 1", x + 10, legendY + 38, 14, GREEN);
    DrawText("Yellow boxes = Level 2", x + 10, legendY + 56, 14, YELLOW);
    DrawCircle(x + width - 50, legendY + 30, 8, Fade(YELLOW, 0.5f));
    DrawText("= Center of Mass", x + width - 180, legendY + 23, 14, YELLOW);
}

void QuadTreeDemo::calculateTreeLayout(std::vector<TreeNodeLayout>& layout, int nodeIdx, float x, float y, float hSpacing, int depth) {
    if (nodeIdx < 0 || nodeIdx >= (int)nodes.size()) return;

    layout[nodeIdx].x = x;
    layout[nodeIdx].y = y;
    layout[nodeIdx].depth = depth;

    const QuadTreeNode& node = nodes[nodeIdx];
    if (!node.isLeaf) {
        float childSpacing = hSpacing / 4.0f;
        float startX = x - hSpacing * 1.5f;

        for (int i = 0; i < 4; i++) {
            if (node.children[i] != -1) {
                calculateTreeLayout(layout, node.children[i], startX + i * hSpacing, y + 80, childSpacing, depth + 1);
            }
        }
    }
}

void QuadTreeDemo::drawTreeNode(int nodeIdx, float x, float y, float spacing, int viewX, int viewY, int viewWidth, int viewHeight) {
    if (nodeIdx < 0 || nodeIdx >= (int)nodes.size()) return;

    const QuadTreeNode& node = nodes[nodeIdx];

    // Check if this is an empty node (leaf without particle)
    bool isEmpty = (node.isLeaf && node.particleIndex == -1);

    if (isEmpty) {
        // Don't draw anything for empty nodes - makes overlaps apparent
        return;  // Skip drawing
    }

    // Node appearance (for non-empty nodes)
    Color nodeColor;
    if (node.isLeaf && node.particleIndex != -1) {
        nodeColor = activeParticles[node.particleIndex].color;
    } else {
        nodeColor = LIGHTGRAY;
    }

    // Draw node box - SMALLER for less clutter
    float nodeWidth = 95;
    float nodeHeight = 60;
    Rectangle nodeRect = {x - nodeWidth/2, y - nodeHeight/2, nodeWidth, nodeHeight};

    DrawRectangleRec(nodeRect, nodeColor);
    DrawRectangleLinesEx(nodeRect, 2, BLACK);

    // Node info text - LARGER, MORE READABLE fonts
    std::ostringstream oss;
    oss << "N" << node.nodeId;  // Shorter label
    DrawText(oss.str().c_str(), x - 18, y - 26, 18, BLACK);  // LARGER node ID

    if (node.mass > 0) {
        oss.str("");
        oss << "M=" << std::fixed << std::setprecision(1) << node.mass;
        DrawText(oss.str().c_str(), x - 26, y - 7, 17, BLACK);  // LARGER mass

        oss.str("");
        oss << "(" << std::fixed << std::setprecision(0) << node.cm_x << "," << node.cm_y << ")";
        DrawText(oss.str().c_str(), x - 28, y + 11, 16, BLACK);  // LARGER coordinates
    }

    // Only show "Int" for internal nodes (color coding shows which particle)
    if (!node.isLeaf) {
        DrawText("Int", x - 13, y + 25, 15, DARKGRAY);  // LARGER "Int" label
    }

    // Draw connections to children
    if (!node.isLeaf) {
        float childY = y + 130;
        float childSpacing = spacing * 1.4f;  // INCREASE spacing significantly as we go deeper

        const char* quadLabels[] = {"NW", "NE", "SW", "SE"};

        // Check if children are all leaves (last level)
        bool allChildrenAreLeaves = true;
        int nonEmptyLeafCount = 0;
        for (int i = 0; i < 4; i++) {
            if (node.children[i] != -1) {
                const QuadTreeNode& child = nodes[node.children[i]];
                if (!child.isLeaf) {
                    allChildrenAreLeaves = false;
                    break;
                }
                // Count non-empty leaf children
                if (child.isLeaf && child.particleIndex != -1) {
                    nonEmptyLeafCount++;
                }
            }
        }

        // For LAST LEVEL: spread non-empty nodes with collision detection
        if (allChildrenAreLeaves && nonEmptyLeafCount > 0) {
            float totalWidth = spacing * 0.8f;  // Tight spacing for children of same parent
            float spreadSpacing = (nonEmptyLeafCount > 1) ? totalWidth / (nonEmptyLeafCount - 1) : 0;
            float spreadStartX = x - totalWidth / 2.0f;

            std::vector<float> placedPositions;  // Track placed node positions
            int nonEmptyIndex = 0;

            for (int i = 0; i < 4; i++) {
                if (node.children[i] != -1) {
                    const QuadTreeNode& child = nodes[node.children[i]];

                    // Skip drawing edge if child is empty
                    if (child.isLeaf && child.particleIndex == -1) {
                        continue;
                    }

                    float childX;
                    if (child.isLeaf && child.particleIndex != -1) {
                        // Non-empty leaf - spread it out
                        childX = spreadStartX + nonEmptyIndex * spreadSpacing;
                        nonEmptyIndex++;

                        // Collision detection - snap to edge if overlapping
                        for (float placedX : placedPositions) {
                            float distance = abs(childX - placedX);
                            if (distance < nodeWidth + 5) {  // 5px margin
                                // Overlapping! Snap to edge
                                if (childX < placedX) {
                                    childX = placedX - nodeWidth - 5;
                                } else {
                                    childX = placedX + nodeWidth + 5;
                                }
                            }
                        }
                        placedPositions.push_back(childX);
                    } else {
                        // Internal node
                        childX = x - spacing * 1.6f + i * spacing;
                    }

                    DrawLineEx({x, y + nodeHeight/2}, {childX, childY - nodeHeight/2}, 3, DARKGRAY);

                    float midX = (x + childX) / 2.0f;
                    float midY = (y + nodeHeight/2 + childY - nodeHeight/2) / 2.0f;
                    DrawText(quadLabels[i], midX - 14, midY - 10, 16, DARKBLUE);  // LARGER labels

                    drawTreeNode(node.children[i], childX, childY, childSpacing, viewX, viewY, viewWidth, viewHeight);
                }
            }
        } else {
            // Normal layout for non-leaf levels with collision detection
            float startX = x - spacing * 1.6f;
            std::vector<float> placedPositions;  // Track placed node positions

            for (int i = 0; i < 4; i++) {
                if (node.children[i] != -1) {
                    const QuadTreeNode& child = nodes[node.children[i]];

                    // Skip drawing edge if child is empty
                    if (child.isLeaf && child.particleIndex == -1) {
                        continue;
                    }

                    float childX = startX + i * spacing;

                    // Collision detection - snap to edge if overlapping
                    for (float placedX : placedPositions) {
                        float distance = abs(childX - placedX);
                        if (distance < nodeWidth + 5) {  // 5px margin
                            // Overlapping! Snap to edge
                            if (childX < placedX) {
                                childX = placedX - nodeWidth - 5;
                            } else {
                                childX = placedX + nodeWidth + 5;
                            }
                        }
                    }
                    placedPositions.push_back(childX);

                    DrawLineEx({x, y + nodeHeight/2}, {childX, childY - nodeHeight/2}, 3, DARKGRAY);

                    float midX = (x + childX) / 2.0f;
                    float midY = (y + nodeHeight/2 + childY - nodeHeight/2) / 2.0f;
                    DrawText(quadLabels[i], midX - 14, midY - 10, 16, DARKBLUE);  // LARGER labels

                    drawTreeNode(node.children[i], childX, childY, childSpacing, viewX, viewY, viewWidth, viewHeight);
                }
            }
        }
    }
}

void QuadTreeDemo::renderTreeDiagram(int x, int y, int width, int height) {
    // Draw background
    DrawRectangle(x, y, width, height, Fade(BLACK, 0.8f));
    DrawRectangleLines(x, y, width, height, WHITE);

    // Draw title
    DrawText("Tree Structure (Mass Propagation)", x + 10, y + 10, 20, WHITE);

    if (nodes.empty()) {
        DrawText("No tree yet - press SPACE to add particles", x + width/2 - 200, y + height/2, 16, GRAY);
        return;
    }

    // Draw tree starting from root
    int viewX = x + 20;
    int viewY = y + 50;  // Start higher for deeper tree
    int viewWidth = width - 40;
    int viewHeight = height - 70;  // Less padding at bottom

    float rootX = viewX + viewWidth / 2.0f - 50;  // Shift left by 50 pixels
    float rootY = viewY + 15;  // Start very close to top for deep tree

    // Scale spacing to FIT within bounds - prevent clipping
    // Spacing increases at each level (×1.4), so we need room for expansion
    float spacing = std::min(130.0f, (float)viewWidth / 9.0f);
    if (spacing < 80) spacing = 80;  // Better minimum for readability

    drawTreeNode(0, rootX, rootY, spacing, viewX, viewY, viewWidth, viewHeight);

    // Instructions - COMPACT
    DrawText("Format: [ID] | M=mass | (x,y)=COM | P#=particle", x + 10, y + height - 20, 11, LIGHTGRAY);
}

std::string QuadTreeDemo::getCalculationExplanation() {
    if (activeParticles.empty() || nodes.empty()) {
        return "Add particles to see center of mass calculations...";
    }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1);

    // Show calculation for root node
    const QuadTreeNode& root = nodes[0];

    if (currentParticleIndex == 1) {
        // Just one particle
        const Particle2D& p = activeParticles[0];
        oss << "Single particle: M=" << p.mass << ", CM=(" << p.x << ", " << p.y << ")";
    } else if (currentParticleIndex >= 2) {
        // Show aggregation formula
        oss << "Center of Mass Formula:  CM_x = (m1*x1 + m2*x2 + ...) / (m1 + m2 + ...)\n\n";

        oss << "Root Node Calculation:\n";
        oss << "Total Mass = ";
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
    // Draw background
    DrawRectangle(x, y, width, height, Fade(BLACK, 0.9f));
    DrawRectangleLines(x, y, width, height, YELLOW);

    // Title - LARGER
    DrawText("CENTER OF MASS CALCULATION", x + 10, y + 10, 20, YELLOW);

    // Draw formula and explanation
    std::string explanation = getCalculationExplanation();

    // Draw multiline text - LARGER
    int textY = y + 38;
    int lineHeight = 20;  // More line spacing for larger font
    std::istringstream stream(explanation);
    std::string line;

    while (std::getline(stream, line)) {
        DrawText(line.c_str(), x + 15, textY, 16, WHITE);  // LARGER font
        textY += lineHeight;
    }

    // Add legend showing particle colors and values - MUCH LARGER
    if (!activeParticles.empty()) {
        int legendX = x + width - 350;
        int legendY = y + 38;
        DrawText("Particles:", legendX, legendY, 18, LIGHTGRAY);  // LARGER title

        for (int i = 0; i < (int)activeParticles.size(); i++) {
            const Particle2D& p = activeParticles[i];
            int itemY = legendY + 26 + i * 24;  // More spacing for larger text

            DrawCircle(legendX + 10, itemY + 8, 8, p.color);  // Bigger circles

            std::ostringstream oss;
            oss << std::fixed << std::setprecision(1);
            oss << "P" << (i+1) << ": m=" << p.mass << " (" << p.x << "," << p.y << ")";
            DrawText(oss.str().c_str(), legendX + 26, itemY, 15, WHITE);  // MUCH LARGER font
        }
    }
}

void QuadTreeDemo::render(int screenWidth, int screenHeight) {
    BeginDrawing();
    ClearBackground(DARKGRAY);

    // Title
    DrawText("QuadTree Construction Demo - 2D Step-by-Step", 20, 20, 28, WHITE);

    // Progress info
    std::ostringstream oss;
    oss << "Particles Added: " << currentParticleIndex << " / " << allParticles.size();
    DrawText(oss.str().c_str(), 20, 60, 20, YELLOW);

    // Instructions
    DrawText("SPACE: Add Next Particle | R: Reset | Q: Return to Menu | ESC: Exit", 20, screenHeight - 30, 18, LIGHTGRAY);

    // Layout: Top section split (spatial + tree), bottom section (calculations)
    // Make layout responsive to screen size
    int padding = 15;
    int calcPanelHeight = std::min(180, screenHeight / 5);  // LARGER calc panel - text was clipping
    int topSectionHeight = screenHeight - calcPanelHeight - 130;

    int viewWidth = (screenWidth - 3 * padding) / 2;
    int viewY = 100;

    // Ensure minimum sizes
    if (topSectionHeight < 400) topSectionHeight = 400;
    if (viewWidth < 500) viewWidth = (screenWidth - 40) / 2;

    // Top panels
    renderSpatialView(padding, viewY, viewWidth, topSectionHeight);
    renderTreeDiagram(viewWidth + 2 * padding, viewY, viewWidth, topSectionHeight);

    // Bottom calculation panel
    renderCalculationPanel(padding, viewY + topSectionHeight + padding, screenWidth - 2 * padding, calcPanelHeight);

    EndDrawing();
}

