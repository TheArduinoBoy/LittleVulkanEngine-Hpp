#pragma once

#include <vulkan/vulkan.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <string>
namespace Engine {

    class Window {
        public:
        Window(std::string name);
        ~Window();

        Window(const Window &) = delete;
        Window &operator=(const Window &) = delete;

        vk::Extent2D getExtent() { return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}; }
        bool wasWindowResized() { return framebufferResized; }
        void resetWindowResizedFlag() { framebufferResized = false; }
        SDL_Window *getSDLwindow() const { return window; }

        void createWindowSurface(vk::Instance instance, vk::SurfaceKHR *surface);
        void framebufferResizeCallback(int width, int height);

        private:
        void initWindow();

        int width;
        int height;
        bool framebufferResized = false;

        std::string windowName;
        SDL_Window *window;
    };
}
