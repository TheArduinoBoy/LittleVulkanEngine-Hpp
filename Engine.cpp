#include "Engine.hpp"

#include "RenderSystem.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <stdexcept>

namespace Engine {
    Engine::Engine() {
        loadGameObjects();
    }

    Engine::~Engine() {}

    void Engine::run() {
        RenderSystem renderSystem{device, renderer.getSwapChainRenderPass()};

        while (!window.shouldClose()) {
            glfwPollEvents();

            if(auto commandBuffer = renderer.beginFrame()) {
                renderer.beginSwapChainRenderPass(commandBuffer);
                renderSystem.renderGameObjects(commandBuffer, gameObjects);
                renderer.endSwapChainRenderPass(commandBuffer);
                renderer.endFrame();
            }
        }

        device.device().waitIdle();
    }

    void Engine::loadGameObjects() {
        std::vector<Model::Vertex> vertices{
            {{0.f, -0.5f}, {1.f, 0.f, 0.f}},
            {{0.5f, 0.5f}, {0.f, 1.f, 0.f}},
            {{-0.5f, 0.5f}, {0.f, 0.f, 1.f}}};

        auto model = std::make_shared<Model>(device, vertices);

        auto triangle = GameObject::createGameObject();
        triangle.model = model;
        triangle.color = {1.f, .8f, .1f};
        triangle.transform2d.translation.x = .2f;
        triangle.transform2d.scale = {2.f, .5f};
        triangle.transform2d.rotation = .25f * glm::two_pi<float>();
        
        gameObjects.push_back(std::move(triangle));
    }
}