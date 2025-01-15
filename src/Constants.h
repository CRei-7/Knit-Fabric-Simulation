#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <glm/glm.hpp>
#include <random>
#include <cmath>

// External variables
extern bool g_ImGuiWantCaptureMouse;
extern float modelRotation;
extern bool isRotating;
extern glm::vec3 windDirection; // Wind direction
extern float windTimer;         // Timer for wind direction change

// Function to generate random wind direction
inline glm::vec3 getRandomWindDirection() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> azimuthalDist(0.0, 2.0 * glm::pi<float>());
    std::uniform_real_distribution<> zDist(-1.0, 1.0);

    float azimuthalAngle = static_cast<float>(azimuthalDist(gen));
    float z = static_cast<float>(zDist(gen));
    float radius = std::sqrt(1.0f - z * z);
    float x = radius * std::cos(azimuthalAngle);
    float y = radius * std::sin(azimuthalAngle);

    return glm::vec3(x, y, z);
}

// Screen dimensions
constexpr unsigned int SCR_WIDTH = 1280;
constexpr unsigned int SCR_HEIGHT = 720;

// Grid parameters
constexpr int column = 10; ///< Number of columns
constexpr int row = 10;    ///< Number of rows
constexpr float disX = 0.05f; ///< Distance between particles in x direction
constexpr float disY = 0.05f; ///< Distance between particles in y direction
constexpr float initialY = 0.3f; ///< Y-coordinate for the top pinned particle
constexpr glm::vec3 Offset(-0.5f, 0.0f, 0.0f); ///< Offset for initial position
constexpr float k = 50.0f; ///< Structural Spring constant
constexpr float shearK = 10.5f; ///< Shear spring constant

// Gravity
constexpr float gravity = -0.05f;

// Wind parameters
constexpr float windScale = 0.02f; ///< Base wind strength
constexpr float windOffsetSpeed = 0.1f; ///< Variability in wind strength
constexpr float windChangeInterval = 0.5f; ///< Time interval to change wind direction

#endif // CONSTANTS_H
