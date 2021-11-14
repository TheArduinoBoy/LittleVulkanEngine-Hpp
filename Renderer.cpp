#include "Renderer.hpp"

// std
#include <array>
#include <cassert>
#include <stdexcept>

namespace Engine {

    Renderer::Renderer(Window& window, Device& device)
        : window{window}, device{device} {
        recreateSwapChain();
        createCommandBuffers();
    }

    Renderer::~Renderer() { freeCommandBuffers(); }

    void Renderer::recreateSwapChain() {
        auto extent = window.getExtent();
        SDL_Event event;
        while (extent.width == 0 || extent.height == 0) {
            extent = window.getExtent();
            SDL_WaitEvent(&event);
        }
        device.device().waitIdle();

        if (swapChain == nullptr) {
            swapChain = std::make_unique<SwapChain>(device, extent);
        } else {
            std::shared_ptr<SwapChain> oldSwapChain = std::move(swapChain);
            swapChain = std::make_unique<SwapChain>(device, extent, oldSwapChain);

            if (!oldSwapChain->compareSwapFormats(*swapChain.get())) {
                throw std::runtime_error("Swap chain image(or depth) format has changed!");
            }
        }
    }

    void Renderer::createCommandBuffers() {
        commandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

        vk::CommandBufferAllocateInfo allocInfo{device.getCommandPool(), vk::CommandBufferLevel::ePrimary, static_cast<uint32_t>(commandBuffers.size())};

        if(device.device().allocateCommandBuffers(&allocInfo, commandBuffers.data()) != vk::Result::eSuccess) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void Renderer::freeCommandBuffers() {
        device.device().freeCommandBuffers(device.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
        commandBuffers.clear();
    }

    vk::CommandBuffer Renderer::beginFrame() {
        assert(!isFrameStarted && "Can't call beginFrame while already in progress");

        auto result = swapChain->acquireNextImage(&currentImageIndex);
        if (result == vk::Result::eErrorOutOfDateKHR) {
            recreateSwapChain();
            return nullptr;
        }

        if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        isFrameStarted = true;

        auto commandBuffer = getCurrentCommandBuffer();
        vk::CommandBufferBeginInfo beginInfo{};

        if (commandBuffer.begin(&beginInfo) != vk::Result::eSuccess) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }
        return commandBuffer;
    }

    void Renderer::endFrame() {
        assert(isFrameStarted && "Can't call endFrame while frame is not in progress");
        auto commandBuffer = getCurrentCommandBuffer();
        commandBuffer.end();

        auto result = swapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
        if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || window.wasWindowResized()) {
            window.resetWindowResizedFlag();
            recreateSwapChain();
        } else if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        isFrameStarted = false;
        currentFrameIndex = (currentFrameIndex + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
    }

    void Renderer::beginSwapChainRenderPass(vk::CommandBuffer commandBuffer) {
        assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
        assert(
            commandBuffer == getCurrentCommandBuffer() &&
            "Can't begin render pass on command buffer from a different frame");

        vk::RenderPassBeginInfo renderPassInfo{swapChain->getRenderPass(), swapChain->getFrameBuffer(currentImageIndex), {{0, 0}, swapChain->getSwapChainExtent()}};

        std::array<vk::ClearValue, 2> clearValues{};
        clearValues[0].setColor(std::array<float, 4>{0.01f, 0.01f, 0.01f, 1.f});
        clearValues[1].setDepthStencil({1.0f, 0});
        renderPassInfo.setClearValueCount(static_cast<uint32_t>(clearValues.size()));
        renderPassInfo.setPClearValues(clearValues.data());

        commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

        vk::Viewport viewport{0.f, 0.f, static_cast<float>(swapChain->getSwapChainExtent().width), static_cast<float>(swapChain->getSwapChainExtent().height), 0.f, 1.f};
        vk::Rect2D scissor{{0, 0}, swapChain->getSwapChainExtent()};
        commandBuffer.setViewport(0, 1, &viewport);
        commandBuffer.setScissor(0, 1, &scissor);
    }

    void Renderer::endSwapChainRenderPass(vk::CommandBuffer commandBuffer) {
        assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
        assert(
            commandBuffer == getCurrentCommandBuffer() &&
            "Can't end render pass on command buffer from a different frame");
        commandBuffer.endRenderPass();
    }

}
