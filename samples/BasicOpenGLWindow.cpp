#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <iostream>
#include <memory>

auto constexpr screen_width = 800;
auto constexpr screen_height = 600;

// Whenever the window size changed this callback function executes
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
    std::cerr << "width: " << width << std::endl;
    std::cerr << "height: " << height << std::endl;
}

GLFWwindow *createAndConfigureWindow()
{
    // Initialize and configure glfw
    if (!glfwInit())
    {
        std::cout << "Failed to initialize glfw" << std::endl;
        return nullptr;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // OpenGL version 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    // Create window using glfw
    auto window = glfwCreateWindow(screen_width, screen_height, "OpenGL", nullptr, nullptr);
    if (!window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);
    return window;
}

void render(GLFWwindow *window)
{
    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        // Set color for the window
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Swap buffers
        glfwSwapBuffers(window);
        // Poll IO events
        glfwPollEvents();
    }
}

int main()
{
    auto glfw_window_deleter = [](GLFWwindow *window)
    {
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
        std::cout << "Failed to initialize GLAD" << std::endl;
        return 1;
    }

    render(window.get());

    return 0;
}