#ifndef PATHFINDER_H
#define PATHFINDER_H

#include "Snow.h"
#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>

class Pathfinder {
public:
    Pathfinder(SnowSimulation* sim);
    ~Pathfinder();

    void initRendering(GLuint shaderProgram);
    
    // Generates a path from one of the 4 corners (0=NW, 1=NE, 2=SW, 3=SE) to the summit.
    // Takes snowLevel to penalize snowy cells.
    void generatePath(int cornerIdx, float snowLevel);
    
    void clearPaths();
    
    void render(const glm::mat4& view, const glm::mat4& projection);

private:
    struct PathData {
        std::vector<glm::vec3> vertices;
        glm::vec4 color;
        int vertexOffset;
        int vertexCount;
    };

    SnowSimulation* sim;
    GLuint vao, vbo;
    std::vector<PathData> paths;
    GLuint shader;
    
    int findSummitIndex();
    std::vector<glm::vec3> findPathAStar(int startIdx, int targetIdx, float snowLevel);
    int getValidCorner(int cornerIdx);
    
    void updateVBO();
};

#endif
