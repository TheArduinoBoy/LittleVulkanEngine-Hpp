#include "RenderSystem.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <stdexcept>

namespace Engine {

    struct PushConstantData {
        glm::mat4 transform{1.f};
        glm::mat4 normalMatrix{1.f};
    };

    RenderSystem::RenderSystem(Device& device, vk::RenderPass renderPass)
        : device{device} {
        createPipelineLayout();
        createPipeline(renderPass);
    }

    RenderSystem::~RenderSystem() {
        device.device().destroyPipelineLayout(pipelineLayout, nullptr);
    }

    void RenderSystem::createPipelineLayout() {
        vk::PushConstantRange pushConstantRange{{vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment}, 0, sizeof(PushConstantData)};

        vk::PipelineLayoutCreateInfo pipelineLayoutInfo{{}, 0, nullptr, 1, &pushConstantRange};
        if (device.device().createPipelineLayout(&pipelineLayoutInfo, nullptr, &pipelineLayout) != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void RenderSystem::createPipeline(vk::RenderPass renderPass) {
        assert(pipelineLayout && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipelineConfig{};
        Pipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        pipeline = std::make_unique<Pipeline>(
            device,
            "Shaders/Shader.vert.spv",
            "Shaders/Shader.frag.spv",
            pipelineConfig);
    }

    void RenderSystem::renderGameObjects(
        vk::CommandBuffer commandBuffer,
        std::vector<GameObject>& gameObjects,
        const Camera& camera) {
        pipeline->bind(commandBuffer);

        auto projectionView = camera.getProjection() * camera.getView();

        for (auto& obj : gameObjects) {
            PushConstantData push{};
            auto modelMatrix = obj.transform.mat4();
            push.transform = projectionView * modelMatrix;
            push.normalMatrix = obj.transform.normalMatrix();

            vkCmdPushConstants(
                commandBuffer,
                pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(PushConstantData),
                &push);
            obj.model->bind(commandBuffer);
            obj.model->draw(commandBuffer);
        }
    }

}
