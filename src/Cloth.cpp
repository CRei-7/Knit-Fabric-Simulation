// Cloth.cpp
#include "Cloth.h"

Cloth::Cloth(int columns, int rows, float distance, float springConstant)
    : numColumns(columns), numRows(rows),
      particleDistance(distance), springK(springConstant),
      mesh(nullptr) {
    std::cout << "Cloth constructor" << std::endl;
    setUpMesh();
    // reset(true);
}

Cloth::~Cloth() {
    if (mesh) {
        delete mesh;
    }
}

void Cloth::reset(bool verticalOrientation) {
    isVertical = verticalOrientation;
    particles.clear();
    springs.clear();
    vertices.clear();
    indices.clear();

    // Initialize particles
    for (int i = 0; i < numColumns; ++i) {
        for (int j = 0; j < numRows; ++j) {
            float x = i * particleDistance;
            float y = isVertical ? -j * particleDistance : 0.0f;
            float z = isVertical ? 0.0f : j * particleDistance;

            bool isStatic = isVertical ? (j == 0) : false;
            particles.emplace_back(glm::vec3(x, y, z), isStatic);
        }
    }
    std::cout << "Particles initialized" << std::endl;
    std::cout << "cloth" << particles.size() << std::endl;

    // Initialize springs (same for both orientations)
    for (int i = 0; i < numColumns; ++i) {
        for (int j = 0; j < numRows; ++j) {
            // Right edge, avoiding the addition of spring to the right side
            if (i != numColumns - 1) {
                springs.emplace_back(k, disX, &particles[i * numRows + j], &particles[(i + 1) * numRows + j]);
            }

            // Bottom edge, avoiding the addition of spring to the bottom side
            if (j != numRows - 1) {
                springs.emplace_back(k, disY, &particles[i * numRows + j], &particles[i * numRows + (j + 1)]);
            }

            // Shear springs
            if (i != numColumns - 1 && j != numRows - 1) {
                springs.emplace_back(shearK, sqrt(disX * disX + disY * disY), &particles[i * numRows + j], &particles[(i + 1) * numRows + (j + 1)]);
                springs.emplace_back(shearK, sqrt(disX * disX + disY * disY), &particles[(i + 1) * numRows + j], &particles[i * numRows + (j + 1)]);
            }
        }
    }
    std::cout << "cloth" << springs.size() << std::endl;
    setupMesh();
}

void Cloth::setupMesh() {
    // Create vertices from particles
    // vertices.clear();
    // for (const auto& particle : particles) {
    //     Vertex vertex;
    //     vertex.Position = particle.getPosition();
    //     vertex.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
    //     vertex.TexCoords = glm::vec2(0.0f, 0.0f);
    //     vertices.push_back(vertex);
    // }

    // Generate indices for triangles
    // indices.clear();
    // for (int i = 0; i < numColumns - 1; ++i) {
    //     for (int j = 0; j < numRows - 1; ++j) {
    //         indices.push_back(i * numRows + j);
    //         indices.push_back((i + 1) * numRows + j);
    //         indices.push_back(i * numRows + (j + 1));

    //         indices.push_back((i + 1) * numRows + (j + 1));
    //         indices.push_back(i * numRows + (j + 1));
    //         indices.push_back((i + 1) * numRows + j);
    //     }
    // }
    // // Create new mesh
    // if (mesh) {
    //     delete mesh;
    // }
    // std::vector<Texture> textures; // Empty textures vector
    std::vector<Vertex> vertices = {
        {{0.0f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.5f, 1.0f}},
        {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}
    };

    std::vector<unsigned int> indices = { 0, 1, 2 };
   Mesh mesh(vertices, indices);
}

// void Cloth::updateMesh() {
//     // Update vertex positions and calculate normals
//     for (size_t i = 0; i < particles.size(); i++) {
//         vertices[i].Position = particles[i].getPosition();
//         vertices[i].Normal = glm::vec3(0.0f); // Reset normals
//     }

//     // Calculate normals
//     for (size_t i = 0; i < indices.size(); i += 3) {
//         glm::vec3& pos1 = vertices[indices[i]].Position;
//         glm::vec3& pos2 = vertices[indices[i + 1]].Position;
//         glm::vec3& pos3 = vertices[indices[i + 2]].Position;

//         glm::vec3 edge1 = pos2 - pos1;
//         glm::vec3 edge2 = pos3 - pos1;
//         glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

//         vertices[indices[i]].Normal += normal;
//         vertices[indices[i + 1]].Normal += normal;
//         vertices[indices[i + 2]].Normal += normal;
//     }

//     // Normalize accumulated normals
//     for (auto& vertex : vertices) {
//         vertex.Normal = glm::normalize(vertex.Normal);
//     }

//     // Update mesh with new vertex data
//     mesh->updateVertices(vertices);
// }

// void Cloth::update(float deltaTime, bool enableWind) {
//     // Update particles and springs
//     for (auto& particle : particles) {
//         particle.update(deltaTime);
//         // particle.render(shaderProgram, view, projection);
//     }

//     for (auto& spring : springs) {
//         spring.update();
//     }

//     updateMesh();
// }

void Cloth::render(Shader& shader, const glm::mat4& view, const glm::mat4& projection) {
    shader.use();
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    shader.setMat4("model", glm::mat4(1.0f));

    // Set lighting uniforms
    shader.setVec3("lightPos", glm::vec3(5.0f, 5.0f, 5.0f));
    shader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    shader.setVec3("Color", glm::vec3(0.7f, 0.2f, 0.2f));

    mesh->Draw(shader);
}
