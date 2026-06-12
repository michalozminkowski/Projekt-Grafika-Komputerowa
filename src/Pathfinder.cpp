#include "Pathfinder.h"
#include <iostream>
#include <queue>
#include <cmath>
#include <algorithm>

Pathfinder::Pathfinder(SnowSimulation* sim) : sim(sim), vao(0), vbo(0) {}

Pathfinder::~Pathfinder() {
    if (vao != 0) glDeleteVertexArrays(1, &vao);
    if (vbo != 0) glDeleteBuffers(1, &vbo);
}

void Pathfinder::initRendering(GLuint shaderProgram) {
    shader = shaderProgram;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
}

void Pathfinder::clearPaths() {
    paths.clear();
    updateVBO();
}

int Pathfinder::findSummitIndex() {
    int bestIdx = -1;
    float maxHeight = -9999.0f;
    for (size_t i = 0; i < sim->heightGrid.size(); ++i) {
        if (sim->heightGrid[i].height > maxHeight) {
            maxHeight = sim->heightGrid[i].height;
            bestIdx = i;
        }
    }
    return bestIdx;
}

int Pathfinder::getValidCorner(int cornerIdx) {
    int targetX = 0, targetZ = 0;
    if (cornerIdx == 0) { targetX = 0; targetZ = 0; } // NW
    else if (cornerIdx == 1) { targetX = sim->gridWidth - 1; targetZ = 0; } // NE
    else if (cornerIdx == 2) { targetX = 0; targetZ = sim->gridHeight - 1; } // SW
    else if (cornerIdx == 3) { targetX = sim->gridWidth - 1; targetZ = sim->gridHeight - 1; } // SE

    int bestIdx = -1;
    float minDist = 99999.0f;
    for (int z = 0; z < sim->gridHeight; ++z) {
        for (int x = 0; x < sim->gridWidth; ++x) {
            int idx = z * sim->gridWidth + x;
            if (sim->heightGrid[idx].height > -9000.0f) {
                float dist = std::sqrt(std::pow(x - targetX, 2) + std::pow(z - targetZ, 2));
                if (dist < minDist) {
                    minDist = dist;
                    bestIdx = idx;
                }
            }
        }
    }
    return bestIdx;
}

struct Node {
    int idx;
    float f;
    bool operator>(const Node& other) const { return f > other.f; }
};

std::vector<glm::vec3> Pathfinder::findPathAStar(int startIdx, int targetIdx, float snowLevel) {
    if (startIdx < 0 || targetIdx < 0) return {};

    int w = sim->gridWidth;
    int h = sim->gridHeight;
    int n = w * h;

    std::vector<float> gScore(n, 999999.0f);
    std::vector<int> cameFrom(n, -1);
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> openSet;

    gScore[startIdx] = 0;
    openSet.push({startIdx, 0.0f});

    auto getPos = [&](int idx) -> glm::vec3 {
        int x = idx % w;
        int z = idx / w;
        return glm::vec3(sim->minX + (x + 0.5f) * sim->cellSizeX,
                         sim->heightGrid[idx].height,
                         sim->minZ + (z + 0.5f) * sim->cellSizeZ);
    };

    glm::vec3 targetPos = getPos(targetIdx);

    int dx[8] = {-1, 1, 0, 0, -1, -1, 1, 1};
    int dz[8] = {0, 0, -1, 1, -1, 1, -1, 1};

    while (!openSet.empty()) {
        int current = openSet.top().idx;
        openSet.pop();

        if (current == targetIdx) {
            std::vector<glm::vec3> path;
            int curr = current;
            while (curr != -1) {
                path.push_back(getPos(curr) + glm::vec3(0, 0.005f, 0)); // very slightly above ground
                curr = cameFrom[curr];
            }
            std::reverse(path.begin(), path.end());
            return path;
        }

        int cx = current % w;
        int cz = current / w;
        glm::vec3 currentPos = getPos(current);

        for (int i = 0; i < 8; ++i) {
            int nx = cx + dx[i];
            int nz = cz + dz[i];
            if (nx >= 0 && nx < w && nz >= 0 && nz < h) {
                int neighbor = nz * w + nx;
                if (sim->heightGrid[neighbor].height < -9000.0f) continue;

                glm::vec3 neighborPos = getPos(neighbor);
                float dist = glm::distance(currentPos, neighborPos);
                
                // Naismith's rule (time estimation)
                float timeCost = dist;
                float heightDiff = neighborPos.y - currentPos.y;
                if (heightDiff > 0) {
                    timeCost += heightDiff * 6.0f; // Climbing penalty
                } else {
                    timeCost += std::abs(heightDiff) * 1.5f; // Descending penalty
                }

                // Snow penalty
                float ny = sim->heightGrid[neighbor].normal.y;
                float minSlope = std::max(0.8f - (snowLevel * 0.15f), 0.1f);
                float maxSlope = std::max(0.95f - (snowLevel * 0.1f), 0.2f);
                
                float slopeFactor = 0.0f;
                if (ny <= minSlope) slopeFactor = 0.0f;
                else if (ny >= maxSlope) slopeFactor = 1.0f;
                else {
                    float t = (ny - minSlope) / (maxSlope - minSlope);
                    slopeFactor = t * t * (3.0f - 2.0f * t); // smoothstep
                }
                float snowCoverage = std::max(0.0f, std::min(1.0f, slopeFactor * snowLevel));
                float snowPenalty = snowCoverage * 30.0f * dist; // Huge penalty for deep snow

                float tentative_gScore = gScore[current] + timeCost + snowPenalty;

                if (tentative_gScore < gScore[neighbor]) {
                    cameFrom[neighbor] = current;
                    gScore[neighbor] = tentative_gScore;
                    // Heuristic: Euclidean distance to target (underestimate of timeCost)
                    float hScore = glm::distance(neighborPos, targetPos);
                    openSet.push({neighbor, tentative_gScore + hScore});
                }
            }
        }
    }
    return {};
}

