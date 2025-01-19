// #include <glad/glad.h>
// #include <GLFW/glfw3.h>
// #include <vector>
// #include <iostream>

// struct Vertex {
//     float position[3];
//     float color[3];
// };

// class Mesh {
// public:
//     Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
//         : vertices(vertices), indices(indices) {
//         setupMesh();
//     }

//     ~Mesh() {
//         glDeleteVertexArrays(1, &VAO);
//         glDeleteBuffers(1, &VBO);
//         glDeleteBuffers(1, &EBO);
//     }

//     void Draw(unsigned int shaderProgram, const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {
//         glUseProgram(shaderProgram);

//         // Set uniform matrices
//         unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
//         unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
//         unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");

//         glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
//         glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
//         glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

//         // Bind VAO and draw
//         glBindVertexArray(VAO);
//         glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
//         glBindVertexArray(0);
//     }

// private:
//     unsigned int VAO, VBO, EBO;
//     std::vector<Vertex> vertices;
//     std::vector<unsigned int> indices;

//     void setupMesh() {
//         glGenVertexArrays(1, &VAO);
//         glGenBuffers(1, &VBO);
//         glGenBuffers(1, &EBO);

//         glBindVertexArray(VAO);

//         glBindBuffer(GL_ARRAY_BUFFER, VBO);
//         glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

//         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
//         glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

//         glEnableVertexAttribArray(0); // Position attribute
//         glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

//         glEnableVertexAttribArray(1); // Color attribute
//         glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, color)));

//         glBindVertexArray(0);
//     }
// };
