#include "Application.h"
#include <algorithm>
#include <stdio.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Error callback function
static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// Constructor
Application::Application() : window(nullptr), glsl_version("#version 330"), cKeyPressed(false), tKeyPressed(false), oKeyPressed(false) {}

// Destructor
Application::~Application() {}

// Initialize GLFW, OpenGL, and ImGui
bool Application::Init()
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()){
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    // Setup OpenGL context version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    window = glfwCreateWindow(1280, 720, "Dear ImGui", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
                glfwTerminate();
                return false;
    }
    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);
    glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xposIn, double yposIn) {
        static_cast<Application*>(glfwGetWindowUserPointer(window))->mouse_callback(window, xposIn, yposIn);
    });
    glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
        static_cast<Application*>(glfwGetWindowUserPointer(window))->scroll_callback(window, xoffset, yoffset);
    });
    //glfwSetCursorPosCallback(window, mouse_callback);
    //glfwSetScrollCallback(window, scroll_callback);

    //Tells GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    cursorVisible = false;
    toggle_wind = false;
    toggleClothOrientation = true;
    clothNeedsReset = false;

    glfwSwapInterval(1); // Enable vsync

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD\n";
        return false;
    }

    // Initialize ImGui
    imgui_manager.Init(window, glsl_version);
    std::cout << "ImGui initialized" << std::endl;

    shader = new Shader("../shaders/VertShader.vert", "../shaders/FragShader.frag");

    SetupOpenGL();

    return true;
}

