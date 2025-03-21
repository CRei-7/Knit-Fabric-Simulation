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
Application::Application() : window(nullptr), glsl_version("#version 330"), cKeyPressed(false), tKeyPressed(false), oKeyPressed(false), fKeyPressed(false) {}

// Destructor
Application::~Application() {}

// Initialize GLFW, OpenGL, and ImGui
bool Application::Init()
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    // Setup OpenGL context version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    window = glfwCreateWindow(1280, 720, "Fabric Simulation", nullptr, nullptr);
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
    ShowFur = true;
    StartSimulation = false;
    ShowParticle = false;
    ShowSpring = false;

    k = 100.0f; // Structural Spring constant
    shearK = k * 0.3f; // Shear spring constant
    bendK = k * 0.1f; // Lower stiffness for bend springs

    StaticFrictionCoefficient = 0.6f;
    KineticFrictionCoefficient = 0.4f;

    lightPos = glm::vec3(0.0f, 1.0f, 1.0f);
    lightColor = { 1.0f, 1.0f, 1.0f };  // White light

    filename = "./fabric_images/real_madrid.jpg";

    glfwSwapInterval(1); // Enable vsync

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD\n";
        return false;
    }

    // Initialize ImGui
    imgui_manager.Init(window, glsl_version);
    std::cout << "ImGui initialized" << std::endl;
    // Assuming imguiManager is your ImGuiManager instance
    imgui_manager.SetToggleWind(&toggle_wind);
    imgui_manager.SetToggleCloth(&toggleClothOrientation, &clothNeedsReset);
    imgui_manager.SetGravity(&gravity);
    imgui_manager.SetFur(&ShowFur);
    imgui_manager.SetLightColor(&lightColor);
    imgui_manager.SetSimulation(&StartSimulation);
    imgui_manager.ParticleShow(&ShowParticle);
    imgui_manager.SpringShow(&ShowSpring);
    imgui_manager.SetLightPosition(&lightPos);

    imgui_manager.SetK(&k);
    imgui_manager.SetShearK(&shearK);
    imgui_manager.SetBendK(&bendK);

    imgui_manager.SetCube(&SelectCube);
    imgui_manager.SetSphere(&SelectSphere);

    imgui_manager.SetTexturePath(&filename);

    std::cout << "GPU: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    // For model loading
    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    // stbi_set_flip_vertically_on_load(true);

    glEnable(GL_DEPTH_TEST);
    shader = new Shader("VertShader.vert", "FragShader.frag");
    // importedModelShader = new Shader("../shaders/modelVertex.vert", "../shaders/modelFragment.frag");
    // std::cout << "Loading models" << std::endl;
       // // load models
    // ourModel = new Model("../models/backpack/backpack.obj");

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

    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    firstMouse = true;
    yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
    pitch = 0.0f;
    lastX = display_w / 2.0;
    lastY = display_h / 2.0;
    fov = 45.0f;

    deltaTime = 0.01f;
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

    // Toggles Fur with key 'F'
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        if (!fKeyPressed) {
            ShowFur = !ShowFur;
            fKeyPressed = true;

        }
    }

    // Resets the flag when the key is released
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE) {
        fKeyPressed = false;
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

void Application::generateFurStrands(const std::vector<Particle>& particles, int column, int row) {
    float furDensity = 0.15f; // Distance between fur base points
    int furLayers = 10; // Number of layers for the fur
    float furLength = 0.025f; // Length of each fur strand

    furVertices.clear();
    furIndices.clear();
    furTexCoords.clear();
    furLengths.clear();

    // Precompute random offsets for fur strands
    std::vector<glm::vec3> randomOffsets;
    randomOffsets.reserve(indices.size() / 3 * furLayers);
    for (int i = 0; i < indices.size() / 3 * furLayers; ++i) {
        float randomOffsetX = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.01f;
        float randomOffsetY = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.01f;
        randomOffsets.emplace_back(randomOffsetX, randomOffsetY, 0.0f);
    }

    // Reserve memory for furVertices and furIndices
    int maxFurVertices = indices.size() / 3 * furLayers * (1 + 1 / furDensity) * (1 + 1 / furDensity);
    int maxFurIndices = maxFurVertices * 2;
    furVertices.reserve(maxFurVertices);
    furIndices.reserve(maxFurIndices);
    furTexCoords.reserve(maxFurVertices);
    furLengths.reserve(maxFurVertices);

    // Iterate over each face (triangle) in the cloth mesh
    for (int i = 0; i < indices.size(); i += 3) {
        // Get the three vertices of the triangle
        glm::vec3 v0 = vertices[indices[i]];
        glm::vec3 v1 = vertices[indices[i + 1]];
        glm::vec3 v2 = vertices[indices[i + 2]];

        glm::vec2 t0 = texCoords[indices[i]];
        glm::vec2 t1 = texCoords[indices[i+1]];
        glm::vec2 t2 = texCoords[indices[i+2]];

        // Calculate the normal of the triangle
        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

        // Generate interpolated points across the triangle
        for (float u = 0.0f; u <= 1.0f; u += furDensity) {
            for (float v = 0.0f; v <= ( 1.0f - u ); v += furDensity) {
                // Barycentric coordinates
                const float w = 1.0f - u - v;
                
                // Barycentric interpolation to get the base point
                glm::vec3 basePoint = v0 * w + v1 * u + v2 * v;

                // Texture coordinate interpolation
                glm::vec2 baseTexCoord = t0 * w + t1 * u + t2 * v;

                // Generate fur strands along the normal
                for (int layer = 0; layer < furLayers; ++layer) {
                    float t = static_cast<float>(layer) / furLayers;
                    glm::vec3 furPos = basePoint + normal * furLength * t;

                    // Add precomputed randomness to the fur direction
                    furPos += randomOffsets[(i / 3) * furLayers + layer] * t;

                    furVertices.push_back(furPos);
                    furTexCoords.push_back(baseTexCoord);
                    furLengths.push_back(t); // Stores normalized fur length

                    // Connect fur strands to the base point
                    if (layer > 0) {
                        furIndices.push_back(furVertices.size() - 2);
                        furIndices.push_back(furVertices.size() - 1);
                    }
                }
            }
        }
    }
}

void Application::setupCloth() {
    // Grid parameters
    int column = 20; // Number of columns
    int row = 20;    // Number of rows
    float disX = 0.05f; // Distance between particles in x direction
    float disY = 0.05f; // Distance between particles in y direction
    float initialY = 0.3f; // Y-coordinate for the top pinned particle
    glm::vec3 Offset(-0.5f, 0.0f, 0.0f); // Offset for initial position

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
                particles.emplace_back(glm::vec3(xPos, 0.15f, zPos), staticParticle);
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

    // Bend springs for better cloth draping and resistance to bending
    for (int i = 0; i < column; ++i) {
        for (int j = 0; j < row; ++j) {
            // Horizontal bend springs (skip 1 particle)
            if (i < column - 2) {
                springs.emplace_back(bendK, disX * 2,
                    &particles[i * row + j],
                    &particles[(i + 2) * row + j]);
            }

            // Vertical bend springs (skip 1 particle)
            if (j < row - 2) {
                springs.emplace_back(bendK, disY * 2,
                    &particles[i * row + j],
                    &particles[i * row + (j + 2)]);
            }
        }
    }

    setupClothMesh(particles, column, row);
}


void Application::setupClothMesh(const std::vector<Particle>& particles, int column, int row) {
    vertices.clear();
    indices.clear();
    normals.clear();
    texCoords.clear();
    furVertices.clear();
    furIndices.clear();
    furTexCoords.clear();
    furLengths.clear();

    // Store particle positions as vertices
    vertices.reserve(particles.size());
    texCoords.reserve(particles.size()); // Reserve space for texture coordinates
    for (int i = 0; i < column; ++i) {
        for (int j = 0; j < row; ++j) {
            int idx = i * row + j;
            vertices.push_back(particles[idx].getPosition());

            // Calculate texture coordinates based on grid position
            float u = static_cast<float>(i) / (column - 1);
            float v = static_cast<float>(j) / (row - 1);
            texCoords.push_back(glm::vec2(u, v));
        }
    }

    // Generate the indices for triangles between neighboring particles
    indices.reserve((column - 1) * (row - 1) * 6);
    for (int i = 0; i < column - 1; ++i) {
        for (int j = 0; j < row - 1; ++j) {
            // Top-left triangle (CCW)
            indices.push_back(i * row + j);
            indices.push_back(i * row + (j + 1)); // Changed
            indices.push_back((i + 1) * row + j); // Changed

            // Bottom-right triangle (CCW)
            indices.push_back((i + 1) * row + (j + 1));
            indices.push_back((i + 1) * row + j); // Changed
            indices.push_back(i * row + (j + 1)); // Changed
        }
    }

    calculateNormals();

    // Generate BVH for the cloth
    clothBVH = new BVH(particles, indices);

    if (ShowFur) {
        // Setup for fur
        generateFurStrands(particles, column, row);
    }

    // Generate VAO, VBO, and EBO for the mesh
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &normalVBO);
    glGenBuffers(1, &texCoordVBO);
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

    glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO);
    glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(glm::vec2), &texCoords[0], GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(2);

    // Bind EBO for indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

    if (ShowFur) {
        // Generate VAO, VBO, and EBO for fur
        glGenVertexArrays(1, &furVAO);
        glGenBuffers(1, &furVBO);
        glGenBuffers(1, &furTexCoordVBO);
        glGenBuffers(1, &furLengthVBO);
        glGenBuffers(1, &furEBO);

        glBindVertexArray(furVAO);

        // Bind VBO for fur vertex positions
        glBindBuffer(GL_ARRAY_BUFFER, furVBO);
        glBufferData(GL_ARRAY_BUFFER, furVertices.size() * sizeof(glm::vec3), &furVertices[0], GL_DYNAMIC_DRAW);

        // Set vertex attributes for position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glEnableVertexAttribArray(0);

        // Calculate normals for fur (the same normals as the base mesh)
        std::vector<glm::vec3> furNormals;
        furNormals.resize(furVertices.size(), glm::vec3(0.0f, 1.0f, 0.0f)); // Default normal

        // Bind VBO for fur normals (using cloth normals)
        glGenBuffers(1, &furNormalVBO);
        glBindBuffer(GL_ARRAY_BUFFER, furNormalVBO);
        glBufferData(GL_ARRAY_BUFFER, furNormals.size() * sizeof(glm::vec3), &furNormals[0], GL_DYNAMIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glEnableVertexAttribArray(1);

        // Binds VBO for Texture Coordinates
        glBindBuffer(GL_ARRAY_BUFFER, furTexCoordVBO);
        glBufferData(GL_ARRAY_BUFFER, furTexCoords.size() * sizeof(glm::vec2), furTexCoords.data(), GL_DYNAMIC_DRAW);

        // Sets vertex attributes for Texture Coordinates
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
        glEnableVertexAttribArray(2);

        // Bind VBO for fur length attribute
        glBindBuffer(GL_ARRAY_BUFFER, furLengthVBO);
        glBufferData(GL_ARRAY_BUFFER, furLengths.size() * sizeof(float), furLengths.data(), GL_DYNAMIC_DRAW);

        // Set vertex attribute for fur length
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
        glEnableVertexAttribArray(3);

        // Bind EBO for fur indices
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, furEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, furIndices.size() * sizeof(GLuint), &furIndices[0], GL_STATIC_DRAW);
    }
    else {
        furVAO = furVBO = furEBO = 0;
    }

    TextureSetup();

    glBindVertexArray(0);
}

