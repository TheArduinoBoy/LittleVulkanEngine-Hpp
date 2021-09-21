#include "Window.hpp"

// std
#include <stdexcept>

namespace Engine {

    Window::Window(std::string name) : windowName{name} {
        initWindow();
    }

    Window::~Window() {
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void Window::initWindow() {
        SDL_Init(SDL_INIT_EVERYTHING);

        SDL_DisplayMode displayMode;
        SDL_GetCurrentDisplayMode(0, &displayMode);
        width = displayMode.w;
        height = displayMode.h;

        window = SDL_CreateWindow(windowName.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_VULKAN | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_ALLOW_HIGHDPI);
        SDL_ShowCursor(SDL_DISABLE);
        SDL_SetRelativeMouseMode(SDL_TRUE);
    }

    void Window::createWindowSurface(vk::Instance instance, vk::SurfaceKHR *surface) {
        if (SDL_Vulkan_CreateSurface(window, static_cast<VkInstance>(instance), reinterpret_cast<VkSurfaceKHR*>(surface)) != SDL_TRUE) {
            throw std::runtime_error("failed to craete window surface");
        }
    }

    void Window::framebufferResizeCallback(int width, int height) {
        this->framebufferResized = true;
        this->width = width;
        this->height = height;
    }

}  // namespace lve