void Application::SetupOpenGL() {
    // std::vector<Vertex> vertices = {
    //     // Table top vertices (y = 0.8 - 0.05 = 0.75)
    //     {-0.5f,  0.75f,  0.3f},  // 0 front left
    //     { 0.5f,  0.75f,  0.3f},  // 1 front right
    //     { 0.5f,  0.75f, -0.3f},  // 2 back right
    //     {-0.5f,  0.75f, -0.3f},  // 3 back left
    //     {-0.5f,  0.65f,  0.3f},  // 4 front left bottom
    //     { 0.5f,  0.65f,  0.3f},  // 5 front right bottom
    //     { 0.5f,  0.65f, -0.3f},  // 6 back right bottom
    //     {-0.5f,  0.65f, -0.3f},  // 7 back left bottom

    //     // Front left leg (y -= 0.05)
    //     {-0.45f, 0.65f,  0.25f}, // 8  top
    //     {-0.35f, 0.65f,  0.25f}, // 9  top
    //     {-0.35f, 0.65f,  0.15f}, // 10 top
    //     {-0.45f, 0.65f,  0.15f}, // 11 top
    //     {-0.45f, -0.05f,  0.25f}, // 12 bottom
    //     {-0.35f, -0.05f,  0.25f}, // 13 bottom
    //     {-0.35f, -0.05f,  0.15f}, // 14 bottom
    //     {-0.45f, -0.05f,  0.15f}, // 15 bottom

    //     // Front right leg (y -= 0.05)
    //     { 0.35f, 0.65f,  0.25f}, // 16 top
    //     { 0.45f, 0.65f,  0.25f}, // 17 top
    //     { 0.45f, 0.65f,  0.15f}, // 18 top
    //     { 0.35f, 0.65f,  0.15f}, // 19 top
    //     { 0.35f, -0.05f,  0.25f}, // 20 bottom
    //     { 0.45f, -0.05f,  0.25f}, // 21 bottom
    //     { 0.45f, -0.05f,  0.15f}, // 22 bottom
    //     { 0.35f, -0.05f,  0.15f}, // 23 bottom

    //     // Back left leg (y -= 0.05)
    //     {-0.45f, 0.65f, -0.15f}, // 24 top
    //     {-0.35f, 0.65f, -0.15f}, // 25 top
    //     {-0.35f, 0.65f, -0.25f}, // 26 top
    //     {-0.45f, 0.65f, -0.25f}, // 27 top
    //     {-0.45f, -0.05f, -0.15f}, // 28 bottom
    //     {-0.35f, -0.05f, -0.15f}, // 29 bottom
    //     {-0.35f, -0.05f, -0.25f}, // 30 bottom
    //     {-0.45f, -0.05f, -0.25f}, // 31 bottom

    //     // Back right leg (y -= 0.05)
    //     { 0.35f, 0.65f, -0.15f}, // 32 top
    //     { 0.45f, 0.65f, -0.15f}, // 33 top
    //     { 0.45f, 0.65f, -0.25f}, // 34 top
    //     { 0.35f, 0.65f, -0.25f}, // 35 top
    //     { 0.35f, -0.05f, -0.15f}, // 36 bottom
    //     { 0.45f, -0.05f, -0.15f}, // 37 bottom
    //     { 0.45f, -0.05f, -0.25f}, // 38 bottom
    //     { 0.35f, -0.05f, -0.25f}  // 39 bottom
    // };


    // // Indices for the tabletop (2 triangles)
    //     std::vector<unsigned int> indices = {
    //         // Table top - top face
    //                0, 1, 2,    0, 2, 3,
    //                // Table top - bottom face
    //                4, 5, 6,    4, 6, 7,
    //                // Table top - front face
    //                0, 1, 5,    0, 5, 4,
    //                // Table top - back face
    //                2, 3, 7,    2, 7, 6,
    //                // Table top - left face
    //                0, 3, 7,    0, 7, 4,
    //                // Table top - right face
    //                1, 2, 6,    1, 6, 5,

    //                // Front left leg
    //                8, 9, 13,   8, 13, 12,    // front
    //                9, 10, 14,  9, 14, 13,    // right
    //                10, 11, 15, 10, 15, 14,   // back
    //                11, 8, 12,  11, 12, 15,   // left

    //                // Front right leg
    //                16, 17, 21, 16, 21, 20,   // front
    //                17, 18, 22, 17, 22, 21,   // right
    //                18, 19, 23, 18, 23, 22,   // back
    //                19, 16, 20, 19, 20, 23,   // left

    //                // Back left leg
    //                24, 25, 29, 24, 29, 28,   // front
    //                25, 26, 30, 25, 30, 29,   // right
    //                26, 27, 31, 26, 31, 30,   // back
    //                27, 24, 28, 27, 28, 31,   // left

    //                // Back right leg
    //                32, 33, 37, 32, 37, 36,   // front
    //                33, 34, 38, 33, 38, 37,   // right
    //                34, 35, 39, 34, 39, 38,   // back
    //                35, 32, 36, 35, 36, 39    // left
    //     };

    // table = new Mesh(vertices, indices);

    glEnable(GL_DEPTH_TEST);//for depth testing
    glEnable(GL_CULL_FACE);//for shading two sides of the mesh with different colors

    //camera
    cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
    cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    lightPos = glm::vec3(0.0f, 1.0f, 0.0f);

    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    firstMouse = true;
    yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
    pitch = 0.0f;
    lastX = display_w / 2.0;
    lastY = display_h / 2.0;
    fov = 45.0f;

    deltaTime = 0.0f;
    lastFrame = 0.0f;
}

void Application::processInput(GLFWwindow* window) {
    //Esc for closing the window
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    /*double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    // Check if the left mouse button is pressed
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        // Get window size
        int width, height;
        glfwGetWindowSize(window, &width, &height);

        // Convert to OpenGL normalized device coordinates (NDC)
        float xNDC = (2.0f * xpos) / width - 1.0f;
        float yNDC = 1.0f - (2.0f * ypos) / height;

        // Print the OpenGL coordinates to the console
        std::cout << "Mouse Click at OpenGL Coordinates: (" << xNDC << ", " << yNDC << ")\n";
    }*/

    //For keyboard movement
    float cameraSpeed = 2.5f * deltaTime; //speed of camera movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

    // Toggles cursor visibility with 'C'
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        if (!cKeyPressed) {
            cursorVisible = !cursorVisible;

            if (cursorVisible) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // Show cursor
            }
            else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Hide cursor
                firstMouse = true;
            }

            cKeyPressed = true;
        }
    }

    // Resets the flag when the key is released
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE) {
        cKeyPressed = false;
    }

    // Toggles wind with key 'T'
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
        if (!tKeyPressed) {
            toggle_wind = !toggle_wind;
            tKeyPressed = true;
        }
    }

    // Resets the flag when the key is released
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE) {
        tKeyPressed = false;
    }

    // Toggles wind with key 'O'
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
        if (!oKeyPressed) {
            toggleClothOrientation = !toggleClothOrientation;
            clothNeedsReset = true;
            oKeyPressed = true;

        }
    }

    // Resets the flag when the key is released
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_RELEASE) {
        oKeyPressed = false;
    }
}

