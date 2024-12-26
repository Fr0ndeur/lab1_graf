#include <iostream>
#include <vector>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// -----------------------------------------------------------------------------
// Global Variables
// -----------------------------------------------------------------------------
float aspectRatio = 0.0f;
int currentWindowWidth = 0;
int currentWindowHeight = 0;

float xOffset = 0.25f;
float yOffset = -1.0f;
float moveBy = 0.0f;

const float bgColor[] = {0.9686f, 0.1137f, 0.1804f, 1.0f};
const float entityColor[] = {0.0f, 0.6f, 0.9843f, 1.0f};
constexpr size_t logSizeChars = 1024;

// Vertex and Element data
float vertices[] = {
    // 2D positions (x, y)
    -0.4f, 0.6f,    // 0
    -0.2f, 0.6f,    // 1
    -0.05f, 0.75f,  // 2
    0.25f, 0.75f,   // 3
    0.1f, 0.6f,     // 4
    -0.4f, 0.4f,    // 5
    -0.20f, 0.4f,   // 6
    0.20f, 0.00f,   // 7
    -0.2f, 0.15f,   // 8
    -0.35f, 0.00f,  // 9
    -0.20f, 0.00f,  // 10
    -0.20f, -0.15f, // 11
    0.00f, -0.2f,   // 12
    0.2f, -0.4f,    // 13
    -0.2f, -0.4f,   // 14
    -0.2f, -0.2f};  // 15

unsigned int indices[] = {
    0, 5, 6,     //
    0, 1, 6,     //
    1, 2, 4,     //
    3, 2, 4,     //
    6, 10, 7,    //
    8, 9, 11,    //
    10, 7, 13,   //
    12, 13, 14,  //
    14, 15, 12}; //

unsigned int lineIndices[] = {
    0, 1,   //
    0, 5,   //
    5, 6,   //
    1, 6,   //
    1, 2,   //
    2, 3,   //
    3, 4,   //
    1, 4,   //
    6, 7,   //
    7, 10,  //
    6, 10,  //
    8, 9,   //
    9, 11,  //
    10, 11, //
    10, 13, //
    13, 14, //
    12, 14, //
    14, 15, //
    12, 15, //
    7, 13};

// -----------------------------------------------------------------------------
// Shaders
// -----------------------------------------------------------------------------
const char *vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;

// We replicate the original behavior: scale x by 'aspectRatio', 
// and translate by xOffset / yOffset
uniform float aspectRatio;
uniform float xOffset;
uniform float yOffset;

void main()
{
    gl_Position = vec4(
        xOffset + aPos.x * aspectRatio,
        yOffset + aPos.y,
        0.0,
        1.0
    );
}
)";

const char *fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
uniform vec4 aColor;

void main()
{
    FragColor = aColor;
}
)";

// -----------------------------------------------------------------------------
// Function Prototypes
// -----------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

