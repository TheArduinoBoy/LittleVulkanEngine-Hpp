#pragma once

#include "Camera.hpp"
#include "Device.hpp"
#include "GameObject.hpp"
#include "Pipeline.hpp"

// std
#include <memory>
#include <vector>

namespace Engine {
    class RenderSystem {
        public:
        RenderSystem(Device &device, vk::RenderPass renderPass);
        ~RenderSystem();

        RenderSystem(const RenderSystem &) = delete;
        RenderSystem &operator=(const RenderSystem &) = delete;

        void renderGameObjects(vk::CommandBuffer commandBuffer, std::vector<GameObject> &gameObjects, const Camera &camera);

        private:
        void createPipelineLayout();
        void createPipeline(vk::RenderPass renderPass);

        Device& device;
        std::unique_ptr<Pipeline> pipeline;
        vk::PipelineLayout pipelineLayout;
    };
}