glm::vec3 getRandomWindDirection() {
    // Creates a random number generator
    std::random_device rd;
    std::mt19937 gen(rd());

    // Uniform distribution for azimuthal angle (0 to 2*PI)
    std::uniform_real_distribution<> azimuthalDist(0.0, 2.0 * glm::pi<float>());

    // Uniform distribution for z component of the vector (cosine of the polar angle)
    std::uniform_real_distribution<> zDist(-1.0, 1.0);

    // Generates random azimuthal angle (phi)
    float azimuthalAngle = static_cast<float>(azimuthalDist(gen));

    // Generates random z component
    float z = static_cast<float>(zDist(gen));

    // Calculates the corresponding x and y components using azimuthal angle and radius (r = sqrt(1 - z^2))
    float radius = std::sqrt(1.0f - z * z);
    float x = radius * std::cos(azimuthalAngle);
    float y = radius * std::sin(azimuthalAngle);

    // Returns the random normalized vector
    return glm::vec3(x, y, z);
}

void Application::setupCloth() {
    // Grid parameters
    int column = 20; // Number of columns
    int row = 20;    // Number of rows
    float disX = 0.05f; // Distance between particles in x direction
    float disY = 0.05f; // Distance between particles in y direction
    float initialY = 0.3f; // Y-coordinate for the top pinned particle
    glm::vec3 Offset(-0.5f, 0.0f, 0.0f); // Offset for initial position
    float k = 50.0f; // Structural Spring constant
    float shearK = 10.5f; // Shear spring constant

    gravity = -0.05f;

    // Wind parameters
    windDirection = getRandomWindDirection(); // Initial random wind direction
    windScale = 0.02f; // Base wind strength
    windOffsetSpeed = 0.1f; // Variability in wind strength
    windChangeInterval = 0.5f; // Time in seconds to change wind direction
    windTimer = 0.0f; // Timer for wind direction change

    particles.reserve(column * row); // Reserve space to avoid multiple allocations

    if (toggleClothOrientation) {
        // Initialize particles as (x, y, 0)
        for (int i = 0; i < column; ++i) {
            for (int j = 0; j < row; ++j) {
                float xPos = i * disX + Offset.x;
                float yPos = initialY - j * disY; // Starts from initialY and moves downward
                bool staticParticle = j == 0; // Top row particles are static
                particles.emplace_back(glm::vec3(xPos, yPos, 0.0f), staticParticle);
            }
        }
    }
    else {
        // Initialize particles as (x, 0, z)
        for (int i = 0; i < column; ++i) {
            for (int j = 0; j < row; ++j) {
                float xPos = i * disX + Offset.x;
                float zPos = j * disY; // Use disY for Z direction when orientation is different
                bool staticParticle = false; // j == 0; //First row static in this orientation
                particles.emplace_back(glm::vec3(xPos, 0.22f, zPos), staticParticle);
            }
        }
    }

    // Initialize springs (same for both orientations)
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

    setupClothMesh(particles, column, row);
}


