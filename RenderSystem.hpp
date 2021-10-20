#pragma once

#include "Camera.hpp"
#include "Device.hpp"
#include "GameObject.hpp"
#include "Pipeline.hpp"
#include "FrameInfo.hpp"

// std
#include <memory>
#include <vector>

namespace Engine {
    class RenderSystem {
        public:
        RenderSystem(Device &device, vk::RenderPass renderPass, vk::DescriptorSetLayout globalSetLayout);
        ~RenderSystem();

        RenderSystem(const RenderSystem &) = delete;
        RenderSystem &operator=(const RenderSystem &) = delete;

        void renderGameObjects(
            FrameInfo& frameInfo,
            std::vector<GameObject> &gameObjects);

        private:
        void createPipelineLayout(vk::DescriptorSetLayout globalSetLayout);
        void createPipeline(vk::RenderPass renderPass);

        Device &device;

        std::unique_ptr<Pipeline> pipeline;
        vk::PipelineLayout pipelineLayout;
    };
}  // namespace lve
