// Constants.h
#include <glm/glm.hpp>
#include <random>
#include <cmath>

#ifndef CONSTANTS_H
#define CONSTANTS_H

glm::vec3 getRandomWindDirection();

// Screen dimensions
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// Grid parameters
const int column = 10; // Number of columns
const int row = 10;    // Number of rows
const float disX = 0.1f; // Distance between particles in x direction
const float disY = 0.1f; // Distance between particles in y direction
const float initialY = 0.3f; // Y-coordinate for the top pinned particle
glm::vec3 Offset(-0.5f, 0.0f, 0.0f); // Offset for initial position
const float k = 50.0f; // Structural Spring constant
const float shearK = 10.5f; // Shear spring constant

const float gravity = -0.02f;

// Wind parameters
glm::vec3 windDirection = getRandomWindDirection(); // Initial random wind direction
const float windScale = 0.02f; // Base wind strength
const float windOffsetSpeed = 0.1f; // Variability in wind strength
const float windChangeInterval = 0.5f; // Time in seconds to change wind direction
float windTimer = 0.0f; // Timer for wind direction change

// for random wind direction
glm::vec3 getRandomWindDirection() {
    // Creates a random number generator
    std::random_device rd;
    std::mt19937 gen(rd());

    // Uniform distribution for azimuthal angle (0 to 2*PI)
    std::uniform_real_distribution<> azimuthalDist(0.0, 2.0 * glm::pi<float>());

    // Uniform distribution for z component of the vector (cosine of the polar angle)
    std::uniform_real_distribution<> zDist(-1.0, 1.0);

    // Generates random azimuthal angle (phi)
    float azimuthalAngle = static_cast<float>(azimuthalDist(gen));

    // Generates random z component
    float z = static_cast<float>(zDist(gen));

    // Calculates the corresponding x and y components using azimuthal angle and radius (r = sqrt(1 - z^2))
    float radius = std::sqrt(1.0f - z * z);
    float x = radius * std::cos(azimuthalAngle);
    float y = radius * std::sin(azimuthalAngle);

    // Returns the random normalized vector
    return glm::vec3(x, y, z);
}

#endif // CONSTANTS_H
