#include "Engine.hpp"

// std
#include <array>
#include <stdexcept>

namespace Engine {

    Engine::Engine() {
        loadModels();
        createPipelineLayout();
        recreateSwapChain();
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

    void Engine::loadModels() {
        std::vector<Model::Vertex> vertices{
            {{0.f, -0.5f}, {1.f, 0.f, 0.f}},
            {{0.5f, 0.5f}, {0.f, 1.f, 0.f}},
            {{-0.5f, 0.5f}, {0.f, 0.f, 1.f}}};

        model = std::make_unique<Model>(device, vertices);
    }

    void Engine::createPipelineLayout() {
        vk::PipelineLayoutCreateInfo pipelineLayoutInfo{{}, 0, nullptr, 0, nullptr};
        if (device.device().createPipelineLayout(&pipelineLayoutInfo, nullptr, &pipelineLayout) != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void Engine::createPipeline() {
        assert(swapChain && "Cannot create pipeline before swap chain!");
        assert(pipelineLayout && "Cannot create pipeline before pipeline layout!");

        PipelineConfigInfo pipelineConfig{};
        Pipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = swapChain->getRenderPass();
        pipelineConfig.pipelineLayout = pipelineLayout;
        pipeline = std::make_unique<Pipeline>(device, "Shaders/Shader.vert.spv", "Shaders/Shader.frag.spv", pipelineConfig);
    }

    void Engine::recreateSwapChain() {
        auto extent = window.getExtent();
        while(extent.width == 0 || extent.height == 0) {
            extent = window.getExtent();
            glfwWaitEvents();
        }
        device.device().waitIdle();
        if(swapChain == nullptr) {
            swapChain = std::make_unique<SwapChain>(device, extent);
        } else {
            swapChain = std::make_unique<SwapChain>(device, extent, std::move(swapChain));
            if(swapChain->imageCount() != commandBuffers.size()) {
                freeCommandBuffers();
                createCommandBuffers();
            }
        }
        createPipeline();
    }

    void Engine::createCommandBuffers() {
        commandBuffers.resize(swapChain->imageCount());

        vk::CommandBufferAllocateInfo allocInfo{device.getCommandPool(), vk::CommandBufferLevel::ePrimary, static_cast<uint32_t>(commandBuffers.size())};

        if (device.device().allocateCommandBuffers(&allocInfo, commandBuffers.data()) != vk::Result::eSuccess) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void Engine::freeCommandBuffers() {
        device.device().freeCommandBuffers(device.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
        commandBuffers.clear();
    }

    void Engine::recordCommandBuffer(int imageIndex) {
        vk::CommandBufferBeginInfo beginInfo{};

        if (commandBuffers[imageIndex].begin(&beginInfo) != vk::Result::eSuccess) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        vk::RenderPassBeginInfo renderPassInfo{swapChain->getRenderPass(), swapChain->getFrameBuffer(imageIndex), {{0, 0}, swapChain->getSwapChainExtent()}};

        std::array<vk::ClearValue, 2> clearValues{};
        clearValues[0].setColor(std::array<float, 4>{0.1f, 0.1f, 0.1f, 1.0f});
        clearValues[1].setDepthStencil({1.0f, 0});
        renderPassInfo.setClearValueCount(static_cast<uint32_t>(clearValues.size()));
        renderPassInfo.setPClearValues(clearValues.data());

        commandBuffers[imageIndex].beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

        vk::Viewport viewport{0.f, 0.f, static_cast<float>(swapChain->getSwapChainExtent().width), static_cast<float>(swapChain->getSwapChainExtent().height), 0.f, 1.f};
        vk::Rect2D scissor{{0, 0}, swapChain->getSwapChainExtent()};
        commandBuffers[imageIndex].setViewport(0, 1, &viewport);
        commandBuffers[imageIndex].setScissor(0, 1, &scissor);

        pipeline->bind(commandBuffers[imageIndex]);
        model->bind(commandBuffers[imageIndex]);
        model->draw(commandBuffers[imageIndex]);

        commandBuffers[imageIndex].endRenderPass();
        
        commandBuffers[imageIndex].end();
    }

    void Engine::drawFrame() {
        uint32_t imageIndex;
        auto result = swapChain->acquireNextImage(&imageIndex);

        if(result == vk::Result::eErrorOutOfDateKHR) {
            recreateSwapChain();
            return;
        }

        if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        recordCommandBuffer(imageIndex);
        result = swapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
        if(result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || window.wasWindowResized()) {
            window.resetWindowResizedFlag();
            recreateSwapChain();
            return;
        }
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to present swap chain image!");
        }
    }
}