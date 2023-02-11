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
#include <vector>

auto constexpr screenWidth = 800;
auto constexpr screenHeight = 800;

// OpenGL resources
unsigned int geometryVertexBuffer = 0;
unsigned int geometryIndexBuffer = 0;
unsigned int geometryVertexArrayObject = 0;

// Shader variables
unsigned int shaderProgram = 0;
unsigned int vertexColorShaderVar = 0;
unsigned int modelShaderVar = 0;

// Geometry
std::vector<float> sphereVertices;
std::vector<float> sphereNormals;
std::vector<float> sphereTextCoords;
std::vector<short> sphereIndices;

// Position of the Sun
float sunPositionX = 0.0f;
float sunPositionY = 0.0f;

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

// Process the keyboard events
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        // Quit application on pressing the escape button
        glfwSetWindowShouldClose(window, true);
    }
    else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        sunPositionX -= 0.005f;
    }
    else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        sunPositionX += 0.005f;
    }
    else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        sunPositionY += 0.005f;
    }
    else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        sunPositionY -= 0.005f;
    }
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
    auto window = glfwCreateWindow(screenWidth, screenHeight, "Basic Solar System", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }
    glfwMakeContextCurrent(window);
    return window;
}

void createSphere(float radius, int numSegmentsInWidth, int numSegmentsInHeight)
{
    int numVertices = (numSegmentsInWidth + 1) * (numSegmentsInHeight + 1);
    int numIndices = 2 * numSegmentsInWidth * (numSegmentsInHeight - 1) * 3;
    int numUvs = (numSegmentsInHeight + 1) * (numSegmentsInWidth + 1) * 2;

    sphereVertices.resize((unsigned int)(numVertices * 3));
    sphereNormals.resize((unsigned int)(numVertices * 3));
    sphereIndices.resize((unsigned int)numIndices);

    int segmentInWidth, segmentInHeight;
    int vertIndex = 0, index = 0;
    const float normLen = 1.0f / radius;

    for (segmentInHeight = 0; segmentInHeight <= numSegmentsInHeight; ++segmentInHeight)
    {
        float horAngle = (float)glm::pi<float>() * segmentInHeight / numSegmentsInHeight;
        float z = radius * (float)cos(horAngle);
        float ringRadius = radius * (float)sin(horAngle);

        for (segmentInWidth = 0; segmentInWidth <= numSegmentsInWidth; ++segmentInWidth)
        {
            float verAngle = 2.0f * (float)glm::pi<float>() * segmentInWidth / numSegmentsInWidth;
            float x = ringRadius * (float)cos(verAngle);
            float y = ringRadius * (float)sin(verAngle);

            sphereNormals[vertIndex] = x * normLen;
            sphereVertices[vertIndex++] = x;
            sphereNormals[vertIndex] = z * normLen;
            sphereVertices[vertIndex++] = z;
            sphereNormals[vertIndex] = y * normLen;
            sphereVertices[vertIndex++] = y;

            if (segmentInWidth > 0 && segmentInHeight > 0)
            {
                int a = ((numSegmentsInWidth + 1) * segmentInHeight + segmentInWidth);
                int b = ((numSegmentsInWidth + 1) * segmentInHeight + segmentInWidth - 1);
                int c = ((numSegmentsInWidth + 1) * (segmentInHeight - 1) + segmentInWidth - 1);
                int d = ((numSegmentsInWidth + 1) * (segmentInHeight - 1) + segmentInWidth);

                if (segmentInHeight == numSegmentsInHeight)
                {
                    sphereIndices[index++] = (short)a;
                    sphereIndices[index++] = (short)c;
                    sphereIndices[index++] = (short)d;
                }
                else if (segmentInHeight == 1)
                {
                    sphereIndices[index++] = (short)a;
                    sphereIndices[index++] = (short)b;
                    sphereIndices[index++] = (short)c;
                }
                else
                {
                    sphereIndices[index++] = (short)a;
                    sphereIndices[index++] = (short)b;
                    sphereIndices[index++] = (short)c;
                    sphereIndices[index++] = (short)a;
                    sphereIndices[index++] = (short)c;
                    sphereIndices[index++] = (short)d;
                }
            }
        }
    }

    sphereTextCoords.resize((unsigned int)numUvs);

    numUvs = 0;
    for (segmentInHeight = 0; segmentInHeight <= numSegmentsInHeight; ++segmentInHeight)
    {
        for (segmentInWidth = 0; segmentInWidth <= numSegmentsInWidth; ++segmentInWidth)
        {
            sphereTextCoords[numUvs++] = (float)segmentInWidth / numSegmentsInWidth;
            sphereTextCoords[numUvs++] = (float)segmentInHeight / numSegmentsInHeight;
        }
    }
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
    modelShaderVar = glGetUniformLocation(shaderProgram, "uTransform");

    createSphere(2, 20, 20);

    glGenVertexArrays(1, &geometryVertexArrayObject);
    glBindVertexArray(geometryVertexArrayObject);

    // Create and bind buffer of vertex
    glGenBuffers(1, &geometryVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, geometryVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(sphereVertices[0]), sphereVertices.data(), GL_STATIC_DRAW);

    // Create and bind buffer of vertex indices
    glGenBuffers(1, &geometryIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometryIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(sphereIndices[0]), sphereIndices.data(), GL_STATIC_DRAW);

    // Set the binded vertex array to vertex position attribute
    auto aPos = glGetAttribLocation(shaderProgram, "aPos");
    glVertexAttribPointer(aPos, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    // Enable the vertex attribute array
    glEnableVertexAttribArray(aPos);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

glm::mat4 drawPlanet(glm::mat4 const &parentTransformation, glm::mat4 const &initialTransformation, float const &rotation, float const &revolution, glm::vec3 const &fillColor)
{
    // Set the set shader program
    glUseProgram(shaderProgram);

    // Set the fill color to the shader
    glUniform4f(vertexColorShaderVar, fillColor[0], fillColor[1], fillColor[2], 1.0f);

    // Set the initial transformation
    auto modelTransformation{initialTransformation};

    // Get the current translation
    auto modelPosition = glm::translate(glm::mat4(1.0f), glm::vec3(modelTransformation[3]));
    // Move the model to the world origin
    modelTransformation = glm::inverse(modelPosition) * modelTransformation;

    // Rotate on its own axis
    auto modelRotation = glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3(0.0, 0.0, 1.0));
    modelTransformation = modelRotation * modelTransformation;

// Do scale based on the origin - Only to try out pivot based scaling
// Enable if required
#if 0
        auto modelScale = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f));
        modelTransformation = modelScale * modelTransformation;
