// face  to face collision detection
#include "NewCollision.h"
#include <iostream>
#include "Particle.h"
#include "Object.h"
#include "BVH.h"

bool NewCollision::isColliding = false;
int NewCollision::bvhCollisionChecks = 0;
int NewCollision::collisionChecks = 0;
float NewCollision::offset = 0.01f;

bool NewCollision::checkTriangleObjectIntersection(
    const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3,
    const Object& object, glm::vec3& intersectionPoint, glm::vec3& normal) {

    // Calculate triangle normal first - this is needed for both cube and sphere
    glm::vec3 edge1 = v2 - v1;
    glm::vec3 edge2 = v3 - v1;
    normal = -1.0f * glm::normalize(glm::cross(edge1, edge2));

    if (object.isCube()) {
        // Get cube bounds
        glm::vec3 center = object.getCenter();
        float halfLength = object.getHalfLength() + offset;
        glm::vec3 cubeMin = center - glm::vec3(halfLength);
        glm::vec3 cubeMax = center + glm::vec3(halfLength);

        // Calculate triangle bounds
        glm::vec3 triangleMin = glm::min(glm::min(v1, v2), v3);
        glm::vec3 triangleMax = glm::max(glm::max(v1, v2), v3);

        // Simple AABB overlap test first
        if (triangleMax.x < cubeMin.x || triangleMin.x > cubeMax.x ||
            triangleMax.y < cubeMin.y || triangleMin.y > cubeMax.y ||
            triangleMax.z < cubeMin.z || triangleMin.z > cubeMax.z) {
            return false;
        }

        // If any vertex is inside the cube, we have collision
        auto isInside = [&](const glm::vec3& p) {
            return p.x >= cubeMin.x && p.x <= cubeMax.x &&
                    p.y >= cubeMin.y && p.y <= cubeMax.y &&
                    p.z >= cubeMin.z && p.z <= cubeMax.z;
        };

        if (isInside(v1) || isInside(v2) || isInside(v3)) {
            // Use the center of the triangle as intersection point
            intersectionPoint = (v1 + v2 + v3) / 3.0f;
            // std::cout << "Collision detected with triangle vertices: "
            //                 << "v1(" << v1.x << ", " << v1.y << ", " << v1.z << "), "
            //                 << "v2(" << v2.x << ", " << v2.y << ", " << v2.z << "), "
            //                 << "v3(" << v3.x << ", " << v3.y << ", " << v3.z << ")\n";
            return true;
        }

        return false;
    }
    else if (object.isSphere()) {
        glm::vec3 center = object.getCenter();
        float radius = object.getHalfLength() + offset;

        // Check if any vertex is inside the sphere
        auto isInsideSphere = [&](const glm::vec3& p) {
            return glm::length(p - center) <= radius;
        };

        if (isInsideSphere(v1) || isInsideSphere(v2) || isInsideSphere(v3)) {
            intersectionPoint = (v1 + v2 + v3) / 3.0f;
            // std::cout << "Collision detected with triangle vertices: "
            //                 << "v1(" << v1.x << ", " << v1.y << ", " << v1.z << "), "
            //                 << "v2(" << v2.x << ", " << v2.y << ", " << v2.z << "), "
            //                 << "v3(" << v3.x << ", " << v3.y << ", " << v3.z << ")\n";
            return true;
        }

        return false;
    }
    return false;
}

void NewCollision::traverseBVH(BVHNode* node, const Object& object, std::vector<GLuint>& potentialTriangles) {
    if (!node) return;

    bool intersects = false;
    if (object.isCube()) {
        glm::vec3 center = object.getCenter();
        float halfLength = object.getHalfLength() + offset;
        AABB cubeAABB{center - glm::vec3(halfLength), center + glm::vec3(halfLength)};
        intersects = node->aabb.intersects(cubeAABB);
    } else if (object.isSphere()) {
        intersects = node->aabb.intersectsSphere(object.getCenter(), object.getHalfLength() + offset);
    }

    if (!intersects) return;

    if (node->isLeaf()) {
        potentialTriangles.insert(potentialTriangles.end(),
            node->triangleIndices.begin(), node->triangleIndices.end());
    } else {
        traverseBVH(node->left, object, potentialTriangles);
        traverseBVH(node->right, object, potentialTriangles);
    }
}

void NewCollision::resolveCollision(
    std::vector<Particle>& particles,
    BVH* clothBVH,
    const std::vector<GLuint>& triangleIndices,
    const Object& object,
    float deltaTime,
    std::vector<GLuint>& collidingIndices, // Pass by reference
    float StaticFriction, float KineticFriction
    ) {

    // Add debug print
    // std::cout << "Starting collision resolution with " << triangleIndices.size() / 3
    //           << " triangles" << std::endl;

    // Validate input
    if (triangleIndices.size() % 3 != 0) {
        return;
    }

    if (!clothBVH || !clothBVH->root) return;

    clothBVH->refit(); // Update BVH with current positions

    std::vector<GLuint> potentialTriangles;
    traverseBVH(clothBVH->root, object, potentialTriangles);

    for (size_t i = 0; i < potentialTriangles.size(); i += 3) {
        bvhCollisionChecks++;
        if (i + 2 >= potentialTriangles.size()) break;
        GLuint i1 = potentialTriangles[i], i2 = potentialTriangles[i+1], i3 = potentialTriangles[i+2];
        if (i1 >= particles.size() || i2 >= particles.size() || i3 >= particles.size()) continue;

        Particle& p1 = particles[i1];
        Particle& p2 = particles[i2];
        Particle& p3 = particles[i3];
        glm::vec3 intersectionPoint, normal;

        // Perform detailed collision check for the triangle
        if (checkTriangleObjectIntersection(
            p1.getPosition(), p2.getPosition(), p3.getPosition(),
            object, intersectionPoint, normal)) {

            // Calculate penetration depths
            float penetrationDepth1 = glm::dot(normal, intersectionPoint - p1.getPosition());
            float penetrationDepth2 = glm::dot(normal, intersectionPoint - p2.getPosition());
            float penetrationDepth3 = glm::dot(normal, intersectionPoint - p3.getPosition());

            // Add colliding indices
            //collidingIndices.push_back(triangleIndices[i]);
            //collidingIndices.push_back(triangleIndices[i + 1]);
            //collidingIndices.push_back(triangleIndices[i + 2]);

            isColliding = true;

            // Resolve collision for each particle
            resolveParticleCollision(p1, normal, penetrationDepth1, deltaTime, StaticFriction, KineticFriction);
            resolveParticleCollision(p2, normal, penetrationDepth2, deltaTime, StaticFriction, KineticFriction);
            resolveParticleCollision(p3, normal, penetrationDepth3, deltaTime, StaticFriction, KineticFriction);
        }
    }
}

