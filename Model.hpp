#pragma once

#include "Device.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace Engine {
    class Model {
        public:
        struct Vertex {
            glm::vec2 position;
            glm::vec3 color;

            static std::vector<vk::VertexInputBindingDescription> getBindingDescriptions();
            static std::vector<vk::VertexInputAttributeDescription> getAttributeDescriptions();
        };

        Model(Device &device, const std::vector<Vertex> &vertices);
        ~Model();

        Model(const Model &) = delete;
        Model &operator=(const Model &) = delete;

        void bind(vk::CommandBuffer commandBuffer);
        void draw(vk::CommandBuffer commandBuffer);

        private:
        void createVertexBuffers(const std::vector<Vertex> &vertices);

        Device &device;
        vk::Buffer vertexBuffer;
        vk::DeviceMemory vertexBufferMemory;
        uint32_t vertexCount;
    };
}