#include "ImGuiManager.h"
#include <iostream>

ImGuiManager::ImGuiManager()
    : clear_color(ImVec4(0.45f, 0.55f, 0.60f, 1.00f)), show_demo_window(false), show_another_window(false)
{
}

ImGuiManager::~ImGuiManager(){}

void ImGuiManager::Init(GLFWwindow* window, const char* glsl_version)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.Fonts->AddFontDefault();
    menu_font = io.Fonts->AddFontFromFileTTF("../utils/fonts/Roboto-Regular.ttf", 20.0f); // Load font
    if(menu_font == nullptr){
        std::cerr << "Font file not found" << std::endl;
    }
    IM_ASSERT(menu_font != nullptr);

    // Rebuild font atlas
    io.Fonts->Build();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

    // Setup Dear ImGui style
    // ImGuiStyle& style = ImGui::GetStyle();
    ImGui::StyleColorsLight();

    // // Increase menu bar height
    // style.FramePadding.y = 10.0f; // Increase the padding inside menu items
    // style.ItemSpacing.y = 5.0f;  // Increase spacing between menu items
    // style.WindowPadding = ImVec2(0, 0); // Optional: adjust padding around the window if needed

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

void ImGuiManager::BeginFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiManager::EndFrame()
{
    ImGui::Render();
}

// void ImGuiManager::Render()
// {
//     // 2. Show a custom window
//     {
//         static float f = 0.0f;
//         static int counter = 0;
//         ImGui::PushFont(menu_font);
//         ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.

//         ImGui::Text("This is some useful text.");
//         ImGui::Checkbox("Demo Window", &show_demo_window);
//         ImGui::Checkbox("Another Window", &show_another_window);

//         ImGui::SliderFloat("float", &f, 0.0f, 1.0f); // Edit 1 float using a slider from 0.0f to 1.0f
//         ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

//         if (ImGui::Button("Button"))
//             counter++;
//         ImGui::SameLine();
//         ImGui::Text("counter = %d", counter);

//         ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
//         ImGui::PopFont();
//         ImGui::End();
//     }
// }

void ImGuiManager::RenderWireframeToggle()
{
    ImGui::PushFont(menu_font);
    ImGui::Begin("Mode Change");
    if (ImGui::Button(wireframeMode ? "Solid Mode" : "Wireframe Mode",ImVec2(150,30)))
    {
        wireframeMode = !wireframeMode;
    }
    ImGui::PopFont();
    ImGui::End();
}

void ImGuiManager::Cleanup()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiManager::SetupMenuBar(GLFWwindow* window, bool* should_close)
{
    if (ImGui::BeginMainMenuBar())
    {
        ImGui::PushFont(menu_font);
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New", "Ctrl+N")) { /* Handle new */ }
            if (ImGui::MenuItem("Open...", "Ctrl+O")) { /* Handle open */ }
            ImGui::Separator();
            if (ImGui::MenuItem("Save", "Ctrl+S")) { /* Handle save */ }
            if (ImGui::MenuItem("Save As...")) { /* Handle save as */ }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit Window", "Alt+F4")) { *should_close = true; }
            ImGui::EndMenu();
        }
        ImGui::PopFont();
        ImGui::EndMainMenuBar();
    }
}

ImVec4& ImGuiManager::GetClearColor()
{
    return clear_color;
}

bool& ImGuiManager::ShowDemoWindow()
{
    return show_demo_window;
}

bool& ImGuiManager::ShowAnotherWindow()
{
    return show_another_window;
}