void Application::TextureSetup() {
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width, height, nrChannels;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        std::cout << "Data has beenloaded" << endl;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    //glBindVertexArray(0);
}

void Application::calculateNormals() {
    normals.resize(vertices.size(), glm::vec3(0.0f));
#pragma omp parallel for
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

    // Normalize normals
#pragma omp parallel for
    for (auto& normal : normals) {
        normal = glm::normalize(normal);
    }
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

    calculateNormals();

    // Update normals buffer if needed (if particles have moved significantly)
    glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, normals.size() * sizeof(glm::vec3), &normals[0]);

    if (ShowFur) {
        // Precompute random offsets for fur strands
        static std::vector<glm::vec3> randomOffsets;
        if (randomOffsets.empty()) {
            randomOffsets.reserve(indices.size() / 3 * 10); // 10 layers of fur
            for (int i = 0; i < indices.size() / 3 * 10; ++i) {
                float randomOffsetX = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.01f;
                float randomOffsetY = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.01f;
                randomOffsets.emplace_back(randomOffsetX, randomOffsetY, 0.0f);
            }
        }

        // Generate fur vertices and indices
        furVertices.clear();
        furIndices.clear();
        furTexCoords.clear();
        furLengths.clear();

        float furDensity = 0.15f; // Distance between fur base points
        int furLayers = 10; // Number of layers for the fur
        float furLength = 0.025f; // Length of each fur strand

#pragma omp parallel for
        for (int i = 0; i < indices.size(); i += 3) {
            // Get the three vertices of the triangle
            glm::vec3 v0 = vertices[indices[i]];
            glm::vec3 v1 = vertices[indices[i + 1]];
            glm::vec3 v2 = vertices[indices[i + 2]];

            glm::vec2 t0 = texCoords[indices[i]];
            glm::vec2 t1 = texCoords[indices[i + 1]];
            glm::vec2 t2 = texCoords[indices[i + 2]];

            // Calculate the normal of the triangle
            glm::vec3 edge1 = v1 - v0;
            glm::vec3 edge2 = v2 - v0;
            glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

            // Generate interpolated points across the triangle
            for (float u = 0.0f; u <= 1.0f; u += furDensity) {
                for (float v = 0.0f; v <= (1.0f - u); v += furDensity) {
                    // Barycentric coordinates
                    const float w = 1.0f - u - v;

                    // Barycentric interpolation to get the base point
                    glm::vec3 basePoint = v0 * w + v1 * u + v2 * v;

                    // Texture coordinate interpolation
                    glm::vec2 baseTexCoord = t0 * w + t1 * u + t2 * v;

                    // Generate fur strands along the normal
                    int baseIndex = furVertices.size();

                    // Generate fur strands along the normal
                    for (int layer = 0; layer < furLayers; ++layer) {
                        float t = static_cast<float>(layer) / furLayers;
                        glm::vec3 furPos = basePoint + normal * furLength * t;

                        // Add precomputed randomness to the fur direction
                        furPos += randomOffsets[(i / 3) * furLayers + layer] * t;

#pragma omp critical
                        {
                            furVertices.push_back(furPos);
                            furTexCoords.push_back(baseTexCoord);
                            furLengths.push_back(t);

                            // Connect fur strands to the base point
                            if (layer > 0) {
                                furIndices.push_back(furVertices.size() - 2);
                                furIndices.push_back(furVertices.size() - 1);
                            }
                        }
                    }
                }
            }
        }

        // Calculate fur normals (same as cloth triangle normals)
        std::vector<glm::vec3> furNormals;
        furNormals.resize(furVertices.size(), glm::vec3(0.0f, 0.0f, 0.0f));

        for (size_t i = 0; i < furIndices.size(); i += 2) {
            if (i + 1 < furIndices.size()) {
                glm::vec3 v0 = furVertices[furIndices[i]];
                glm::vec3 v1 = furVertices[furIndices[i + 1]];
                glm::vec3 dir = glm::normalize(v1 - v0);
                // Perpendicular to the strand direction (approximate normal)
                glm::vec3 normal = glm::normalize(glm::vec3(-dir.y, dir.x, dir.z));

                furNormals[furIndices[i]] = normal;
                furNormals[furIndices[i + 1]] = normal;
            }
        }

        // Update fur vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, furVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, furVertices.size() * sizeof(glm::vec3), &furVertices[0]);

        glBindBuffer(GL_ARRAY_BUFFER, furTexCoordVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, furTexCoords.size() * sizeof(glm::vec2), furTexCoords.data());

        glBindBuffer(GL_ARRAY_BUFFER, furNormalVBO);
        glBufferData(GL_ARRAY_BUFFER, furNormals.size() * sizeof(glm::vec3), furNormals.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, furLengthVBO);
        glBufferData(GL_ARRAY_BUFFER, furLengths.size() * sizeof(float), furLengths.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, furEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, furIndices.size() * sizeof(GLuint), &furIndices[0], GL_DYNAMIC_DRAW);
    }

    // Set light properties (light position, view position, and color)
    GLuint lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
    glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));

    glm::vec3 viewPos = cameraPos;  // Camera/view position
    GLuint viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
    glUniform3fv(viewPosLoc, 1, glm::value_ptr(viewPos));

    // Activate and Bind Texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    GLuint textureLoc = glGetUniformLocation(shaderProgram, "clothTexture");
    glUniform1i(textureLoc, 0); //Sets texture unit 0

    // Shadow parameters
    GLuint furShadowStrengthLoc = glGetUniformLocation(shaderProgram, "furShadowStrength");
    glUniform1f(furShadowStrengthLoc, 0.6f); // Adjust for stronger/weaker shadows

    GLuint furAlphaLoc = glGetUniformLocation(shaderProgram, "furAlpha");
    glUniform1f(furAlphaLoc, 0.8f); // Controls transparency gradient of fur

    // Set isFur uniform to 0 for cloth
    GLuint isFurLoc = glGetUniformLocation(shaderProgram, "isFur");
    glUniform1i(isFurLoc, 0);

    // Bind VAO for cloth mesh
    glBindVertexArray(VAO);

    // Set useTexture uniform to tell shader to use texture
    GLuint useTextureLoc = glGetUniformLocation(shaderProgram, "useTexture");
    glUniform1i(useTextureLoc, 1); // 1 = use texture

    GLint loc = glGetUniformLocation(shaderProgram, "fabricType");
    glUniform1i(loc, imgui_manager.GetFabricTypeUniform());

    // This is to invert the normal if the associated face is the back face.
    GLuint isBackFaceLoc = glGetUniformLocation(shaderProgram, "isBackFace");
    glUniform1i(isBackFaceLoc, 1); // 1 = backface

    glFrontFace(GL_CW);
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
    glm::vec3 frontColor(1.0f, 1.0f, 1.0f);  // Color for the front face
    glUniform3fv(colorLoc, 1, glm::value_ptr(frontColor));
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    glm::vec3 backColor(1.0f, 1.0f, 1.0f);  // Color for the back face
    
    glUniform1i(isBackFaceLoc, 0);
    glCullFace(GL_FRONT);  // Cull the front faces, render back faces

    // Turn off texturing for back face if desired
    glUniform1i(useTextureLoc, 1); // 0 = don't use texture

    if (!collidingIndices.empty()) {
        glUniform3fv(colorLoc, 1, glm::value_ptr(backColor));
        glDrawElements(GL_TRIANGLES, collidingIndices.size(), GL_UNSIGNED_INT, 0);
    }

    glUniform3fv(colorLoc, 1, glm::value_ptr(backColor));
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    glFrontFace(GL_CCW);
    // Render fur with a different color
    if (ShowFur && !furVertices.empty() && !furIndices.empty()) {
        glBindVertexArray(furVAO);
        //glm::vec3 furColor(1.0f, 0.0f, 0.0f);  // Red fur

        // Set fur to not use texture
        //glUniform1i(useTextureLoc, 0);
        //glUniform3fv(colorLoc, 1, glm::value_ptr(furColor));

        //glDrawElements(GL_LINES, furIndices.size(), GL_UNSIGNED_INT, 0);

        // Enable alpha blending for fur transparency
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Set isFur uniform to 1 for fur rendering
        glUniform1i(isFurLoc, 1);

        // Uses same texture as cloth
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(textureLoc, 0);

        // Enable texture sampling for fur
        glUniform1i(useTextureLoc, 1); // 1 = use texture

        // Set color for fur (tinted slightly)
        glm::vec3 furColor(0.95f, 0.95f, 0.95f); // Slightly off-white
        glUniform3fv(colorLoc, 1, glm::value_ptr(furColor));


        // Set uniform for color (white to allow full texture color)
        //glUniform3fv(colorLoc, 1, glm::value_ptr(glm::vec3(1.0f)));

        // Disable back face culling for fur (render both sides)
        glDisable(GL_CULL_FACE);

        glDrawElements(GL_LINES, furIndices.size(), GL_UNSIGNED_INT, 0);

        // Re-enable face culling
        glEnable(GL_CULL_FACE);
        glDisable(GL_BLEND);
    }

    glBindVertexArray(0);
    glUseProgram(0);
}

