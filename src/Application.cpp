#include "Application.h"
#include <stdio.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Error callback function
static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// Constructor
Application::Application() : window(nullptr), glsl_version("#version 330"), cKeyPressed(false) {}

// Destructor
Application::~Application() {}

// Initialize GLFW, OpenGL, and ImGui
bool Application::Init()
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return false;

    // Setup OpenGL context version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    window = glfwCreateWindow(1280, 720, "Dear ImGui", nullptr, nullptr);
    if (window == nullptr)
        return false;
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

    glfwSwapInterval(1); // Enable vsync

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD\n";
        return false;
    }

    // Initialize ImGui
    imgui_manager.Init(window, glsl_version);

    SetupOpenGL();

    return true;
}

std::string Application::readShaderSource(const char* filePath) {
    std::string content;
    std::ifstream fileStream(filePath, std::ios::in);
    std::string line = "";
    while (!fileStream.eof()) {
        getline(fileStream, line);
        content.append(line + "\n");
    }
    fileStream.close();
    return content;
}

void Application::SetupOpenGL() {
    //reads the glsl file
    std::string vertShaderStr = readShaderSource("VertShader.glsl");
    std::string fragShaderStr = readShaderSource("FragShader.glsl");
    const char* vertexShaderSource = vertShaderStr.c_str();
    const char* fragmentShaderSource = fragShaderStr.c_str();

    //compilation of shaders
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);//(shader Object ot compile to, number of strings, actual source code, NULL)
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    //To check the success of the compilation of shaders
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    //Linking shaders
    //GLuint shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    //To check for the linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    //shader objects are no longer needed
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glEnable(GL_DEPTH_TEST);//for depth testing

    //vertices for rectangle
    float vertices[] = {
        // positions       
         0.5f,  0.5f, 0.0f, // top right
         0.5f, -0.5f, 0.0f,// bottom right
        -0.5f, -0.5f, 0.0f,// bottom left
        -0.5f,  0.5f, 0.0f,// top left 
    };
    unsigned int indices[] = {
    0, 1, 3,   // first triangle
    1, 2, 3    // second triangle
    };

    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);

    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);//binds VAO

    glBindBuffer(GL_ARRAY_BUFFER, VBO);//buffer type of VBO is GL_ARRAY_BUFFER
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);//copies previously defined vertex data into the buffer's memory; last input can have three forms (check internet)

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); //(specifies which vertex attribute we want to configure (location=0), size of vertex attribute (vec3), type of data, if the data is to be normalized, stride (space between consecutive vertex attributes, offset))
    glEnableVertexAttribArray(0);

    //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    //glEnableVertexAttribArray(1);

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

    deltaTime = 0.0f;
    lastFrame = 0.0f;
}

void Application::processInput(GLFWwindow* window) {
    //Esc for closing the window
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

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

    // Toggle cursor visibility with 'C'
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

    // Reset the flag when the key is released
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE) {
        cKeyPressed = false;
    }
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

        processInput(window);

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
        glUseProgram(shaderProgram);

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

        model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        //model = glm::rotate(model, (float)glfwGetTime() * glm::radians(-50.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        //view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
        projection = glm::perspective(glm::radians(fov), (float)display_w / (float)display_h, 0.1f, 100.0f);

        // Retrieve the matrix uniform locations in the shader
        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");

        // Pass matrices to the shader
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));   // Pass model matrix
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));     // Pass view matrix
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection)); // Pass projection matrix

        glBindVertexArray(VAO);
        //glDrawArrays(GL_TRIANGLES, 0, 3);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);//GL_LINE for wireframe,GL_FILL for filled
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

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
