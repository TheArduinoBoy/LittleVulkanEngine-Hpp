#pragma once

#include "Device.hpp"
#include "GameObject.hpp"
#include "Renderer.hpp"
#include "Window.hpp"

// std
#include <memory>
#include <vector>

namespace Engine {
    class Engine {
        public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

        Engine();
        ~Engine();

        Engine(const Engine &) = delete;
        Engine &operator=(const Engine &) = delete;

        void run();

        private:
        void loadGameObjects();

        Window window{WIDTH, HEIGHT, "Vulkan Engine"};
        Device device{window};
        Renderer renderer{window, device};

        std::vector<GameObject> gameObjects;
    };
}
