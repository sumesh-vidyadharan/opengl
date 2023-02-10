#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <cmath>
#include <filesystem>

auto constexpr screen_width = 800;
auto constexpr screen_height = 800;
// OpenGL resources
unsigned int vertexBuffer = 0;
unsigned int elementBuffer = 0;
unsigned int vertexArray = 0;
unsigned int textureWall = 0;
unsigned int textureFace = 0;
// Shader variables
unsigned int shaderProgram = 0;
unsigned int textureShaderVar = 0;

const char *vertexShaderSource = "#version 330 core\n"
                                 "layout (location = 0) in vec3 aPos;\n"
                                 "layout (location = 1) in vec2 aTexCoord;\n"
                                 "out vec2 TexCoord;;\n"
                                 "void main()\n"
                                 "{\n"
                                 "   gl_Position = vec4(aPos, 1.0);\n"
                                 "   TexCoord = vec2(aTexCoord.x, aTexCoord.y);;\n"
                                 "}\0";

const char *fragmentShaderSource = "#version 330 core\n"
                                   "out vec4 FragColor;\n"
                                   "in vec2 TexCoord;\n"
                                   "uniform sampler2D uTexture;\n"
                                   "void main()\n"
                                   "{\n"
                                   "   FragColor = texture(uTexture, TexCoord);\n"
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

unsigned int  loadImage(std::string const& imageFilePath)
{
    unsigned int texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // OpenGL texture (0,0) is bottom left
    // Image (0,0) is top left
    // So we need to flip the image vertically
    stbi_set_flip_vertically_on_load(true);

    int imageWidth, imageHeight, channels;
    auto imageData = stbi_load(imageFilePath.c_str(), &imageWidth, &imageHeight, &channels, 0);
    if (imageData)
    {
        // If there is 4 channel in the image, we need to corresponding OpenGL format
        auto format = (4 == channels ) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, imageWidth, imageHeight, 0, format, GL_UNSIGNED_BYTE, imageData);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        throw std::runtime_error("Failed to load image:" + imageFilePath);
    }
    stbi_image_free(imageData);
    return texture;
}

void setupImageGeometry()
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

    textureShaderVar = glGetUniformLocation(shaderProgram, "uTexture");
    // Set up vertex and texture coordinate
    float vertices[] = {
        // vertex             // texture coords
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f,  // bottom left
        -0.5f,  0.5f, 0.0f,   0.0f, 1.0f,  // top left 
         0.5f,  0.5f, 0.0f,   1.0f, 1.0f,  // top right
         0.5f, -0.5f, 0.0f,   1.0f, 0.0f   // bottom right
    };

    const unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };;

    glGenVertexArrays(1, &vertexArray);
    glBindVertexArray(vertexArray);

    // Create and bind buffer of vertex
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Create and bind buffer of vertex indices
    glGenBuffers(1, &elementBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Set vertex position attribute
    auto aPos = glGetAttribLocation(shaderProgram, "aPos");
    glVertexAttribPointer(aPos, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    // Enable the vertex attribute array
    glEnableVertexAttribArray(aPos);

    // Set texture coordinate attribute
    auto aTexCoord = glGetAttribLocation(shaderProgram, "aTexCoord");
    glVertexAttribPointer(aTexCoord, 2 , GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    // Enable the vertex attribute array
    glEnableVertexAttribArray(aTexCoord);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void drawImage(unsigned int const& texture)
{
    // Set the set shader program
    glUseProgram(shaderProgram);

    // Active the 0th texture unit
    glActiveTexture(GL_TEXTURE0);
    // Bind the texture to the 0th texture unit
    glBindTexture(GL_TEXTURE_2D, texture);
    // Attach the 0th texture unit to the texture shader variable
    glUniform1f(textureShaderVar, 0);

    glBindVertexArray(vertexArray);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);
}

void render(GLFWwindow *window)
{
    // Enable alpha blending to support transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        // Set color for the window
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        drawImage(textureWall);
        drawImage(textureFace);

        // Swap buffers
        glfwSwapBuffers(window);
        // Poll IO events
        glfwPollEvents();
    }
}

void cleanup()
{
    if (0 < shaderProgram)
    {
        glDeleteProgram(shaderProgram);
    }

    if (0 < textureFace)
    {
        glDeleteTextures(1, &textureFace);
    }

    if (0 < textureWall)
    {
        glDeleteTextures(1, &textureWall);
    }

    if (0 < vertexArray)
    {
        auto vertexArrays{vertexArray};
        glDeleteVertexArrays(1, &vertexArrays);
    }

    if (0 < elementBuffer)
    {
        auto elementBuffers{elementBuffer};
        glDeleteBuffers(1, &elementBuffers);
    }

    if (0 < vertexBuffer)
    {
        auto vertexBuffers{vertexBuffer};
        glDeleteBuffers(1, &vertexBuffers);
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

    textureFace = loadImage("../bin/images/face.png");
    textureWall = loadImage("../bin/images/wall.jpg");
    setupImageGeometry();

    render(window.get());

    return 0;
}