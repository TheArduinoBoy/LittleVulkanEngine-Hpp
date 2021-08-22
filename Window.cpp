#include "Window.hpp"

// std
#include <stdexcept>

namespace Engine {

    Window::Window(int w, int h, std::string name) : width{w}, height{h}, windowName{name} {
        initWindow();
    }

    Window::~Window() {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void Window::initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }

    void Window::createWindowSurface(vk::Instance instance, vk::SurfaceKHR *surface) {
        if (glfwCreateWindowSurface(static_cast<VkInstance>(instance), window, nullptr, reinterpret_cast<VkSurfaceKHR*>(surface)) != VK_SUCCESS) {
            throw std::runtime_error("failed to craete window surface");
        }
    }

    void Window::framebufferResizeCallback(GLFWwindow *window, int width, int height) {
        auto eWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
        eWindow->framebufferResized = true;
        eWindow->width = width;
        eWindow->height = height;
    }

}  // namespace lve