#pragma once

#include "Device.hpp"
#include "Model.hpp"
#include "GameObject.hpp"
#include "Pipeline.hpp"
#include "SwapChain.hpp"
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
        void createPipelineLayout();
        void createPipeline();
        void createCommandBuffers();
        void freeCommandBuffers();
        void drawFrame();
        void recreateSwapChain();
        void recordCommandBuffer(int imageIndex);
        void renderGameObjects(vk::CommandBuffer commandBuffer);

        Window window{WIDTH, HEIGHT, "Hello Vulkan!"};
        Device device{window};
        std::unique_ptr<SwapChain> swapChain;
        std::unique_ptr<Pipeline> pipeline;
        vk::PipelineLayout pipelineLayout;
        std::vector<vk::CommandBuffer> commandBuffers;
        std::vector<GameObject> gameObjects;
    };
}