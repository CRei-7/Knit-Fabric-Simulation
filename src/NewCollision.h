#pragma once

#include <iostream>
#include <glm/glm.hpp>
#include "Particle.h"
#include "Object.h"
#include "BVH.h"
#include <glad/glad.h>
#include <algorithm>

class NewCollision {
public:
    static bool isColliding;
    static int bvhCollisionChecks;
    static int collisionChecks;
    static float offset;
    static bool checkTriangleObjectIntersection(
        const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3,
        const Object& object, glm::vec3& intersectionPoint, glm::vec3& normal);
    static void resolveCollision(
        std::vector<Particle>& particles,
        BVH* clothBVH,
        const std::vector<GLuint>& triangleIndices,
        const Object& object,
        float deltaTime,
        std::vector<GLuint>& collidingIndices, // Pass by reference);
        float StaticFriction, float KineticFriction
    );
    static void resolveCollisionWithOutBVH(
        std::vector<Particle>& particles,
        const std::vector<GLuint>& triangleIndices,
        const Object& object,
        float deltaTime,
        std::vector<GLuint>& collidingIndices, // Pass by reference);
        float StaticFriction, float KineticFriction
    );
      static void traverseBVH(
          BVHNode* node,
          const Object& object,
          std::vector<GLuint>& potentialTriangles);
    // static void initializeBVH(const std::vector<Particle>& particles, const std::vector<GLuint>& triangleIndices);
    // static void updateBVH(const std::vector<Particle>& particles);

    // static void resolveCollisionForTriangle(
    //     Particle& p1, Particle& p2, Particle& p3,
    //     const glm::vec3& normal, const glm::vec3& intersectionPoint);

private:
    static void resolveParticleCollision(
        Particle& particle,
        const glm::vec3& normal,
        float penetrationDepth,
        float deltaTime, float Fs, float Fk);

    // static void traverseBVHForCollisions(
    //        BVHNode* node,
    //        const std::vector<Particle>& particles,
    //        const Object& object,
    //        std::vector<GLuint>& collidingIndices
    //    );
};
