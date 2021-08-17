#pragma once

#include "Window.hpp"

namespace Engine {

    class Engine {
        public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

        void run();

        private:
        Window window{WIDTH, HEIGHT, "Hello Vulkan!"};
    };
}
