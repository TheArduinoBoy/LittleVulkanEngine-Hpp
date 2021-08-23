#include "Engine.hpp"

namespace Engine {

    void Engine::run() {
        while (!window.shouldClose()) {
            glfwPollEvents();
        }
    }
}