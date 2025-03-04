#include "ImGuiManager.h"

ImGuiManager::ImGuiManager()
    : clear_color(ImVec4(0.45f, 0.55f, 0.60f, 1.00f)), show_demo_window(true), show_another_window(false)
{
}

ImGuiManager::~ImGuiManager()
{
}

void ImGuiManager::Init(GLFWwindow* window, const char* glsl_version)
{
    this->window = window;
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

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

void ImGuiManager::Render()
{
    // 1. Show demo window
    //if (show_demo_window)
    //    ImGui::ShowDemoWindow(&show_demo_window);

    // 2. Show a custom window
    static bool showSidebar = true;  // Sidebar visibility toggle

    ImGui::Begin("Sidebar Control", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.6f, 1.0f)); // Subtle blue button
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.5f, 0.7f, 1.0f));
    if (ImGui::Button(showSidebar ? "Hide Controls" : "Show Controls", ImVec2(120, 30)))
    {
        showSidebar = !showSidebar;
    }
    ImGui::PopStyleColor(2);
    ImGui::End();

    if (showSidebar) {

        // Set position and size for left sidebar
        ImGui::SetNextWindowPos(ImVec2(0, 20)); // Adjust position slightly for main menu bar
        ImGui::SetNextWindowSize(ImVec2(300, ImGui::GetIO().DisplaySize.y - 20)); // Adjust width as needed

        ImGui::Begin("Sidebar", &showSidebar, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_MenuBar);
        //ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
        

        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("Simulation"))
            {
                ImGui::MenuItem("Controls", nullptr, false, false);
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + 290.0f);

        ImGui::BeginGroup();
        ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "Simulation Settings");
        ImGui::Separator();

        if (StartSimulation) {
            ImGui::Checkbox("Start Simulation", StartSimulation);
        }
        ImGui::EndGroup();

        ImGui::Spacing();

        ImGui::BeginGroup();
        ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "Cloth Dynamics");
        ImGui::Separator();

        if (toggleWind) {
            ImGui::Checkbox("Wind Enabled", toggleWind);
        }
        if (toggleCloth) {
            // Track the previous state of the cloth orientation
            static bool prevToggleCloth = *toggleCloth;

            // Show the checkbox for cloth orientation
            if (ImGui::Checkbox("Cloth Orientation", toggleCloth)) {
                // If the state has changed, set clothNeedsReset to true
                if (*toggleCloth != prevToggleCloth) {
                    if (clothNeedsReset) {
                        *clothNeedsReset = true;
                    }
                    prevToggleCloth = *toggleCloth; // Update the previous state
                }
            }
        }

        // Add a slider for gravity
        if (gravity) {
            ImGui::SliderFloat("Gravity", gravity, -0.0f, -0.1f); // Adjust range as needed
        }

        ImGui::EndGroup();

        ImGui::Spacing();

        ImGui::BeginGroup();
        ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "Spring Parameters");
        ImGui::Separator();

        if (k) {
            ImGui::InputFloat("Structural Spring Constant", k, 0.1f, 1.0f, "%.3f");  // Adjust format specifier as needed
        }

        if (ShearK) {
            ImGui::InputFloat("Shear Spring Constant", ShearK, 0.1f, 1.0f, "%.3f");  // Adjust format specifier as needed
        }

        ImGui::EndGroup();

        ImGui::Spacing();

        ImGui::BeginGroup();
        ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "Visualization");
        ImGui::Separator();

        if (showFur) {
            ImGui::Checkbox("Show Fur", showFur);
        }

        if (ShowParticle) {
            ImGui::Checkbox("Show Particles", ShowParticle);
        }
        if (ShowSpring) {
            ImGui::Checkbox("Show Springs", ShowSpring);
        }

        ImGui::EndGroup();

        ImGui::Spacing();

        ImGui::BeginGroup();
        ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "Lighting");
        ImGui::Separator();

        if (LightPosition) {
            ImGui::DragFloat3("Light Position", glm::value_ptr(*LightPosition), 0.1f, -100.0f, 100.0f);
        }
        //ImGui::Checkbox("Demo Window", &show_demo_window);
        //ImGui::Checkbox("Another Window", &show_another_window);

        //ImGui::SliderFloat("float", &f, 0.0f, 1.0f); // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("Background Color", (float*)&clear_color); // Edit 3 floats representing a color

        if (LightColor) {  // Ensure the pointer is valid before using it
            ImGui::ColorEdit3("Light Color", (float*)LightColor);
        }

        ImGui::EndGroup();

        ImGui::Spacing();

        //if (ImGui::Button("Button"))
        //    counter++;
        //ImGui::SameLine();
        //ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        
        //ImGui::PopStyleVar();
        ImGui::PopTextWrapPos();
        ImGui::End();
    }
}

void ImGuiManager::RenderWireframeToggle()
{
    ImGui::PushFont(menu_font);
    ImGui::Begin("Mode Change");
    if (ImGui::Button(wireframeMode ? "Solid Mode" : "Wireframe Mode", ImVec2(150, 30)))
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