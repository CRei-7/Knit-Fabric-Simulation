#pragma once
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

enum class ObjectType { Cube, Sphere };

class Object {
private:
    std::vector<float> vertices;
    std::vector<unsigned int> indices; // For sphere index buffer
    glm::vec3 center;
    float halfLength; // For object dimensions
    GLuint VAO, VBO, EBO;
    glm::vec3 color;
    ObjectType objectType; // To differentiate between object and sphere

public:
    Object() : VAO(0), VBO(0), EBO(0), color(glm::vec3(1.0f, 0.0f, 1.0f)), objectType(ObjectType::Cube) {}

    ~Object() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        if (EBO) {
            glDeleteBuffers(1, &EBO);
        }
    }

    void SetupCube(float length, const glm::vec3& cubeCenter) {
        objectType = ObjectType::Cube;
        center = cubeCenter;
        halfLength = length / 2.0f;

        vertices = {
            // Front face
            center.x - halfLength, center.y - halfLength, center.z + halfLength,
            center.x + halfLength, center.y - halfLength, center.z + halfLength,
            center.x + halfLength, center.y + halfLength, center.z + halfLength,
            center.x + halfLength, center.y + halfLength, center.z + halfLength,
            center.x - halfLength, center.y + halfLength, center.z + halfLength,
            center.x - halfLength, center.y - halfLength, center.z + halfLength,

            // Back face
            center.x - halfLength, center.y - halfLength, center.z - halfLength,
            center.x + halfLength, center.y - halfLength, center.z - halfLength,
            center.x + halfLength, center.y + halfLength, center.z - halfLength,
            center.x + halfLength, center.y + halfLength, center.z - halfLength,
            center.x - halfLength, center.y + halfLength, center.z - halfLength,
            center.x - halfLength, center.y - halfLength, center.z - halfLength,

            // Left face
            center.x - halfLength, center.y + halfLength, center.z + halfLength,
            center.x - halfLength, center.y + halfLength, center.z - halfLength,
            center.x - halfLength, center.y - halfLength, center.z - halfLength,
            center.x - halfLength, center.y - halfLength, center.z - halfLength,
            center.x - halfLength, center.y - halfLength, center.z + halfLength,
            center.x - halfLength, center.y + halfLength, center.z + halfLength,

            // Right face
            center.x + halfLength, center.y + halfLength, center.z + halfLength,
            center.x + halfLength, center.y + halfLength, center.z - halfLength,
            center.x + halfLength, center.y - halfLength, center.z - halfLength,
            center.x + halfLength, center.y - halfLength, center.z - halfLength,
            center.x + halfLength, center.y - halfLength, center.z + halfLength,
            center.x + halfLength, center.y + halfLength, center.z + halfLength,

            // Bottom face
            center.x - halfLength, center.y - halfLength, center.z - halfLength,
            center.x + halfLength, center.y - halfLength, center.z - halfLength,
            center.x + halfLength, center.y - halfLength, center.z + halfLength,
            center.x + halfLength, center.y - halfLength, center.z + halfLength,
            center.x - halfLength, center.y - halfLength, center.z + halfLength,
            center.x - halfLength, center.y - halfLength, center.z - halfLength,

            // Top face
            center.x - halfLength, center.y + halfLength, center.z - halfLength,
            center.x + halfLength, center.y + halfLength, center.z - halfLength,
            center.x + halfLength, center.y + halfLength, center.z + halfLength,
            center.x + halfLength, center.y + halfLength, center.z + halfLength,
            center.x - halfLength, center.y + halfLength, center.z + halfLength,
            center.x - halfLength, center.y + halfLength, center.z - halfLength
        };

        // Generate VAO, VBO
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void SetupSphere(float radius, const glm::vec3& sphereCenter, int sectorCount = 36, int stackCount = 18) {
        objectType = ObjectType::Sphere;
        center = sphereCenter;
        halfLength = radius;
        vertices.clear();
        indices.clear();

        float x, y, z, xy;                              // Vertex position
        float sectorStep = 2 * glm::pi<float>() / sectorCount;
        float stackStep = glm::pi<float>() / stackCount;
        float sectorAngle, stackAngle;

        for (int i = 0; i <= stackCount; ++i) {
            stackAngle = glm::pi<float>() / 2 - i * stackStep; // from pi/2 to -pi/2
            xy = radius * cosf(stackAngle);             // r * cos(u)
            z = radius * sinf(stackAngle);              // r * sin(u)

            for (int j = 0; j <= sectorCount; ++j) {
                sectorAngle = j * sectorStep;           // from 0 to 2pi
                x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
                y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
                vertices.push_back(x + center.x);
                vertices.push_back(y + center.y);
                vertices.push_back(z + center.z);
            }
        }

        int k1, k2;
        for (int i = 0; i < stackCount; ++i) {
            k1 = i * (sectorCount + 1);     // beginning of current stack
            k2 = k1 + sectorCount + 1;      // beginning of next stack

            for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
                if (i != 0) {
                    indices.push_back(k1);
                    indices.push_back(k2);
                    indices.push_back(k1 + 1);
                }

                if (i != (stackCount - 1)) {
                    indices.push_back(k1 + 1);
                    indices.push_back(k2);
                    indices.push_back(k2 + 1);
                }
            }
        }

        // Generate VAO, VBO, and EBO (element buffer for indices)
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        // Vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void render(GLuint shaderProgram, const glm::mat4& view, const glm::mat4& projection) {
        glm::mat4 model = glm::mat4(1.0f);

        glUseProgram(shaderProgram);

        GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        GLint colorLoc = glGetUniformLocation(shaderProgram, "Color");
        glUniform3fv(colorLoc, 1, &color[0]);

        glBindVertexArray(VAO);
        if (objectType == ObjectType::Cube) {
            glDrawArrays(GL_TRIANGLES, 0, 36); // Cube uses arrays, not indices
        }
        else {
            glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0); // Sphere uses indices
        }
        glBindVertexArray(0);

        glUseProgram(0);
    }

    bool checkCollision(const glm::vec3& point) const {
        if (objectType == ObjectType::Cube) {
            return (point.x >= center.x - halfLength && point.x <= center.x + halfLength &&
                point.y >= center.y - halfLength && point.y <= center.y + halfLength &&
                point.z >= center.z - halfLength && point.z <= center.z + halfLength);
        }
        else {
            // For a sphere, we check if the point is within the radius
            float distanceSquared = glm::dot(point - center, point - center);
            return distanceSquared <= (halfLength * halfLength); // For spheres, halfLength is the radius
        }
    }

    bool isCube() const {
        if (objectType == ObjectType::Cube)
            return true;
        return false;
    }

    bool isSphere() const {
        if (objectType == ObjectType::Sphere)
            return true;
        return false;
    }


    glm::vec3 getCenter() const {
        return center;
    }

    float getHalfLength() const {
        return halfLength; // For object, it's half the side length, for sphere, it's the radius
    }

    void setColor(const glm::vec3& newColor) {
        color = newColor;
    }
};
