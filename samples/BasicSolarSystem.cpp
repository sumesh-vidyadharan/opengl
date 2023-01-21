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

auto constexpr screen_width = 800;
auto constexpr screen_height = 600;
// OpenGL resources
unsigned int triangleVertexBuffer = 0;
unsigned int triangleElementBuffer = 0;
unsigned int triangleVertexArray = 0;
// Shader variables
unsigned int shaderProgram = 0;
unsigned int vertexColorLocation = 0;
unsigned int transformLocation = 0;
// Matrices
glm::mat4 modelInitialTransformation(1.0f);

const char *vertexShaderSource = "#version 330 core\n"
                                 "layout (location = 0) in vec3 aPos;\n"
                                 "uniform mat4 uTransform;\n"
                                 "void main()\n"
                                 "{\n"
                                 "   gl_Position = uTransform * vec4(aPos, 1.0);\n"
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
    auto window = glfwCreateWindow(screen_width, screen_height, "Transformations", nullptr, nullptr);
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
    vertexColorLocation = glGetUniformLocation(shaderProgram, "uFillColor");
    transformLocation = glGetUniformLocation(shaderProgram, "uTransform");

    // Set up vertex data and configure vertex attributes
    const float vertices[] = {
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        0.0f, 1.0f, 0.0f};
    const unsigned int indices[] = {0, 1, 2};

    glGenVertexArrays(1, &triangleVertexArray);
    glBindVertexArray(triangleVertexArray);

    // Create and bind buffer of vertex
    glGenBuffers(1, &triangleVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, triangleVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Create and bind buffer of vertex indices
    glGenBuffers(1, &triangleElementBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleElementBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Set the binded vertex array to vertex position attribute
    auto aPos = glGetAttribLocation(shaderProgram, "aPos");
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    // Enable the vertex attribute array
    glEnableVertexAttribArray(aPos);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    auto modelInitialScale = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f));
    modelInitialTransformation = modelInitialScale * modelInitialTransformation;

    auto modelInitialTranslation = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.0f, 0.0f));
    modelInitialTransformation = modelInitialTranslation * modelInitialTransformation;
}

void drawTriangle(glm::mat4 const &parentTransformation, glm::mat4 const &initialTransformation, float const &rotation, float const &revolution, glm::vec3 const &fillColor)
{
    // Set the set shader program
    glUseProgram(shaderProgram);

    // Set the fill color to the shader
    glUniform4f(vertexColorLocation, fillColor[0], fillColor[1], fillColor[2], 1.0f);

    // Set the initial transformation
    auto modelTransformation{initialTransformation};

    // Get the current translation
    auto modelPosition = glm::translate(glm::mat4(1.0f), glm::vec3(modelTransformation[3]));
    // Move the model to the world origin
    modelTransformation = glm::inverse(modelPosition) * modelTransformation;

    // Rotate on its own axis
    auto modelRotation = glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3(0.0, 1.0, 0.0));
    modelTransformation = modelRotation * modelTransformation;

    // Do scale based on the origin
    auto modelScale = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f));
    modelTransformation = modelScale * modelTransformation;

    // Traslate the model back to the original position
    modelTransformation = modelPosition * modelTransformation;

    // Revolution
    auto modelRevolution = glm::rotate(glm::mat4(1.0f), glm::radians(revolution), glm::vec3(0.0, 0.0, 1.0));
    modelTransformation = modelRevolution * modelTransformation;

    // Final transformation = Parent transformation * Model transformation
    auto worldTransform = parentTransformation * modelTransformation;

    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(worldTransform));

    glBindVertexArray(triangleVertexArray);
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
        glClear(GL_COLOR_BUFFER_BIT);

        auto initialScale = glm::scale(glm::mat4(1.0f), glm::vec3(0.125f, 0.125f, 0.125f));

        // Draw Sun
        static float sunPosition = 0.0f;
        // sunPosition += 0.001f;
        auto sunInitialTranslation = glm::translate(glm::mat4(1.0f), glm::vec3(sunPosition, 0.0f, 0.0f));
        ;
        auto sunIntialTransformation = sunInitialTranslation * initialScale;
        drawTriangle(glm::mat4(1.0f), sunIntialTransformation, 0.0f, 0.0f, glm::vec3(1.0f, 1.0f, 0.0f));

        // Draw Earth
        auto earthInitialTransformation = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f));
        ;
        static float earthRotation = 0.0f;
        earthRotation += 1.0f;
        static float earthRevolution = 0.0f;
        earthRevolution += 1.0f;
        drawTriangle(sunIntialTransformation, earthInitialTransformation, earthRotation, earthRevolution, glm::vec3(0.0f, 0.0f, 1.0f));

        // Draw Mars
        auto marsInitialTransformation = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 0.0f, 0.0f));
        static float marsRotation = 0.0f;
        marsRotation += 1.0f;
        static float marsRevolution = 0.0f;
        marsRevolution += 0.25f;
        drawTriangle(sunIntialTransformation, marsInitialTransformation, marsRotation, marsRevolution, glm::vec3(1.0f, 0.0f, 0.0f));

        // Swap buffers
        glfwSwapBuffers(window);
        // Poll IO events
        glfwPollEvents();
    }
}

void cleanup()
{
    if (0 < triangleVertexArray)
    {
        auto vertexArrays{triangleVertexArray};
        glDeleteVertexArrays(1, &vertexArrays);
    }

    if (0 < triangleElementBuffer)
    {
        auto elementBuffers{triangleElementBuffer};
        glDeleteBuffers(1, &elementBuffers);
    }

    if (0 < triangleVertexBuffer)
    {
        auto vertexBuffers{triangleVertexBuffer};
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
    setupTriangle();

    render(window.get());

    return 0;
}