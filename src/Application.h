#ifndef APPLICATION_H
#define APPLICATION_H

#include <GLFW/glfw3.h>
#include "ImGuiManager.h"

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
};

#endif // APPLICATION_H
