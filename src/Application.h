#ifndef APPLICATION_H
#define APPLICATION_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Shader.h"
#include "Model.h"
#include "ImGuiManager.h"

class Application
{
public:
    Application();
    ~Application();
    bool Init();
    void MainLoop();
    void Cleanup();

    Shader* ourShader;
    Model* ourModel;

private:
    GLFWwindow* window;
    ImGuiManager imgui_manager;
    const char* glsl_version;
};

#endif // APPLICATION_H
