#include "Application.h"
#include "Camera.h"
#include "InputHandler.h"
#include <stb/stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdio.h>
#include <iostream>
#include  "Constants.h"


// Error callback function
static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

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
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // cursorVisible = false;
    toggle_wind = true;

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
    importedModelShader = new Shader("../shaders/modelVertex.vert", "../shaders/modelFragment.frag");
    clothShader = new Shader("../shaders/VertShader.glsl", "../shaders/FragShader.glsl");

    std::cout << "Loading models" << std::endl;
    // // load models
    ourModel = new Model("../models/backpack/backpack.obj");

    return true;
}

// Main rendering loop
void Application::MainLoop()
{
    std::vector<Particle> particles;
    particles.reserve(column * row); // Reserve space to avoid multiple allocations

    // Initialization of particles
    for (int i = 0; i < column; ++i) {
        for (int j = 0; j < row; ++j) {
            //Position for each particle
            float xPos = i * disX + Offset.x;
            float yPos = initialY - j * disY; // Starts from initialY and moves downward

            bool staticParticle = j == 0; // Top row particles are static

            particles.emplace_back(glm::vec3(xPos, yPos, 0.0f), staticParticle);
        }
    }

    std::vector<Spring> springs;

    // Initialization of springs
    for (int i = 0; i < column; ++i) {
        for (int j = 0; j < row; ++j) {
            // Right edge, avoiding the addition of spring to the right side
            if (i != column - 1) {
                springs.emplace_back(k, disX, &particles[i * row + j], &particles[(i + 1) * row + j]);
            }

            // Bottom edge, avoiding the addition of spring to the bottom side
            if (j != row - 1) {
                springs.emplace_back(k, disY, &particles[i * row + j], &particles[i * row + (j + 1)]);
            }

            // Shear springs
            if (i != column - 1 && j != row - 1) {
                springs.emplace_back(shearK, sqrt(disX * disX + disY * disY), &particles[i * row + j], &particles[(i + 1) * row + (j + 1)]);
                springs.emplace_back(shearK, sqrt(disX * disX + disY * disY), &particles[(i + 1) * row + j], &particles[i * row + (j + 1)]);
            }
        }
    }

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

        // use shaders
        importedModelShader->use();

        // view/projection transformations for imported model
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        importedModelShader->setMat4("projection", projection);
        importedModelShader->setMat4("view", view);

        // render the loaded model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f)); // it's a bit too big for our scene, so scale it down
        importedModelShader->setMat4("model", model);

        ourModel->Draw(*importedModelShader, imgui_manager.wireframeMode);

        // Now use cloth shader
        clothShader->use();

        // Set view/projection for cloth shader
        clothShader->setMat4("projection", projection);
        clothShader->setMat4("view", view);
        clothShader->setMat4("model", model);

        // Update to the wind direction periodically
        windTimer += deltaTime;
        if (windTimer >= windChangeInterval) {
            windDirection = getRandomWindDirection(); //Randomizes the wind direction
            windTimer = 0.0f; //Timer reset
        }

        for (auto& particle : particles) {
            if (toggle_wind) {
                //Generates a random wind strength factor between -windOffsetSpeed and windOffsetSpeed
                float noise = windScale + ((static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f) * windOffsetSpeed;
                glm::vec3 wind = windDirection * noise;
                particle.applyForce(wind);
            }

            particle.applyForce(glm::vec3(0.0f, gravity, 0.0f)); // Apply gravity
            particle.update(deltaTime);
            particle.render(clothShader->ID, view, projection);
        }

        // Update and render springs
        for (auto& spring : springs) {
            spring.update();
            spring.render(clothShader->ID, view, projection);
        }

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
}


// Cleanup resources
void Application::Cleanup()
{
    // Cleanup ImGui
    imgui_manager.Cleanup();

    // // delete our shader
    delete importedModelShader;
    delete clothShader;
    // // delete our model
    delete ourModel;

    glfwDestroyWindow(window);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glfwTerminate();
}
