#pragma once

#include "Device.hpp"
#include "Descriptors.hpp"
#include "GameObject.hpp"
#include "Renderer.hpp"
#include "Window.hpp"

// std
#include <memory>
#include <vector>

namespace Engine {
    class Core {
        public:
        Core();
        ~Core();

        Core(const Core &) = delete;
        Core &operator=(const Core &) = delete;

        void run();

        private:
        void loadGameObjects();

        Window window{"Vulkan Engine"};
        Device device{window};
        Renderer renderer{window, device};

        std::unique_ptr<DescriptorPool> globalPool{};
        std::vector<GameObject> gameObjects;
    };
}
