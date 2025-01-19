#pragma once  // or use header guards

#include <iostream>
#include <glm/glm.hpp>
#include "Particle.h"
#include "Object.h"
#include <glad/glad.h>

class NewCollision {
public:
    static bool isColliding;

    static bool checkTriangleObjectIntersection(
        const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3,
        const Object& object, glm::vec3& intersectionPoint, glm::vec3& normal);

    static void resolveCollision(
        std::vector<Particle>& particles,
        const std::vector<GLuint>& triangleIndices,
        const Object& object,
        float deltaTime,
        std::vector<GLuint>& collidingIndices // Pass by reference);
    );

private:
    static void resolveParticleCollision(
        Particle& particle,
        const glm::vec3& normal,
        float penetrationDepth);
};
