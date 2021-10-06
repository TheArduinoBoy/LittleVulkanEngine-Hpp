#include "Model.hpp"

#include <cassert>
#include <cstring>

namespace Engine {
    Model::Model(Device &device, const std::vector<Vertex> &vertices) : device{device} {
        createVertexBuffers(vertices);
    }

    Model::~Model() {
        device.device().destroyBuffer(vertexBuffer, nullptr);
        device.device().freeMemory(vertexBufferMemory, nullptr);
    }

    void Model::createVertexBuffers(const std::vector<Vertex> &vertices) {
        vertexCount = static_cast<uint32_t>(vertices.size());
        assert(vertexCount >= 3 && "Vertex count must be at least 3");
        vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
        device.createBuffer(bufferSize, vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
        vertexBuffer, vertexBufferMemory);

        void *data;
        device.device().mapMemory(vertexBufferMemory, 0, bufferSize, {}, &data);
        memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
        device.device().unmapMemory(vertexBufferMemory);
    }

    void Model::bind(vk::CommandBuffer commandBuffer) {
        vk::Buffer buffers[] = {vertexBuffer};
        vk::DeviceSize offsets[] = {0};
        commandBuffer.bindVertexBuffers(0, 1, buffers, offsets);
    }

    void Model::draw(vk::CommandBuffer commandBuffer) {
        commandBuffer.draw(vertexCount, 1, 0, 0);
    }

    std::vector<vk::VertexInputBindingDescription> Model::Vertex::getBindingDescriptions() {
        std::vector<vk::VertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].setBinding(0);
        bindingDescriptions[0].setStride(sizeof(Vertex));
        bindingDescriptions[0].setInputRate(vk::VertexInputRate::eVertex);
        return bindingDescriptions;
    }

    std::vector<vk::VertexInputAttributeDescription> Model::Vertex::getAttributeDescriptions() {
        std::vector<vk::VertexInputAttributeDescription> attributeDescriptions(2);
        attributeDescriptions[0].setBinding(0);
        attributeDescriptions[0].setLocation(0);
        attributeDescriptions[0].setFormat(vk::Format::eR32G32Sfloat);
        attributeDescriptions[0].setOffset(offsetof(Vertex, position));

        attributeDescriptions[1].setBinding(0);
        attributeDescriptions[1].setLocation(1);
        attributeDescriptions[1].setFormat(vk::Format::eR32G32B32Sfloat);
        attributeDescriptions[1].setOffset(offsetof(Vertex, color));
        return attributeDescriptions;
    }
}