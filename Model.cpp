#include "Model.hpp"

#include <cassert>
#include <cstring>

namespace Engine {
    Model::Model(Device &device, const Model::Builder &builder) : device{device} {
        createVertexBuffers(builder.vertices);
        createIndexBuffers(builder.indices);
    }

    Model::~Model() {
        device.device().destroyBuffer(vertexBuffer, nullptr);
        device.device().freeMemory(vertexBufferMemory, nullptr);

        if(hasIndexBuffer) {
            device.device().destroyBuffer(indexBuffer, nullptr);
            device.device().freeMemory(indexBufferMemory, nullptr);
        }
    }

    void Model::createVertexBuffers(const std::vector<Vertex> &vertices) {
        vertexCount = static_cast<uint32_t>(vertices.size());
        assert(vertexCount >= 3 && "Vertex count must be at least 3");
        vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;

        vk::Buffer stagingBuffer;
        vk::DeviceMemory stagingBufferMemory;
        device.createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
        stagingBuffer, stagingBufferMemory);
        
        void *data;
        device.device().mapMemory(stagingBufferMemory, 0, bufferSize, {}, &data);
        memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
        device.device().unmapMemory(stagingBufferMemory);

        device.createBuffer(bufferSize, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal,
        vertexBuffer, vertexBufferMemory);

        device.copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

        device.device().destroyBuffer(stagingBuffer, nullptr);
        device.device().freeMemory(stagingBufferMemory, nullptr);
    }

    void Model::createIndexBuffers(const std::vector<uint32_t> &indices) {
        indexCount = static_cast<uint32_t>(indices.size());
        hasIndexBuffer = indexCount > 0;
        if(!hasIndexBuffer) return;
        vk::DeviceSize bufferSize = sizeof(indices[0]) * indexCount;

        vk::Buffer stagingBuffer;
        vk::DeviceMemory stagingBufferMemory;
        device.createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
        stagingBuffer, stagingBufferMemory);
        
        void *data;
        device.device().mapMemory(stagingBufferMemory, 0, bufferSize, {}, &data);
        memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
        device.device().unmapMemory(stagingBufferMemory);

        device.createBuffer(bufferSize, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal,
        indexBuffer, indexBufferMemory);

        device.copyBuffer(stagingBuffer, indexBuffer, bufferSize);

        device.device().destroyBuffer(stagingBuffer, nullptr);
        device.device().freeMemory(stagingBufferMemory, nullptr);
    }

    void Model::bind(vk::CommandBuffer commandBuffer) {
        vk::Buffer buffers[] = {vertexBuffer};
        vk::DeviceSize offsets[] = {0};
        commandBuffer.bindVertexBuffers(0, 1, buffers, offsets);

        if(hasIndexBuffer) {
            commandBuffer.bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint32);
        }
    }

    void Model::draw(vk::CommandBuffer commandBuffer) {
        if(hasIndexBuffer) {
            commandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);
        } else {
            commandBuffer.draw(vertexCount, 1, 0, 0);
        }
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
        attributeDescriptions[0].setFormat(vk::Format::eR32G32B32Sfloat);
        attributeDescriptions[0].setOffset(offsetof(Vertex, position));

        attributeDescriptions[1].setBinding(0);
        attributeDescriptions[1].setLocation(1);
        attributeDescriptions[1].setFormat(vk::Format::eR32G32B32Sfloat);
        attributeDescriptions[1].setOffset(offsetof(Vertex, color));
        return attributeDescriptions;
    }
}