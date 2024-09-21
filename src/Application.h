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
    bool cKeyPressed;
    void mouse_callback(GLFWwindow* window, double xpos, double ypos);
    void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

    bool toggle_wind;
    bool tKeyPressed;
};

#endif // APPLICATION_H