void checkShaderCompiling(unsigned int shader, char *shaderInfoLog, size_t logSize = logSizeChars);
void checkShaderProgramLinking(unsigned int program, char *shaderInfoLog, size_t logSize = logSizeChars);

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
int main()
{
    // Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create a fullscreen window (optional) or windowed
    const GLFWvidmode *vidmode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    GLFWwindow *window = glfwCreateWindow(
        vidmode->width,
        vidmode->height,
        "Computer Graphics (GLAD, GLFW, GLM)",
        nullptr, nullptr);

    if (window == nullptr)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Load all OpenGL function pointers via GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    // -------------------------------------------------------------------------
    // Build and compile our shader program
    // -------------------------------------------------------------------------
    char infoLog[logSizeChars];

    // 1. Compile vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    checkShaderCompiling(vertexShader, infoLog);

    // 2. Compile fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    checkShaderCompiling(fragmentShader, infoLog);

    // 3. Link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    checkShaderProgramLinking(shaderProgram, infoLog);

    // Once linked, we can delete the individual shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Use the final program
    glUseProgram(shaderProgram);

    // -------------------------------------------------------------------------
    // Setup buffers and arrays
    // -------------------------------------------------------------------------
    unsigned int VBO, VAOs[2], EBOs[2];
    glGenVertexArrays(2, VAOs);
    glGenBuffers(1, &VBO);
    glGenBuffers(2, EBOs);

    // -- VAO[0] : For orange filled triangles (body)
    glBindVertexArray(VAOs[0]);

    // Vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Vertex attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // Unbind VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // -- VAO[1] : For black lines (edges)
    glBindVertexArray(VAOs[1]);

    // We can reuse the same VBO since the vertex data is the same
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Another element buffer for lines
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(lineIndices), lineIndices, GL_STATIC_DRAW);

    // Vertex attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // Unbind VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Optional: set line thickness
    glLineWidth(5.0f);

    // Variables for FPS/time calculation
    double lastTime = glfwGetTime();
    int nbFrames = 0;

    // -------------------------------------------------------------------------
    // Render Loop
    // -------------------------------------------------------------------------
    while (!glfwWindowShouldClose(window))
    {
        // Process input
        processInput(window);

        // Calculate time per frame for offset-based movement
        double currentTime = glfwGetTime();
        ++nbFrames;
        moveBy = static_cast<float>((currentTime - lastTime) / nbFrames);

        if (currentTime - lastTime >= 1.0)
        {
            nbFrames = 0;
            lastTime += 1.0;
        }

        // Clear screen
        glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);
        glClear(GL_COLOR_BUFFER_BIT);

        // Update aspect ratio
        glfwGetWindowSize(window, &currentWindowWidth, &currentWindowHeight);
        aspectRatio = static_cast<float>(currentWindowHeight) / static_cast<float>(currentWindowWidth);

        // Retrieve uniform locations
        int vertexRatioLocation = glGetUniformLocation(shaderProgram, "aspectRatio");
        int vertexXOffsetLocation = glGetUniformLocation(shaderProgram, "xOffset");
        int vertexYOffsetLocation = glGetUniformLocation(shaderProgram, "yOffset");
        int fragmentColorLocation = glGetUniformLocation(shaderProgram, "aColor");

        // Use the program and update uniforms
        glUseProgram(shaderProgram);
        glUniform1f(vertexRatioLocation, aspectRatio);
        glUniform1f(vertexXOffsetLocation, xOffset);
        glUniform1f(vertexYOffsetLocation, yOffset);

        // Draw the main (orange) body
        glBindVertexArray(VAOs[0]);
        glUniform4fv(fragmentColorLocation, 1, entityColor);
        glDrawElements(GL_TRIANGLES, 27, GL_UNSIGNED_INT, 0);

        // Draw black edges
        glBindVertexArray(VAOs[1]);
        glUniform4f(fragmentColorLocation, 0.0f, 0.0f, 0.0f, 1.0f);
        glDrawElements(GL_LINES, 40, GL_UNSIGNED_INT, 0);

        // Swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(2, VAOs);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(2, EBOs);
    glDeleteProgram(shaderProgram);

    // Terminate GLFW
    glfwTerminate();
    return 0;
}

// -----------------------------------------------------------------------------
// Callbacks & Utilities
// -----------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    // Movement keys
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        yOffset += moveBy;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        xOffset -= moveBy;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        yOffset -= moveBy;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        xOffset += moveBy;
    }
}

void checkShaderCompiling(unsigned int shader, char *shaderInfoLog, size_t logSize)
{
    int shaderSuccess;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &shaderSuccess);

    if (!shaderSuccess)
    {
        glGetShaderInfoLog(shader, logSize, nullptr, shaderInfoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n"
                  << shaderInfoLog << std::endl;
    }
}

void checkShaderProgramLinking(unsigned int program, char *shaderInfoLog, size_t logSize)
{
    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success)
    {
        glGetProgramInfoLog(program, logSize, nullptr, shaderInfoLog);
        std::cerr << "ERROR::SHADER_PROGRAM::LINKING_FAILED\n"
                  << shaderInfoLog << std::endl;
    }
}
