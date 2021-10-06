#pragma once

#include "Device.hpp"
#include "SwapChain.hpp"
#include "Window.hpp"

// std
#include <cassert>
#include <memory>
#include <vector>

namespace Engine {
    class Renderer {
        public:
        Renderer(Window& window, Device& device);
        ~Renderer();

        Renderer(const Renderer &) = delete;
        Renderer &operator=(const Renderer &) = delete;

        vk::RenderPass getSwapChainRenderPass() const { return swapChain->getRenderPass(); }
        bool isFrameInProgress() const { return isFrameStarted; }

        vk::CommandBuffer getCurrentCommandBuffer() const {
            assert(isFrameStarted && "Cannot get command buffer when frame is not in progress.");
            return commandBuffers[currentFrameIndex];
        }

        int getFrameIndex() const {
            assert(isFrameStarted && "Cannot get frame index when frame is not in progress.");
            return currentFrameIndex;
        }

        vk::CommandBuffer beginFrame();
        void endFrame();
        void beginSwapChainRenderPass(vk::CommandBuffer commandBuffer);
        void endSwapChainRenderPass(vk::CommandBuffer commandBuffer);

        private:
        void createCommandBuffers();
        void freeCommandBuffers();
        void recreateSwapChain();

        Window& window;
        Device& device;
        std::unique_ptr<SwapChain> swapChain;
        std::vector<vk::CommandBuffer> commandBuffers;

        uint32_t currentImageIndex;
        int currentFrameIndex{0};
        bool isFrameStarted{false};
    };
}