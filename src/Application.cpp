#include "Application.h"
#include "InputHandler.h"
#include <stb/stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdio.h>
#include <iostream>

// Error callback function
static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

float modelRotation = 0.0f;
bool isRotating = false;
bool g_ImGuiWantCaptureMouse = false;

// Constructor
Application::Application() : window(nullptr), glsl_version("#version 330"), cKeyPressed(false), tKeyPressed(false) {}

// Destructor
Application::~Application() {}

// Initialize GLFW, OpenGL, and ImGui
bool Application::Init()
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return false;
    }
    // Setup OpenGL context version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Model Loading", nullptr, nullptr);
    if (window == nullptr){
        fprintf(stderr, "Failed to create GLFW window\n");
        return false;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    glfwSetWindowUserPointer(window, this);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);

    //Tells GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    cursorVisible = false;
    toggle_wind = false;
    toggleClothOrientation = true;
    clothNeedsReset = false;

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD\n";
        return false;
    }

    // Initialize ImGui
    imgui_manager.Init(window, glsl_version);

    // For model loading
    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    std::cout << "Loading shaders" << std::endl;
    // importedModelShader = new Shader("../shaders/modelVertex.vert", "../shaders/modelFragment.frag");
    clothShader = new Shader("../shaders/VertShader.glsl", "../shaders/FragShader.glsl");
    cloth = new Cloth(20, 20, 0.05f, 100.0f);
    // std::cout << "Loading models" << std::endl;
    // // load models
    // ourModel = new Model("../models/backpack/backpack.obj");

    return true;
}

// Main rendering loop
void Application::MainLoop()
{
    while (!glfwWindowShouldClose(window))
    {
        bool should_close = false;

        //for variable speed of movement
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, deltaTime);

        glfwPollEvents();
        imgui_manager.BeginFrame();
        g_ImGuiWantCaptureMouse = ImGui::GetIO().WantCaptureMouse;
        imgui_manager.SetupMenuBar(window, &should_close);

        if (should_close)
            glfwSetWindowShouldClose(window, true);

        imgui_manager.RenderWireframeToggle();
        // imgui_manager.Render();
        imgui_manager.EndFrame();

        // Rendering
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);

        glViewport(0, 0, display_w, display_h);
        // const ImVec4& clear_color = imgui_manager.GetClearColor();
        // glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        // glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // // use shaders
        // importedModelShader->use();

        // // view/projection transformations for imported model
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        // importedModelShader->setMat4("projection", projection);
        // importedModelShader->setMat4("view", view);

        // // render the loaded model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 0.0f)); // translate it down so it's at the left of the scene
        model = glm::rotate(model, glm::radians(modelRotation), glm::vec3(0.0f, 1.0f, 0.0f)); // rotate around Y-axis
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f)); // it's a bit too big for our scene, so scale it down
        // importedModelShader->setMat4("model", model);

        // ourModel->Draw(*importedModelShader, imgui_manager.wireframeMode);

        // Update to the wind direction periodically
        // windTimer += deltaTime;
        // if (windTimer >= windChangeInterval) {
        //     windDirection = getRandomWindDirection(); //Randomizes the wind direction
        //     windTimer = 0.0f; //Timer reset
        // }

        // cloth->update(deltaTime, toggle_wind);
        // if (toggle_wind) {
        //     glm::vec3 windForce = getRandomWindDirection() * windScale;
        //     cloth->applyWindForce(windForce, windScale, windOffsetSpeed);
        // }

        cloth->render(*clothShader, view, projection);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
}

// Cleanup resources
void Application::Cleanup()
{
    // Cleanup ImGui
    imgui_manager.Cleanup();

    // delete our shader
    delete importedModelShader;
    delete clothShader;
    // delete our model
    delete ourModel;
    delete cloth;

    glfwDestroyWindow(window);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glfwTerminate();
}
