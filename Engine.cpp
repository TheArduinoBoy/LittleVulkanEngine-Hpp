#include "Engine.hpp"

#include "MovementController.hpp"
#include "Camera.hpp"
#include "RenderSystem.hpp"
#include "Buffer.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <chrono>
#include <iostream>
#include <stdexcept>

namespace Engine {

    struct GlobalUbo {
        glm::mat4 projectionView{1.f};
        glm::vec3 lightDirection = glm::normalize(glm::vec3{1.f, -3.f, -1.f});
    };

    Engine::Engine() { loadGameObjects(); }

    Engine::~Engine() {}

    void Engine::run() {
        Buffer globalUboBuffer{device, sizeof(GlobalUbo), SwapChain::MAX_FRAMES_IN_FLIGHT, vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible, device.properties.limits.minUniformBufferOffsetAlignment};
        globalUboBuffer.map();

        RenderSystem simpleRenderSystem{device, renderer.getSwapChainRenderPass()};
        Camera camera{};

        auto viewerObject = GameObject::createGameObject();
        MovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();
        bool shouldClose = false;
        SDL_Event event;
        while (!shouldClose) {
            while(SDL_PollEvent(&event)) {
                switch(event.type) {
                    case SDL_WINDOWEVENT_RESIZED: {
                        window.framebufferResizeCallback(window.getExtent().width, window.getExtent().height);
                    }
                    case SDL_QUIT: {
                        shouldClose = true;
                    }
                }
            }

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime =
                std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            cameraController.moveInPlaneXZ(frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            float aspect = renderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);

            if (auto commandBuffer = renderer.beginFrame()) {
                int frameIndex = renderer.getFrameIndex();
                FrameInfo frameInfo{frameIndex, frameTime, commandBuffer, camera};

                GlobalUbo ubo{};
                ubo.projectionView = camera.getProjection() * camera.getView();
                globalUboBuffer.writeToIndex(&ubo, frameIndex);
                globalUboBuffer.flushIndex(frameIndex);

                renderer.beginSwapChainRenderPass(commandBuffer);

                simpleRenderSystem.renderGameObjects(frameInfo, gameObjects);

                renderer.endSwapChainRenderPass(commandBuffer);
                renderer.endFrame();
            }
        }

        device.device().waitIdle();
    }

    void Engine::loadGameObjects() {
        std::shared_ptr<Model> model =
            Model::createModelFromFile(device, "./Models/SmoothVase.obj");
        auto gameObj = GameObject::createGameObject();
        gameObj.model = model;
        gameObj.transform.translation = {.0f, .5f, 2.5f};
        gameObj.transform.scale = glm::vec3(3.f);
        gameObjects.push_back(std::move(gameObj));
    }

}