void NewCollision::resolveCollisionWithOutBVH(
        std::vector<Particle>& particles,
        const std::vector<GLuint>& triangleIndices,
        const Object& object,
        float deltaTime,
        std::vector<GLuint>& collidingIndices, // Pass by reference
        float StaticFriction, float KineticFriction) {
        // Add debug print
        // std::cout << "Starting collision resolution with " << triangleIndices.size() / 3
        //           << " triangles" << std::endl;

        // Validate input
        if (triangleIndices.size() % 3 != 0) {
            return;
        }

        // Iterate through triangles
        for (size_t i = 0; i < triangleIndices.size(); i += 3) {
            // Bounds check
            collisionChecks++;

            if (i + 2 >= triangleIndices.size() ||
                triangleIndices[i] >= particles.size() ||
                triangleIndices[i + 1] >= particles.size() ||
                triangleIndices[i + 2] >= particles.size()) {
                continue;
            }

            // Get particles forming this triangle
            Particle& p1 = particles[triangleIndices[i]];
            Particle& p2 = particles[triangleIndices[i + 1]];
            Particle& p3 = particles[triangleIndices[i + 2]];

            glm::vec3 intersectionPoint, normal;

            if (checkTriangleObjectIntersection(
                p1.getPosition(), p2.getPosition(), p3.getPosition(),
                object, intersectionPoint, normal)) {

                // Calculate penetration depths
                float penetrationDepth1 = glm::dot(normal, intersectionPoint - p1.getPosition());
                float penetrationDepth2 = glm::dot(normal, intersectionPoint - p2.getPosition());
                float penetrationDepth3 = glm::dot(normal, intersectionPoint - p3.getPosition());

                // isColliding = true;
                // Push the indices of the colliding triangle
                //collidingIndices.push_back(triangleIndices[i]);
                //collidingIndices.push_back(triangleIndices[i + 1]);
                //collidingIndices.push_back(triangleIndices[i + 2]);

                isColliding = true;
                // Resolve collision for each particle
                resolveParticleCollision(p1, normal, penetrationDepth1, deltaTime, StaticFriction, KineticFriction);
                resolveParticleCollision(p2, normal, penetrationDepth2, deltaTime, StaticFriction, KineticFriction);
                resolveParticleCollision(p3, normal, penetrationDepth3, deltaTime, StaticFriction, KineticFriction);
            }
        }
    }


void NewCollision::resolveParticleCollision(
    Particle& particle,
    const glm::vec3& normal,
    float penetrationDepth,
    float deltaTime, float Fs, float Fk) {

    // std::cout << "Resolving particle collision" << std::endl;

    const float repulsionStrength = 500.0f;
    const float restitution = 0.5f;

    const float staticFrictionCoeff = Fs;
    const float kineticFrictionCoeff = Fk;

    // Calculate velocity
    glm::vec3 velocity = particle.getPosition() - particle.getPreviousPosition();

    // Apply repulsion force
    glm::vec3 repulsionForce = normal * (repulsionStrength * std::abs(penetrationDepth));
    particle.applyForce(repulsionForce);
    particle.update(deltaTime);

    // Apply restitution
    glm::vec3 velocityNormal = glm::dot(velocity, normal) * normal;
    //std::cout << "normal: { " << normal.x << ", " << normal.y << ", " << normal.z << "}\n";
    //std::cout << "velocityNormal: { " << velocity.x << ", " << velocity.y << ", " << velocity.z << "}\n";
    glm::vec3 velocityTangent = velocityNormal - velocity;
    glm::vec3 newVelocity = velocityTangent - velocityNormal * restitution;

    // Friction calculation
    float normalForce = glm::length(repulsionForce);
    glm::vec3 frictionDirection = glm::length(velocityTangent) > 0.0001f 
        ? glm::normalize(velocityTangent) 
        : glm::vec3(0.0f);
    
    // Choose between static and kinetic friction
    float frictionCoeff = glm::length(velocityTangent) < 0.0001f 
        ? staticFrictionCoeff 
        : kineticFrictionCoeff;
    
    // Calculate friction force
    glm::vec3 frictionForce = -frictionDirection * (normalForce * frictionCoeff);

    // Apply friction
    newVelocity += frictionForce / particle.getMass();

    // Update particle
    particle.setPreviousPosition(particle.getPosition() - newVelocity);

    // Correct position if there's penetration
    if (penetrationDepth < 0.0f) {
        particle.setPosition(particle.getPosition() - normal * penetrationDepth);
    }
}
