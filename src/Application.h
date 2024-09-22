#ifndef APPLICATION_H
#define APPLICATION_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>
#include <random>
#include <cmath>
#include "ImGuiManager.h"
#include "Particle.h"
#include "Spring.h"
#include "Object.h"
#include "Collision.h"

class Application
{
public:
    Application();
    ~Application();
    bool Init();
    void MainLoop();
    void Cleanup();

private:
    GLFWwindow* window;
    ImGuiManager imgui_manager;
    const char* glsl_version;
    GLuint shaderProgram;
    GLuint vertexArray;
    GLuint vertexBuffer;

    unsigned int VBO;//Vertex Buffer Object which stores a large number of vertices in the GPU's memory
    unsigned int VAO;//Vertex Array Object whcih stores all the vertex attribute settings in a single object
    unsigned int EBO;//Element Buffer Object for referencing values in VBO using indices

    glm::vec3 cameraPos;//camera position
    glm::vec3 cameraFront;//for specifying the direction in which camera is pointing
    glm::vec3 cameraUp;//positive y-axis for camera

    //for mouse movement
    bool firstMouse;//to remove sudden jump
    float yaw; //horizontal movement
    float pitch; //vertical movement
    float lastX; //last x position of mouse
    float lastY; //last y position of mouse
    float fov; //for scrolling

    float deltaTime;//time between current and last frame
    float lastFrame;

    void SetupOpenGL();
    void processInput(GLFWwindow* window);
    std::string readShaderSource(const char* filePath);

    bool cursorVisible;//This is used to make sure when cursor is visible, yaw or pitch doesn't occur
    bool cKeyPressed;//Press 'C' for toggling of visibility of cursor
    void mouse_callback(GLFWwindow* window, double xpos, double ypos);
    void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

    bool toggle_wind;
    bool tKeyPressed;//Press 'T' for wind
    void setupCloth();

    bool toggleClothOrientation;//true for hanging cloth and false for falling cloth
    bool oKeyPressed;//Press 'O' for change in orientation
    bool clothNeedsReset;//for reseting cloth for new orientation

    //cloth parameters
    float windTimer;
    float windChangeInterval;
    glm::vec3 windDirection;
    float windScale;
    float windOffsetSpeed;
    float gravity;

    std::vector<Particle> particles;
    std::vector<Spring> springs;

};

#endif // APPLICATION_H
