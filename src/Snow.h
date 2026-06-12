#ifndef SNOW_H
#define SNOW_H

#include "objload.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

struct Snowflake {
  glm::vec3 position;
  glm::vec3 velocity;
  float life;
  float size;
};

struct GridCell {
  float height = -9999.0f;
  glm::vec3 normal = glm::vec3(0.0f, 1.0f, 0.0f);
};

class SnowSimulation {
public:
  SnowSimulation(int numParticles);
  ~SnowSimulation();

  // 1. Generate 2D Height Grid from obj::Model
  void initGridFromModel(const obj::Model &model, const glm::mat4 &modelMatrix);

  // 2. Initialize OpenGL buffers (GL_DYNAMIC_DRAW)
  void initRendering(GLuint shaderProgram);

  // 3. Update Physics (Gravity, Wind, Collision, Sliding)
  void update(float dt, float time);

  // 4. Update VBO and Render
  void render(const glm::mat4 &view, const glm::mat4 &projection);

private:
  std::vector<Snowflake> particles;
  std::vector<glm::vec3> renderData;

public:
  // Grid details
  int gridWidth = 200;
  int gridHeight = 200;
  float minX, minZ, maxX, maxZ;
  float cellSizeX, cellSizeZ;
  std::vector<GridCell> heightGrid;

  GLuint vao, vbo;
  GLuint shader;

public:
  int activeParticleCount = 50000;

private:
  // Physics parameters
  float gravity = 0.8f;
  float windStrength = 0.8f;
  float slidingDownwardForce = 3.0f;

  // Helpers
  void respawn(Snowflake &p);
  GridCell getMountainData(float x, float z);
  bool barycentric(glm::vec2 p, glm::vec2 a, glm::vec2 b, glm::vec2 c, float &u,
                   float &v, float &w);
};

#endif // SNOW_H
