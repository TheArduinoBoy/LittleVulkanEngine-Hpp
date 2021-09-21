#include "MovementController.hpp"

// std
#include <limits>

namespace Engine {

    void MovementController::moveInPlaneXZ(float dt, GameObject& gameObject) {
        const Uint8 *keystate = SDL_GetKeyboardState(NULL);
        int mouseX, mouseY = 0;
        Uint32 mousestate = SDL_GetRelativeMouseState(&mouseX, &mouseY);
        glm::vec3 rotate{0};
        rotate.y += mouseX;
        rotate.x -= mouseY;

        if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
            gameObject.transform.rotation += lookSpeed * dt * glm::normalize(rotate);
        }

        // limit pitch values between about +/- 85ish degrees
        gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
        gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());

        float yaw = gameObject.transform.rotation.y;
        const glm::vec3 forwardDir{sin(yaw), 0.f, cos(yaw)};
        const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};

        glm::vec3 moveDir{0.f};
        if (keystate[keys.moveForward]) moveDir += forwardDir;
        if (keystate[keys.moveBackward]) moveDir -= forwardDir;
        if (keystate[keys.moveRight]) moveDir += rightDir;
        if (keystate[keys.moveLeft]) moveDir -= rightDir;

        if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
            gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
        }
    }
}
