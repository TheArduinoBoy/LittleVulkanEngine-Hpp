#include "RenderSystem.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <stdexcept>

namespace Engine {

    struct PushConstantData {
        glm::mat4 transform{1.f};
        alignas(16) glm::vec3 color;
    };

    RenderSystem::RenderSystem(Device &device, vk::RenderPass renderPass) : device{device} {
        createPipelineLayout();
        createPipeline(renderPass);
    }

    RenderSystem::~RenderSystem() { device.device().destroyPipelineLayout(pipelineLayout, nullptr); }

    void RenderSystem::createPipelineLayout() {
        vk::PushConstantRange pushConstantRange{vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(PushConstantData)};

        vk::PipelineLayoutCreateInfo pipelineLayoutInfo{{}, 0, nullptr, 1, &pushConstantRange};
        if (device.device().createPipelineLayout(&pipelineLayoutInfo, nullptr, &pipelineLayout) != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void RenderSystem::createPipeline(vk::RenderPass renderPass) {
        assert(pipelineLayout && "Cannot create pipeline before pipeline layout!");

        PipelineConfigInfo pipelineConfig{};
        Pipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        pipeline = std::make_unique<Pipeline>(device, "Shaders/Shader.vert.spv", "Shaders/Shader.frag.spv", pipelineConfig);
    }

    void RenderSystem::renderGameObjects(vk::CommandBuffer commandBuffer, std::vector<GameObject> &gameObjects, const Camera &camera) {
        pipeline->bind(commandBuffer);

        for(auto &obj : gameObjects) {
            obj.transform.rotation.y = glm::mod(obj.transform.rotation.y + 0.01f, glm::two_pi<float>());
            obj.transform.rotation.x = glm::mod(obj.transform.rotation.x + 0.005f, glm::two_pi<float>());

            PushConstantData push{};
            push.color = obj.color;
            push.transform = camera.getProjection() * obj.transform.mat4();

            commandBuffer.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(PushConstantData), &push);
            obj.model->bind(commandBuffer);
            obj.model->draw(commandBuffer);
        }
    }
}