void Application::setupClothMesh(const std::vector<Particle>& particles, int column, int row) {
    vertices.clear();
    indices.clear();
    normals.clear();

    // Store particle positions as vertices
    for (const auto& particle : particles) {
        vertices.push_back(particle.getPosition());
        normals.push_back(glm::vec3(0.0f));  // Initialize normals
    }

    // Generate the indices for triangles between neighboring particles
    for (int i = 0; i < column - 1; ++i) {
        for (int j = 0; j < row - 1; ++j) {
            // Top-left triangle
            indices.push_back(i * row + j);
            indices.push_back((i + 1) * row + j);
            indices.push_back(i * row + (j + 1));

            // Bottom-right triangle
            indices.push_back((i + 1) * row + (j + 1));
            indices.push_back(i * row + (j + 1));
            indices.push_back((i + 1) * row + j);
        }
    }

    // Calculate normals
    for (int i = 0; i < indices.size(); i += 3) {
        glm::vec3 v0 = vertices[indices[i]];
        glm::vec3 v1 = vertices[indices[i + 1]];
        glm::vec3 v2 = vertices[indices[i + 2]];

        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

        normals[indices[i]] += normal;
        normals[indices[i + 1]] += normal;
        normals[indices[i + 2]] += normal;
    }

    for (auto& normal : normals) {
        normal = glm::normalize(normal);
    }

    // Generate VAO, VBO, and EBO for the mesh
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &normalVBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // Bind VBO for vertex positions
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_DYNAMIC_DRAW);

    // Set vertex attributes for position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    // Bind VBO for normals
    glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(1);

    // Bind EBO for indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

    glBindVertexArray(0);
}


void Application::renderClothMesh(GLuint shaderProgram, const std::vector<Particle>& particles, const glm::mat4& view, const glm::mat4& projection) {
    glUseProgram(shaderProgram);

    // Set model, view, projection matrices
    glm::mat4 model = glm::mat4(1.0f);
    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Update particle positions in the vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    for (int i = 0; i < particles.size(); ++i) {
        vertices[i] = particles[i].getPosition();  // Update vertex positions
    }
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(glm::vec3), &vertices[0]);

    // Update normals buffer if needed (if particles have moved significantly)
    glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, normals.size() * sizeof(glm::vec3), &normals[0]);

    // Set light properties (light position, view position, and color)
    GLuint lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
    glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));


    glm::vec3 viewPos = cameraPos;  // Camera/view position
    GLuint viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
    glUniform3fv(viewPosLoc, 1, glm::value_ptr(viewPos));

    // Bind VAO
    glBindVertexArray(VAO);

    // Render front faces (side 1)
    glCullFace(GL_BACK);  // Cull the back faces, render front faces

    bool currentCollision = NewCollision::isColliding;
    // std::cout<<"currentCollision: "<<currentCollision<<std::endl;

    // // Prepare a vector to hold the result
    //     std::vector<GLuint> difference;

    //     // Compute the difference
    //     std::set_difference(
    //         indices.begin(), indices.end(),
    //         collidingIndices.begin(), collidingIndices.end(),
    //         std::back_inserter(difference)
    //     );
    GLint colorLoc = glGetUniformLocation(shaderProgram, "Color");
    glm::vec3 frontColor(1.0f, 0.0f, 0.0f);  // Color for the front face
    glUniform3fv(colorLoc, 1, glm::value_ptr(frontColor));
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    // Render back faces (side 2)
    glCullFace(GL_FRONT);  // Cull the front faces, render back faces
    if(!collidingIndices.empty()) {
        glm::vec3 frontColor(1.0f, 0.0f, 0.0f);  // Color for the front face
        glUniform3fv(colorLoc, 1, glm::value_ptr(frontColor));
        glDrawElements(GL_TRIANGLES, collidingIndices.size(), GL_UNSIGNED_INT, 0);
    }

    // glm::vec3 frontColor(1.0f, 0.0f, 0.0f);  // Color for the front face
    // glUniform3fv(colorLoc, 1, glm::value_ptr(frontColor));
    // glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    glm::vec3 backColor(1.0f, 1.0f, 0.0f);  // Color for the back face
    glUniform3fv(colorLoc, 1, glm::value_ptr(backColor));
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glUseProgram(0);
}


