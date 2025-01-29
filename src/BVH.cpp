#include "BVH.h"
#include <algorithm>
#include <limits>

bool AABB::intersects(const AABB& other) const {
    return (min.x <= other.max.x && max.x >= other.min.x) &&
           (min.y <= other.max.y && max.y >= other.min.y) &&
           (min.z <= other.max.z && max.z >= other.min.z);
}

bool AABB::intersectsSphere(const glm::vec3& center, float radius) const {
    glm::vec3 closest = glm::clamp(center, min, max);
    float distance = glm::length(center - closest);
    return distance <= radius;
}

BVHNode::~BVHNode() {
    delete left;
    delete right;
}

void BVHNode::refit(const std::vector<Particle>& particles) {
    if (isLeaf()) {
        aabb.min = glm::vec3(std::numeric_limits<float>::max());
        aabb.max = glm::vec3(-std::numeric_limits<float>::max());
        for (size_t i = 0; i < triangleIndices.size(); i += 3) {
            const glm::vec3& v1 = particles[triangleIndices[i]].getPosition();
            const glm::vec3& v2 = particles[triangleIndices[i+1]].getPosition();
            const glm::vec3& v3 = particles[triangleIndices[i+2]].getPosition();
            aabb.min = glm::min(aabb.min, glm::min(v1, glm::min(v2, v3)));
            aabb.max = glm::max(aabb.max, glm::max(v1, glm::max(v2, v3)));
        }
    } else {
        left->refit(particles);
        right->refit(particles);
        aabb.min = glm::min(left->aabb.min, right->aabb.min);
        aabb.max = glm::max(left->aabb.max, right->aabb.max);
    }
}

bool BVHNode::isLeaf() const { return !left && !right; }

BVH::BVH(const std::vector<Particle>& particles, const std::vector<GLuint>& triangleIndices)
    : particles(particles) {
    root = build(triangleIndices);
}

BVH::~BVH() { delete root; }

void BVH::refit() {
    if (root) root->refit(particles);
}

BVHNode* BVH::build(const std::vector<GLuint>& triangleIndices) {
    if (triangleIndices.size() <= 6) { // Leaf node (2 triangles)
        BVHNode* node = new BVHNode();
        node->triangleIndices = triangleIndices;
        node->refit(particles);
        return node;
    }

    AABB centroidAABB = computeCentroidAABB(triangleIndices);
    glm::vec3 extent = centroidAABB.max - centroidAABB.min;
    int axis = (extent.x > extent.y && extent.x > extent.z) ? 0 :
               (extent.y > extent.z) ? 1 : 2;
    float splitValue = (centroidAABB.min[axis] + centroidAABB.max[axis]) * 0.5f;

    std::vector<GLuint> leftIndices, rightIndices;
    for (size_t i = 0; i < triangleIndices.size(); i += 3) {
        glm::vec3 centroid = (
            particles[triangleIndices[i]].getPosition() +
            particles[triangleIndices[i+1]].getPosition() +
            particles[triangleIndices[i+2]].getPosition()
        ) / 3.0f;
        if (centroid[axis] < splitValue) {
            leftIndices.insert(leftIndices.end(), triangleIndices.begin()+i, triangleIndices.begin()+i+3);
        } else {
            rightIndices.insert(rightIndices.end(), triangleIndices.begin()+i, triangleIndices.begin()+i+3);
        }
    }

    if (leftIndices.empty() || rightIndices.empty()) { // Fallback split
        size_t mid = triangleIndices.size() / 2;
        mid -= mid % 3;
        leftIndices = std::vector<GLuint>(triangleIndices.begin(), triangleIndices.begin() + mid);
        rightIndices = std::vector<GLuint>(triangleIndices.begin() + mid, triangleIndices.end());
    }

    BVHNode* node = new BVHNode();
    node->left = build(leftIndices);
    node->right = build(rightIndices);
    node->refit(particles);
    return node;
}

AABB BVH::computeCentroidAABB(const std::vector<GLuint>& triangleIndices) {
    AABB aabb;
    aabb.min = glm::vec3(std::numeric_limits<float>::max());
    aabb.max = glm::vec3(-std::numeric_limits<float>::max());
    for (size_t i = 0; i < triangleIndices.size(); i += 3) {
        glm::vec3 centroid = (
            particles[triangleIndices[i]].getPosition() +
            particles[triangleIndices[i+1]].getPosition() +
            particles[triangleIndices[i+2]].getPosition()
        ) / 3.0f;
        aabb.min = glm::min(aabb.min, centroid);
        aabb.max = glm::max(aabb.max, centroid);
    }
    return aabb;
}