void Pathfinder::generatePath(int cornerIdx, float snowLevel) {
    std::cout << "Generating path from corner " << cornerIdx << " with snowLevel " << snowLevel << std::endl;
    int targetIdx = findSummitIndex();
    if (targetIdx == -1) return;

    int startIdx = getValidCorner(cornerIdx);
    std::vector<glm::vec3> pathVerts = findPathAStar(startIdx, targetIdx, snowLevel);
    
    if (!pathVerts.empty()) {
        glm::vec4 colors[5] = {
            glm::vec4(1.0f, 0.2f, 0.2f, 1.0f), // Red
            glm::vec4(0.2f, 1.0f, 0.2f, 1.0f), // Green
            glm::vec4(0.2f, 0.4f, 1.0f, 1.0f), // Blue
            glm::vec4(1.0f, 1.0f, 0.2f, 1.0f), // Yellow
            glm::vec4(1.0f, 0.2f, 1.0f, 1.0f)  // Purple
        };
        
        PathData newPath;
        newPath.vertices = pathVerts;
        
        if (paths.size() >= 5) {
            paths.erase(paths.begin()); // Remove oldest
        }
        
        newPath.color = colors[paths.size() % 5];
        paths.push_back(newPath);
        
        updateVBO();
        std::cout << "Path generated. Total paths: " << paths.size() << std::endl;
    } else {
        std::cout << "Path generation failed." << std::endl;
    }
}

void Pathfinder::updateVBO() {
    if (paths.empty()) return;
    
    std::vector<glm::vec3> allVertices;
    for (auto& p : paths) {
        p.vertexOffset = allVertices.size();
        p.vertexCount = p.vertices.size();
        allVertices.insert(allVertices.end(), p.vertices.begin(), p.vertices.end());
    }
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, allVertices.size() * sizeof(glm::vec3), allVertices.data(), GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glBindVertexArray(0);
}

void Pathfinder::render(const glm::mat4& view, const glm::mat4& projection) {
    if (paths.empty()) return;

    glUseProgram(shader);
    glm::mat4 model = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, &model[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, &projection[0][0]);

    glBindVertexArray(vao);
    glLineWidth(4.0f); // Make the path thicker
    
    // Draw each path using its assigned color
    for (const auto& p : paths) {
        glUniform4f(glGetUniformLocation(shader, "color"), p.color.r, p.color.g, p.color.b, p.color.a);
        glDrawArrays(GL_LINE_STRIP, p.vertexOffset, p.vertexCount);
    }
    
    glBindVertexArray(0);
    glLineWidth(1.0f);
}
