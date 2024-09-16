#ifndef IMGUIMANAGER_H
#define IMGUIMANAGER_H

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

class ImGuiManager
{
public:
    ImGuiManager();
    ~ImGuiManager();

    void Init(GLFWwindow* window, const char* glsl_version);
    void BeginFrame();
    void EndFrame();
    void Render();
    void Cleanup();

    void SetupMenuBar(GLFWwindow* window, bool* should_close);

    ImVec4& GetClearColor();
    bool& ShowDemoWindow();
    bool& ShowAnotherWindow();

private:
    ImVec4 clear_color;
    bool show_demo_window;
    bool show_another_window;
};

#endif // IMGUIMANAGER_H