// Main rendering loop
void Application::MainLoop()
{
    setupCloth();
    //Object Cube;
    //Cube.SetupCube(0.4f, glm::vec3(0.0f, -0.2f, 0.5f));

    //Object Sphere;
    //Sphere.SetupSphere(0.3f, glm::vec3(0.0, -0.2, 0.5));

    while (!glfwWindowShouldClose(window))
    {

        if (SelectCube) {
            if (!currentObject || currentObject->objectType != ObjectType::Cube) {
                currentObject = std::make_unique<Object>();
                currentObject->SetupCube(0.4f, glm::vec3(0.0f, -0.2f, 0.5f));
            }
        }
        else if (SelectSphere) {
            if (!currentObject || currentObject->objectType != ObjectType::Sphere) {
                currentObject = std::make_unique<Object>();
                currentObject->SetupSphere(0.3f, glm::vec3(0.0f, -0.2f, 0.5f));
            }
        }
        else {
            currentObject.reset(); // Destroy if no selection
        }

        bool should_close = false;
        //for variable speed of movement
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame; //=0.016
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
            //imgui_manager.RenderWireframeToggle();
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

        // use imported model shaders
        // importedModelShader->use();

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

        // importedModelShader->setMat4("projection", projection);
        // importedModelShader->setMat4("view", view);
        // importedModelShader->setMat4("model", model);

        // ourModel->Draw(*importedModelShader, imgui_manager.wireframeMode);

        if (StartSimulation) {
            // Update to the wind direction periodically
            windTimer += deltaTime;
            if (windTimer >= windChangeInterval) {
                windDirection = getRandomWindDirection(); //Randomizes the wind direction
                windTimer = 0.0f; //Timer reset
            }


            clothBVH->refit();
            //std::cout << " Without BVH: " << NewCollision::collisionChecks << "\n";
            //std::cout << " - With BVH: " << NewCollision::bvhCollisionChecks << "\n";
            if(currentObject)
                NewCollision::resolveCollision(particles, clothBVH, indices, *currentObject, deltaTime, collidingIndices, StaticFrictionCoefficient, KineticFrictionCoefficient);
            //NewCollision::resolveCollisionWithOutBVH(particles, indices, Sphere, deltaTime, collidingIndices);

            for (auto& particle : particles) {
                //resolveCollision(particle, Cube   );
                //for (int i = 0; i < 20; ++i) {
                    // Collision::resolveCollision(particle, Sphere, deltaTime);
                //}

                Collision::resolveSelfCollision(particle, particles); // Check self-collision

                if (toggle_wind) {
                    //Generates a random wind strength factor between -windOffsetSpeed and windOffsetSpeed
                    float noise = windScale + ((static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f) * windOffsetSpeed;
                    // Add turbulence and noise to wind
                    float turbulence = sin(windTimer * 2.0f) * 0.1f;
                    glm::vec3 windVariation = glm::vec3(
                        turbulence * sin(particle.getPosition().x),
                        turbulence * cos(particle.getPosition().y),
                        turbulence * sin(particle.getPosition().z)
                    );
                    glm::vec3 wind = windDirection * noise + windVariation;
                    particle.applyForce(wind);
                }

                particle.applyForce(glm::vec3(0.0f, gravity, 0.0f)); // Apply gravity
                particle.update(deltaTime);
                if(ShowParticle)
                    particle.render(shader->shaderProgram, view, projection);
            }

            // Update and render springs
            for (auto& spring : springs) {
                spring.update();
                if(ShowSpring)
                    spring.render(shader->shaderProgram, model, view, projection);
            }
        }
        glm::vec3 color = glm::vec3(1.0f, 0.0f, 0.0f);

        if (!StartSimulation) {
            for (auto& particle : particles) {
                if (ShowParticle)
                    particle.render(shader->shaderProgram, view, projection);
            }

            for (auto& spring : springs) {
                if (ShowSpring)
                    spring.render(shader->shaderProgram, model, view, projection);
            }
        }

        //Cube.render(shader->shaderProgram, view, projection, lightPos, cameraPos, color);
        if(currentObject)
            currentObject->render(shader->shaderProgram, view, projection, lightPos, cameraPos, color);

        renderClothMesh(shader->shaderProgram, particles, view, projection);
        // table->Draw(shader->shaderProgram, glm::mat4(1.0f), view, projection);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (filename != imgui_manager.GetTexturePath()) {
            glDeleteTextures(1, &texture);
            filename = imgui_manager.GetTexturePath();
            TextureSetup();
        }

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
    delete clothBVH;
    glfwDestroyWindow(window);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &normalVBO);
    glDeleteBuffers(1, &texCoordVBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &furVAO);
    glDeleteBuffers(1, &furVBO);
    glDeleteBuffers(1, &furEBO);
    glDeleteTextures(1, &texture);
    glfwTerminate();
}