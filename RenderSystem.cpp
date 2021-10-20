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
        glm::mat4 modelMatrix{1.f};
        glm::mat4 normalMatrix{1.f};
    };

    RenderSystem::RenderSystem(Device& device, vk::RenderPass renderPass, vk::DescriptorSetLayout globalSetLayout)
        : device{device} {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
    }

    RenderSystem::~RenderSystem() {
        device.device().destroyPipelineLayout(pipelineLayout, nullptr);
    }

    void RenderSystem::createPipelineLayout(vk::DescriptorSetLayout globalSetLayout) {
        vk::PushConstantRange pushConstantRange{{vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment}, 0, sizeof(PushConstantData)};

        std::vector<vk::DescriptorSetLayout> descriptorSetLayout{globalSetLayout};

        vk::PipelineLayoutCreateInfo pipelineLayoutInfo{{}, static_cast<uint32_t>(descriptorSetLayout.size()), descriptorSetLayout.data(), 1, &pushConstantRange};
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
            "./Shaders/Shader.vert.spv",
            "./Shaders/Shader.frag.spv",
            pipelineConfig);
    }

    void RenderSystem::renderGameObjects(FrameInfo& frameInfo, std::vector<GameObject>& gameObjects) {
        pipeline->bind(frameInfo.commandBuffer);

        frameInfo.commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);

        for (auto& obj : gameObjects) {
            PushConstantData push{};
            push.modelMatrix = obj.transform.mat4();
            push.normalMatrix = obj.transform.normalMatrix();

            vkCmdPushConstants(
                frameInfo.commandBuffer,
                pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(PushConstantData),
                &push);
            obj.model->bind(frameInfo.commandBuffer);
            obj.model->draw(frameInfo.commandBuffer);
        }
    }

}
