#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Particle.h"

class Spring {
private:
    Particle* p1;  // First particle attached to the spring
    Particle* p2;  // Second particle attached to the spring

    float k;       // Spring constant
    float restLength;  // Rest length of the spring

    glm::vec3 color;// for spring color

public:
    Spring(float _k, float _length, Particle* _p1, Particle* _p2)
        : k(_k), restLength(_length), p1(_p1), p2(_p2), color(glm::vec3(1.0f, 1.0f, 1.0f)) {}

    // Applies Hooke's Law to update forces between the particles
    void update() {
        glm::vec3 vector = p2->getPosition() - p1->getPosition();
        float currentLength = glm::length(vector);
        glm::vec3 direction = glm::normalize(vector);

        // Hooke's law: F = -k * (currentLength - restLength)
        glm::vec3 force = -k * (currentLength - restLength) * direction;

        // Application of forces to the particles
        p2->applyForce(force);
        p1->applyForce(-force);  // Force opposite to p1
    }

    // Rendering of the spring as a line between two particles
    void render(GLuint shaderProgram, const glm::mat4& model,const glm::mat4& view, const glm::mat4& projection) {
        // Uses the shader program
        glUseProgram(shaderProgram);

        //Initialization of model matrix as identity since no additional transformation is applied to the spring itself
        //glm::mat4 model = glm::mat4(1.0f);

        // Passing the matrices to the shader
        GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Sets the color of spring
        GLint colorLoc = glGetUniformLocation(shaderProgram, "Color");
        glUniform3fv(colorLoc, 1, &color[0]);

        // Vertices for the spring (positions of p1 and p2)
        glm::vec3 lineVertices[2] = {
            p1->getPosition(),  // Position of p1
            p2->getPosition()   // Position of p2
        };

        // Sends the vertices to OpenGL
        GLuint VBO, VAO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertices), &lineVertices, GL_STATIC_DRAW);

        // Defines the vertex attribute for position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Draws the spring (as a line)
        glBindVertexArray(VAO);
        glDrawArrays(GL_LINES, 0, 2);

        // Clean up
        glBindVertexArray(0);
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }
};
