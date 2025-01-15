// Cloth.h
#ifndef CLOTH_H
#define CLOTH_H

#include <vector>
#include <glm/glm.hpp>
#include "Particle.h"
#include "Spring.h"
#include "Mesh.h"
#include "Shader.h"
#include "Constants.h"

class Cloth {
public:
    Cloth(int columns, int rows, float particleDistance, float springConstant);
    ~Cloth();

    void update(float deltaTime, bool enableWind);
    void render(Shader& shader, const glm::mat4& view, const glm::mat4& projection);
    void reset(bool verticalOrientation);
    // void applyWindForce(const glm::vec3& windDirection, float windScale, float windNoise);

private:
    void setupMesh();
    void updateMesh();
    void calculateNormals();

    std::vector<Particle> particles;
    std::vector<Spring> springs;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    Mesh* mesh;

    int numColumns, numRows;
    float particleDistance;
    float springK;
    bool isVertical;
};

#endif // CLOTH_H
