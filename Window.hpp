#pragma once

#include <vulkan/vulkan.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
namespace Engine {

    class Window {
        public:
        Window(int w, int h, std::string name);
        ~Window();

        Window(const Window &) = delete;
        Window &operator=(const Window &) = delete;

        bool shouldClose() { return glfwWindowShouldClose(window); }

        void createWindowSurface(vk::Instance instance, vk::SurfaceKHR *surface);

        vk::Extent2D getExtent() { return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}; }

        private:
        void initWindow();

        const int width;
        const int height;

        std::string windowName;
        GLFWwindow *window;
    };
}