#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "Particle.h"

struct AABB {
    glm::vec3 min;
    glm::vec3 max;

    bool intersects(const AABB& other) const;
    bool intersectsSphere(const glm::vec3& center, float radius) const;
};

class BVHNode {
public:
    AABB aabb;
    std::vector<GLuint> triangleIndices;
    BVHNode* left = nullptr;
    BVHNode* right = nullptr;

    ~BVHNode();
    void refit(const std::vector<Particle>& particles);
    bool isLeaf() const;
};

class BVH {
public:
    BVHNode* root;
    const std::vector<Particle>& particles;

    BVH(const std::vector<Particle>& particles, const std::vector<GLuint>& triangleIndices);
    ~BVH();
    void refit();

private:
    BVHNode* build(const std::vector<GLuint>& triangleIndices);
    AABB computeCentroidAABB(const std::vector<GLuint>& triangleIndices);
};
