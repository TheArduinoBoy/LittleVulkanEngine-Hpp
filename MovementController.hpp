#pragma once

#include "GameObject.hpp"
#include "Window.hpp"

namespace Engine {
    class MovementController {
        public:
        struct KeyMappings {
            SDL_Scancode moveLeft = SDL_SCANCODE_A;
            SDL_Scancode moveRight = SDL_SCANCODE_D;
            SDL_Scancode moveForward = SDL_SCANCODE_W;
            SDL_Scancode moveBackward = SDL_SCANCODE_S;
        };

        void moveInPlaneXZ(float dt, GameObject& gameObject);

        KeyMappings keys{};
        float moveSpeed{3.f};
        float lookSpeed{15.f};
    };
}