// Main rendering loop
void Application::MainLoop()
{
    setupCloth();
    Object Cube;
    Cube.SetupCube(0.4f, glm::vec3(0.0f, -0.2f, 0.5f));

    // Object Sphere;
    // Sphere.SetupSphere(0.3f, glm::vec3(0.0, -0.2, 0.5));

    while (!glfwWindowShouldClose(window))
    {
        bool should_close = false;
        //for variable speed of movement
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        // Check if cloth needs to be reset due to orientation toggle
        if (clothNeedsReset) {
            particles.clear();  // Clear previous particles
            springs.clear();    // Clear previous springs
            setupCloth();       // Re-setup the cloth with the new orientation
            clothNeedsReset = false;  // Reset the flag
        }

        glfwPollEvents();
        imgui_manager.BeginFrame();

        imgui_manager.SetupMenuBar(window, &should_close);
        if (should_close)
            glfwSetWindowShouldClose(window, true);

        imgui_manager.Render();
        imgui_manager.EndFrame();

        // Rendering
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        const ImVec4& clear_color = imgui_manager.GetClearColor();
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//clear the depth buffer at each iteration

        //activates the shader
        glUseProgram(shader->shaderProgram);

        glm::vec3 lightColor(1.0f, 1.0f, 1.0f);  // White light
        GLuint lightColorLoc = glGetUniformLocation(shader->shaderProgram, "lightColor");
        glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

        //for perspective transformation
        glm::mat4 model = glm::mat4(1.0f);//transformations we'd like to apply to all object's vertices to the global world space
        glm::mat4 view = glm::mat4(1.0f);//to set the camera location
        glm::mat4 projection = glm::mat4(1.0f);//for perspective projection

        //camera/view transformation
        view = glm::lookAt(
            cameraPos,
            cameraPos + cameraFront,
            cameraUp
        );//(position of camera, target position, up vector that is a vector pointing in positive y-direction)

        projection = glm::perspective(glm::radians(fov), (float)display_w / (float)display_h, 0.1f, 1000.0f);

        // Update to the wind direction periodically
        windTimer += deltaTime;
        if (windTimer >= windChangeInterval) {
            windDirection = getRandomWindDirection(); //Randomizes the wind direction
            windTimer = 0.0f; //Timer reset
        }

        NewCollision::resolveCollision(particles, indices, Cube, deltaTime, collidingIndices);

        for (auto& particle : particles) {
            //resolveCollision(particle, Cube   );
            for (int i = 0; i < 20; ++i) {
                // Collision::resolveCollision(particle, Sphere, deltaTime);
            }

            Collision::resolveSelfCollision(particle, particles); // Check self-collision

            if (toggle_wind) {
                //Generates a random wind strength factor between -windOffsetSpeed and windOffsetSpeed
                float noise = windScale + ((static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f) * windOffsetSpeed;
                glm::vec3 wind = windDirection * noise;
                particle.applyForce(wind);
            }

            particle.applyForce(glm::vec3(0.0f, gravity, 0.0f)); // Apply gravity
            particle.update(deltaTime);

            particle.render(shader->shaderProgram, view, projection);
        }

        // Update and render springs
        for (auto& spring : springs) {
            spring.update();
            spring.render(shader->shaderProgram, model, view, projection);
        }

        Cube.render(shader->shaderProgram, view, projection, lightPos, cameraPos);
        // Sphere.render(shader->shaderProgram, view, projection, lightPos, cameraPos);

        renderClothMesh(shader->shaderProgram, particles, view, projection);
        // table->Draw(shader->shaderProgram, glm::mat4(1.0f), view, projection);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
}

void Application::mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    if (cursorVisible) {
        return;
    }

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;//For mouse sensitivity
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    //To prevent screen from getting flipped
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

void Application::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 45.0f)
        fov = 45.0f;
}

// Cleanup resources
void Application::Cleanup()
{
    imgui_manager.Cleanup();
    glfwDestroyWindow(window);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glfwTerminate();
}
