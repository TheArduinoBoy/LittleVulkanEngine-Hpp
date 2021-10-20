#pragma once

#include "Device.hpp"
 
// std
#include <memory>
#include <unordered_map>
#include <vector>
 
namespace Engine {
 
    class DescriptorSetLayout {
        public:
        class Builder {
        public:
            Builder(Device &device) : device{device} {}
        
            Builder &addBinding(
                uint32_t binding,
                vk::DescriptorType descriptorType,
                vk::ShaderStageFlags stageFlags,
                uint32_t count = 1);
            std::unique_ptr<DescriptorSetLayout> build() const;
        
        private:
            Device &device;
            std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings{};
        };
    
        DescriptorSetLayout(
            Device &Device, std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings);
        ~DescriptorSetLayout();
        DescriptorSetLayout(const DescriptorSetLayout &) = delete;
        DescriptorSetLayout &operator=(const DescriptorSetLayout &) = delete;
        
        vk::DescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }
        
        private:
        Device &device;
        vk::DescriptorSetLayout descriptorSetLayout;
        std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings;
        
        friend class DescriptorWriter;
    };
    
    class DescriptorPool {
        public:
        class Builder {
        public:
            Builder(Device& device) : device{device} {}
        
            Builder &addPoolSize(vk::DescriptorType descriptorType, uint32_t count);
            Builder &setPoolFlags(vk::DescriptorPoolCreateFlags flags);
            Builder &setMaxSets(uint32_t count);
            std::unique_ptr<DescriptorPool> build() const;
        
        private:
            Device &device;
            std::vector<vk::DescriptorPoolSize> poolSizes{};
            uint32_t maxSets = 1000;
            vk::DescriptorPoolCreateFlags poolFlags = {};        
        };
    
        DescriptorPool(
            Device &device,
            uint32_t maxSets,
            vk::DescriptorPoolCreateFlags poolFlags,
            const std::vector<vk::DescriptorPoolSize> &poolSizes);
        ~DescriptorPool();
        DescriptorPool(const DescriptorPool &) = delete;
        DescriptorPool &operator=(const DescriptorPool &) = delete;
        
        bool allocateDescriptor(
            const vk::DescriptorSetLayout descriptorSetLayout, vk::DescriptorSet &descriptor) const;
        
        void freeDescriptors(std::vector<vk::DescriptorSet> &descriptors) const;
        
        void resetPool();
        
        private:
        Device &device;
        vk::DescriptorPool descriptorPool;
        
        friend class DescriptorWriter;
    };
        
    class DescriptorWriter {
        public:
        DescriptorWriter(DescriptorSetLayout &setLayout, DescriptorPool &pool);
        
        DescriptorWriter &writeBuffer(uint32_t binding, vk::DescriptorBufferInfo *bufferInfo);
        DescriptorWriter &writeImage(uint32_t binding, vk::DescriptorImageInfo *imageInfo);
        
        bool build(vk::DescriptorSet &set);
        void overwrite(vk::DescriptorSet &set);
        
        private:
        DescriptorSetLayout &setLayout;
        DescriptorPool &pool;
        std::vector<vk::WriteDescriptorSet> writes;
    };
}