#endif

    // Translate the model back to the original position
    modelTransformation = modelPosition * modelTransformation;

    // Revolution
    auto modelRevolution = glm::rotate(glm::mat4(1.0f), glm::radians(revolution), glm::vec3(0.0, 0.0, 1.0));
    modelTransformation = modelRevolution * modelTransformation;

    // Final transformation = Parent transformation * Model transformation
    auto worldTransform = parentTransformation * modelTransformation;

    glUniformMatrix4fv(modelShaderVar, 1, GL_FALSE, glm::value_ptr(worldTransform));

    glBindVertexArray(geometryVertexArrayObject);
    glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_SHORT, nullptr);

    glBindVertexArray(0);

    return worldTransform;
}

void render(GLFWwindow *window)
{
    // Draw the sphere as wire frame, to see the rotation
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        // Process keyboard input
        processInput(window);

        // Set color for the window
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        constexpr float scale = 1.0f / 25.0f;
        auto initialScale = glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, scale));
        // Draw Sun - YELLOW
        // We can move the Sun using the arrow keys (LEFT, RIGHT, UP and DOWN)
        auto sunTranslation = glm::translate(glm::mat4(1.0f), glm::vec3(sunPositionX, sunPositionY, 0.0f));
        auto sunTransformation = sunTranslation * initialScale;
        static float sunRotation = 0.0f;
        sunRotation += 0.5f;
        auto sunWorldTransformation = drawPlanet(glm::mat4(1.0f), sunTransformation, sunRotation, 0.0f, glm::vec3(1.0f, 1.0f, 0.0f));

        // Draw Earth as Sun as parent - EARTH
        auto earthTransformation = glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 0.0f, 0.0f));
        static float earthRotation = 0.0f;
        earthRotation += 1.0f;
        static float earthRevolution = 0.0f;
        earthRevolution += 0.5f;
        auto earthWorldTransformation = drawPlanet(sunWorldTransformation, earthTransformation, earthRotation, earthRevolution, glm::vec3(0.0f, 0.0f, 1.0f));

        // Draw Moon as Earth as parent - GREY
        auto moonTranslation = glm::translate(glm::mat4(1.0f), glm::vec3(4.0f, 0.0f, 0.0f));
        // Scale down the Moon a bit
        auto moonScale = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f));
        auto moonTransformation = moonTranslation * moonScale;
        static float moonRotation = 0.0f;
        moonRotation += 1.0f;
        static float moonRevolution = 0.0f;
        moonRevolution += 0.5f;
        auto moonWorldTransformation = drawPlanet(earthWorldTransformation, moonTransformation, moonRotation, moonRevolution, glm::vec3(0.8f, 0.8f, 0.8f));

        // Draw Mars as Sun as parent - RED
        auto marsTranslation = glm::translate(glm::mat4(1.0f), glm::vec3(18.0f, 0.0f, 0.0f));
        // Scale up the Mars a bit
        auto marsScale = glm::scale(glm::mat4(1.0f), glm::vec3(1.5f, 1.5f, 1.5f));
        auto marsTransformation = marsTranslation * marsScale;
        static float marsRotation = 0.0f;
        marsRotation += 1.0f;
        static float marsRevolution = 0.0f;
        marsRevolution += 0.25f;
        drawPlanet(sunWorldTransformation, marsTransformation, marsRotation, marsRevolution, glm::vec3(1.0f, 0.0f, 0.0f));

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
    setupTriangle();

    render(window.get());

    return 0;
}