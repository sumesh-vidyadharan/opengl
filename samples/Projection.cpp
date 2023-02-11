#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <cmath>

auto constexpr screenWidth = 800;
auto constexpr screenHeight = 600;
// OpenGL resources
unsigned int geometryVertexBuffer = 0;
unsigned int geometryIndexBuffer = 0;
unsigned int geometryVertexArrayObject = 0;
// Shader variables
unsigned int shaderProgram = 0;
unsigned int vertexColorShaderVar = 0;
unsigned int modelShaderVar = 0;
unsigned int viewShaderVar = 0;
unsigned int projectionShaderVar = 0;
// Matrices
glm::mat4 projection(1.0f);

const char *vertexShaderSource = "#version 330 core\n"
                                 "layout (location = 0) in vec3 aPos;\n"
                                 "uniform mat4 uModel;\n"
                                 "uniform mat4 uView;\n"
                                 "uniform mat4 uProjection;\n"
                                 "void main()\n"
                                 "{\n"
                                 "   gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);\n"
                                 "}\0";

const char *fragmentShaderSource = "#version 330 core\n"
                                   "out vec4 FragColor;\n"
                                   "uniform vec4 uFillColor;\n"
                                   "void main()\n"
                                   "{\n"
                                   "   FragColor = uFillColor;\n"
                                   "}\n\0";

// Whenever the window size changed this callback function executes
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

GLFWwindow *createAndConfigureWindow()
{
    // Initialize and configure glfw
    if (!glfwInit())
    {
        throw std::runtime_error("Failed to initialize glfw");
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // OpenGL version 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    // Create window using glfw
    auto window = glfwCreateWindow(screenWidth, screenHeight, "Projection", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }
    glfwMakeContextCurrent(window);
    return window;
}

void setupTriangle()
{
    // Build and compile shader program
    // Vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        throw std::runtime_error("Failed to compile vertex shader " + std::string(infoLog));
    }

    // Fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // Check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        throw std::runtime_error("Failed to compile fragment shader " + std::string(infoLog));
    }

    // Link shaders
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // Check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        throw std::runtime_error("Failed to link program " + std::string(infoLog));
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Get the shader variables
    vertexColorShaderVar = glGetUniformLocation(shaderProgram, "uFillColor");
    modelShaderVar = glGetUniformLocation(shaderProgram, "uModel");
    viewShaderVar = glGetUniformLocation(shaderProgram, "uView");
    projectionShaderVar = glGetUniformLocation(shaderProgram, "uProjection");

    // Set up vertex data and configure vertex attributes
    const float vertices[] = {
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,};
    const unsigned int indices[] = {0, 1, 2};

    glGenVertexArrays(1, &geometryVertexArrayObject);
    glBindVertexArray(geometryVertexArrayObject);

    // Create and bind buffer of vertex
    glGenBuffers(1, &geometryVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, geometryVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Create and bind buffer of vertex indices
    glGenBuffers(1, &geometryIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometryIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Set the binded vertex array to vertex position attribute
    auto aPos = glGetAttribLocation(shaderProgram, "aPos");
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    // Enable the vertex attribute array
    glEnableVertexAttribArray(aPos);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void drawTriangle(glm::mat4 const &modelTransformation, glm::vec3 const &fillColor)
{
    // Set the set shader program
    glUseProgram(shaderProgram);

    // Set the fill color to the shader
    glUniform4f(vertexColorShaderVar, fillColor[0], fillColor[1], fillColor[2], 1.0f);

    glUniformMatrix4fv(modelShaderVar, 1, GL_FALSE, glm::value_ptr(modelTransformation));
    glUniformMatrix4fv(viewShaderVar, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
    glUniformMatrix4fv(projectionShaderVar, 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(geometryVertexArrayObject);
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);
}

void render(GLFWwindow *window)
{
    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        // Set color for the window
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto initialScale = glm::scale(glm::mat4(1.0f), glm::vec3(0.25f, 0.25f, 0.25f));

        // Draw Red triangle
        auto redTranslation = glm::translate(glm::mat4(1.0f), glm::vec3(-0.25f, 0.0f, -2.0f));
        auto redTransformation = redTranslation * initialScale ;
        drawTriangle(redTransformation, glm::vec3(1.0f, 0.0f, 0.0f));

        // Draw Green triangle
        auto greenTranslation = glm::translate(glm::mat4(1.0f), glm::vec3(0.25f, 0.0f, -1.f));
        auto greenTransformation = greenTranslation * initialScale;
        drawTriangle(greenTransformation, glm::vec3(0.0f, 1.0f, 0.0f));

        // Swap buffers
        glfwSwapBuffers(window);
        // Poll IO events
        glfwPollEvents();
    }
}

void cleanup()
{
    if (0 < geometryVertexArrayObject)
    {
        auto vertexArrays{geometryVertexArrayObject};
        glDeleteVertexArrays(1, &vertexArrays);
    }

    if (0 < geometryIndexBuffer)
    {
        auto elementBuffers{geometryIndexBuffer};
        glDeleteBuffers(1, &elementBuffers);
    }

    if (0 < geometryVertexBuffer)
    {
        auto vertexBuffers{geometryVertexBuffer};
        glDeleteBuffers(1, &vertexBuffers);
    }

    if (0 < shaderProgram)
    {
        glDeleteProgram(shaderProgram);
    }
}

int main()
{
    auto glfw_window_deleter = [](GLFWwindow *window)
    {
        cleanup();
        // Delete the created window
        glfwDestroyWindow(window);
        // Terminate glfw
        glfwTerminate();
    };
    std::unique_ptr<GLFWwindow, decltype(glfw_window_deleter)> window(createAndConfigureWindow(), glfw_window_deleter);
    if (!window)
    {
        return 1;
    }
    glfwSetFramebufferSizeCallback(window.get(), framebuffer_size_callback);

    // Load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        throw std::runtime_error("Failed to initialize GLAD");
    }

    #if 1
        const float aspectRatio = (float)screenWidth/screenHeight;
        projection = glm::perspective(glm::radians(90.0f), aspectRatio, 0.1f, 100.0f);
    #else
        projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f);
    #endif

    glEnable(GL_DEPTH_TEST);

    setupTriangle();

    render(window.get());

    return 0;
}