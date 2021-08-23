#include "Engine.hpp"

// std
#include <array>
#include <stdexcept>

namespace Engine {

    Engine::Engine() {
        createPipelineLayout();
        createPipeline();
        createCommandBuffers();
    }

    Engine::~Engine() { device.device().destroyPipelineLayout(pipelineLayout, nullptr); }

    void Engine::run() {
        while (!window.shouldClose()) {
            glfwPollEvents();
            drawFrame();
        }

        device.device().waitIdle();
    }

    void Engine::createPipelineLayout() {
        vk::PipelineLayoutCreateInfo pipelineLayoutInfo{{}, 0, nullptr, 0, nullptr};
        if (device.device().createPipelineLayout(&pipelineLayoutInfo, nullptr, &pipelineLayout) != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void Engine::createPipeline() {
        PipelineConfigInfo pipelineConfig{};
        Pipeline::defaultPipelineConfigInfo(pipelineConfig, swapChain.width(), swapChain.height());
        pipelineConfig.renderPass = swapChain.getRenderPass();
        pipelineConfig.pipelineLayout = pipelineLayout;
        pipeline = std::make_unique<Pipeline>(device, "Shaders/Shader.vert.spv", "Shaders/Shader.frag.spv", pipelineConfig);
    }

    void Engine::createCommandBuffers() {
        commandBuffers.resize(swapChain.imageCount());

        vk::CommandBufferAllocateInfo allocInfo{device.getCommandPool(), vk::CommandBufferLevel::ePrimary, static_cast<uint32_t>(commandBuffers.size())};

        if (device.device().allocateCommandBuffers(&allocInfo, commandBuffers.data()) != vk::Result::eSuccess) {
            throw std::runtime_error("failed to allocate command buffers!");
        }

        for (int i = 0; i < commandBuffers.size(); i++) {
            vk::CommandBufferBeginInfo beginInfo{};

            if (commandBuffers[i].begin(&beginInfo) != vk::Result::eSuccess) {
                throw std::runtime_error("failed to begin recording command buffer!");
            }

            vk::RenderPassBeginInfo renderPassInfo{swapChain.getRenderPass(), swapChain.getFrameBuffer(i), {{0, 0}, swapChain.getSwapChainExtent()}};

            std::array<vk::ClearValue, 2> clearValues{};
            clearValues[0].setColor(std::array<float, 4>{0.1f, 0.1f, 0.1f, 1.0f});
            clearValues[1].setDepthStencil({1.0f, 0});
            renderPassInfo.setClearValueCount(static_cast<uint32_t>(clearValues.size()));
            renderPassInfo.setPClearValues(clearValues.data());

            commandBuffers[i].beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

            pipeline->bind(commandBuffers[i]);
            commandBuffers[i].draw(3, 1, 0, 0);

            commandBuffers[i].endRenderPass();
            
            commandBuffers[i].end();
        }
    }

    void Engine::drawFrame() {
        uint32_t imageIndex;
        auto result = swapChain.acquireNextImage(&imageIndex);
        if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        result = swapChain.submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to present swap chain image!");
        }
    }
}