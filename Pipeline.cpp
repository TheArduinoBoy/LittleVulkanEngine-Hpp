#include "Pipeline.hpp"

// std
#include <cassert>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace Engine {

    Pipeline::Pipeline(
        Device& device,
        const std::string& vertFilepath,
        const std::string& fragFilepath,
        const PipelineConfigInfo& configInfo)
        : device{device} {
        createGraphicsPipeline(vertFilepath, fragFilepath, configInfo);
    }

    Pipeline::~Pipeline() {
        device.device().destroyShaderModule(vertShaderModule, nullptr);
        device.device().destroyShaderModule(fragShaderModule, nullptr);
        device.device().destroyPipeline(graphicsPipeline, nullptr);
    }

    std::vector<char> Pipeline::readFile(const std::string& filepath) {
        std::ifstream file{filepath, std::ios::ate | std::ios::binary};

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file: " + filepath);
        }

        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();
        return buffer;
    }

    void Pipeline::createGraphicsPipeline(
        const std::string& vertFilepath,
        const std::string& fragFilepath,
        const PipelineConfigInfo& configInfo) {
        assert(configInfo.pipelineLayout && "Cannot create graphics pipeline: no pipelineLayout provided in configInfo");
        assert(configInfo.renderPass && "Cannot create graphics pipeline: no renderPass provided in configInfo");

        auto vertCode = readFile(vertFilepath);
        auto fragCode = readFile(fragFilepath);

        createShaderModule(vertCode, &vertShaderModule);
        createShaderModule(fragCode, &fragShaderModule);

        vk::PipelineShaderStageCreateInfo shaderStages[2];
        shaderStages[0].setFlags({});
        shaderStages[0].setStage(vk::ShaderStageFlagBits::eVertex);
        shaderStages[0].setModule(vertShaderModule);
        shaderStages[0].setPName("main");
        shaderStages[0].setPNext(nullptr);
        shaderStages[0].setPSpecializationInfo(nullptr);
        shaderStages[1].setFlags({});
        shaderStages[1].setStage(vk::ShaderStageFlagBits::eFragment);
        shaderStages[1].setModule(fragShaderModule);
        shaderStages[1].setPName("main");
        shaderStages[1].setPNext(nullptr);
        shaderStages[1].setPSpecializationInfo(nullptr);

        vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.setVertexAttributeDescriptionCount(0);
        vertexInputInfo.setVertexBindingDescriptionCount(0);
        vertexInputInfo.setPVertexAttributeDescriptions(nullptr);
        vertexInputInfo.setPVertexBindingDescriptions(nullptr);

        vk::PipelineViewportStateCreateInfo viewportInfo;
        viewportInfo.viewportCount = 1;
        viewportInfo.pViewports = &configInfo.viewport;
        viewportInfo.scissorCount = 1;
        viewportInfo.pScissors = &configInfo.scissor;

        vk::GraphicsPipelineCreateInfo pipelineInfo{{}, 2, shaderStages, &vertexInputInfo, &configInfo.inputAssemblyInfo, nullptr, &viewportInfo, &configInfo.rasterizationInfo,
        &configInfo.multisampleInfo, &configInfo.depthStencilInfo, &configInfo.colorBlendInfo, nullptr, configInfo.pipelineLayout, configInfo.renderPass, configInfo.subpass, nullptr, -1};

        if (device.device().createGraphicsPipelines(nullptr, 1, &pipelineInfo, nullptr, &graphicsPipeline) != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create graphics pipeline");
        }
    }

    void Pipeline::createShaderModule(const std::vector<char>& code, vk::ShaderModule* shaderModule) {
        vk::ShaderModuleCreateInfo createInfo{{}, code.size(), reinterpret_cast<const uint32_t*>(code.data())};

        if (device.device().createShaderModule(&createInfo, nullptr, shaderModule) != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create shader module");
        }
    }

    void Pipeline::bind(vk::CommandBuffer commandBuffer) {
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);
    }

    void Pipeline::defaultPipelineConfigInfo(PipelineConfigInfo& configInfo, uint32_t width, uint32_t height) {
        configInfo.inputAssemblyInfo.setTopology(vk::PrimitiveTopology::eTriangleList);
        configInfo.inputAssemblyInfo.setPrimitiveRestartEnable(false);

        configInfo.viewport.setX(0.f);
        configInfo.viewport.setY(0.f);
        configInfo.viewport.setWidth(static_cast<float>(width));
        configInfo.viewport.setHeight(static_cast<float>(height));
        configInfo.viewport.setMinDepth(0.f);
        configInfo.viewport.setMaxDepth(1.f);

        configInfo.scissor.setOffset({0, 0});
        configInfo.scissor.setExtent({width, height});

        configInfo.rasterizationInfo.setDepthClampEnable(false);
        configInfo.rasterizationInfo.setRasterizerDiscardEnable(false);
        configInfo.rasterizationInfo.setPolygonMode(vk::PolygonMode::eFill);
        configInfo.rasterizationInfo.setLineWidth(1.f);
        configInfo.rasterizationInfo.setCullMode(vk::CullModeFlagBits::eNone);
        configInfo.rasterizationInfo.setFrontFace(vk::FrontFace::eClockwise);
        configInfo.rasterizationInfo.setDepthBiasEnable(false);
        configInfo.rasterizationInfo.setDepthBiasConstantFactor(0.f);
        configInfo.rasterizationInfo.setDepthBiasClamp(0.f);
        configInfo.rasterizationInfo.setDepthBiasSlopeFactor(0.f);

        configInfo.multisampleInfo.setSampleShadingEnable(false);
        configInfo.multisampleInfo.setRasterizationSamples(vk::SampleCountFlagBits::e1);
        configInfo.multisampleInfo.setMinSampleShading(1.f);
        configInfo.multisampleInfo.setPSampleMask(nullptr);
        configInfo.multisampleInfo.setAlphaToCoverageEnable(false);
        configInfo.multisampleInfo.setAlphaToOneEnable(false);

        configInfo.colorBlendAttachment.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB 
        | vk::ColorComponentFlagBits::eA);
        configInfo.colorBlendAttachment.setBlendEnable(false);
        configInfo.colorBlendAttachment.setSrcColorBlendFactor(vk::BlendFactor::eOne);
        configInfo.colorBlendAttachment.setDstColorBlendFactor(vk::BlendFactor::eZero);
        configInfo.colorBlendAttachment.setColorBlendOp(vk::BlendOp::eAdd);
        configInfo.colorBlendAttachment.setSrcAlphaBlendFactor(vk::BlendFactor::eOne);
        configInfo.colorBlendAttachment.setDstAlphaBlendFactor(vk::BlendFactor::eZero);
        configInfo.colorBlendAttachment.setAlphaBlendOp(vk::BlendOp::eAdd);

        configInfo.colorBlendInfo.setLogicOpEnable(false);
        configInfo.colorBlendInfo.setLogicOp(vk::LogicOp::eCopy);
        configInfo.colorBlendInfo.setAttachmentCount(1);
        configInfo.colorBlendInfo.setPAttachments(&configInfo.colorBlendAttachment);
        configInfo.colorBlendInfo.setBlendConstants({0.f, 0.f, 0.f, 0.f});

        configInfo.depthStencilInfo.setDepthTestEnable(true);
        configInfo.depthStencilInfo.setDepthWriteEnable(true);
        configInfo.depthStencilInfo.setDepthCompareOp(vk::CompareOp::eLess);
        configInfo.depthStencilInfo.setDepthBoundsTestEnable(false);
        configInfo.depthStencilInfo.setMinDepthBounds(0.f);
        configInfo.depthStencilInfo.setMaxDepthBounds(1.f);
        configInfo.depthStencilInfo.setStencilTestEnable(false);
        configInfo.depthStencilInfo.setFront({});
        configInfo.depthStencilInfo.setBack({});
    }

}