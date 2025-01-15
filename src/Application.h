#ifndef APPLICATION_H
#define APPLICATION_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include "Shader.h"
#include "Model.h"
#include "Camera.h"
#include "ImGuiManager.h"
#include "Cloth.h"

class Application
{
public:
    Application();
    ~Application();

    float deltaTime;//time between current and last frame
    float lastFrame;

    bool Init();
    void MainLoop();
    void Cleanup();

    Shader* importedModelShader;
    Shader* clothShader;
    Model* ourModel;
    Cloth* cloth;
    Camera camera;

private:
    GLFWwindow* window;
    ImGuiManager imgui_manager;
    const char* glsl_version;

    GLuint vertexArray;
    GLuint vertexBuffer;

    unsigned int VBO;//Vertex Buffer Object which stores a large number of vertices in the GPU's memory
    unsigned int VAO;//Vertex Array Object whcih stores all the vertex attribute settings in a single object
    unsigned int EBO;//Element Buffer Object for referencing values in VBO using indices

    bool toggle_wind;
    bool cKeyPressed;
    bool tKeyPressed;
    bool cursorVisible;
    bool toggleClothOrientation;//true for hanging cloth and false for falling cloth
    bool oKeyPressed;//Press 'O' for change in orientation
    bool clothNeedsReset;//for reseting cloth for new orientation
};

#endif // APPLICATION_H
