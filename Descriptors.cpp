#include "Descriptors.hpp"

#include <cassert>
#include <stdexcept>
 
namespace Engine {
 
// *************** Descriptor Set Layout Builder *********************
 
DescriptorSetLayout::Builder &DescriptorSetLayout::Builder::addBinding(uint32_t binding, vk::DescriptorType descriptorType, vk::ShaderStageFlags stageFlags, uint32_t count) {
    assert(bindings.count(binding) == 0 && "Binding already in use");

    vk::DescriptorSetLayoutBinding layoutBinding{binding, descriptorType, count, stageFlags};
    bindings[binding] = layoutBinding;
    return *this;
}
 
std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Builder::build() const {
    return std::make_unique<DescriptorSetLayout>(device, bindings);
}
 
// *************** Descriptor Set Layout *********************
 
DescriptorSetLayout::DescriptorSetLayout(Engine::Device &device, std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings) : device{device}, bindings{bindings} {
    std::vector<vk::DescriptorSetLayoutBinding> setLayoutBindings{};
    for (auto kv : bindings) {
        setLayoutBindings.push_back(kv.second);
    }
    
    vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{{}, static_cast<uint32_t>(setLayoutBindings.size()), setLayoutBindings.data()};
    
    if (device.device().createDescriptorSetLayout(&descriptorSetLayoutInfo, nullptr, &descriptorSetLayout) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}
 
DescriptorSetLayout::~DescriptorSetLayout() {
    device.device().destroyDescriptorSetLayout(descriptorSetLayout, nullptr);
}
 
// *************** Descriptor Pool Builder *********************
 
DescriptorPool::Builder &DescriptorPool::Builder::addPoolSize(vk::DescriptorType descriptorType, uint32_t count) {
    poolSizes.push_back({descriptorType, count});
    return *this;
}
 
DescriptorPool::Builder &DescriptorPool::Builder::setPoolFlags(vk::DescriptorPoolCreateFlags flags) {
    poolFlags = flags;
    return *this;
}

DescriptorPool::Builder &DescriptorPool::Builder::setMaxSets(uint32_t count) {
    maxSets = count;
    return *this;
}
 
std::unique_ptr<DescriptorPool> DescriptorPool::Builder::build() const {
    return std::make_unique<DescriptorPool>(device, maxSets, poolFlags, poolSizes);
}
 
// *************** Descriptor Pool *********************
 
DescriptorPool::DescriptorPool(Engine::Device &device, uint32_t maxSets, vk::DescriptorPoolCreateFlags poolFlags, const std::vector<vk::DescriptorPoolSize> &poolSizes) : device{device} {
    vk::DescriptorPoolCreateInfo descriptorPoolInfo{poolFlags, maxSets, static_cast<uint32_t>(poolSizes.size()), poolSizes.data()};
    
    if (device.device().createDescriptorPool(&descriptorPoolInfo, nullptr, &descriptorPool) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}
 
DescriptorPool::~DescriptorPool() {
    device.device().destroyDescriptorPool(descriptorPool, nullptr);
}
 
bool DescriptorPool::allocateDescriptor(const vk::DescriptorSetLayout descriptorSetLayout, vk::DescriptorSet &descriptor) const {
    vk::DescriptorSetAllocateInfo allocInfo{descriptorPool, 1, &descriptorSetLayout};
    
    // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
    // a new pool whenever an old pool fills up. But this is beyond our current scope
    if (device.device().allocateDescriptorSets(&allocInfo, &descriptor) != vk::Result::eSuccess) {
        return false;
    }
    return true;
}
 
void DescriptorPool::freeDescriptors(std::vector<vk::DescriptorSet> &descriptors) const {
    device.device().freeDescriptorSets(descriptorPool, static_cast<uint32_t>(descriptors.size()), descriptors.data());
}
 
void DescriptorPool::resetPool() {
    device.device().resetDescriptorPool(descriptorPool, {});
}
 
// *************** Descriptor Writer *********************
 
DescriptorWriter::DescriptorWriter(DescriptorSetLayout &setLayout, DescriptorPool &pool) : setLayout{setLayout}, pool{pool} {}
 
DescriptorWriter &DescriptorWriter::writeBuffer(uint32_t binding, vk::DescriptorBufferInfo *bufferInfo) {
    assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");
    
    auto &bindingDescription = setLayout.bindings[binding];
    
    assert(
        bindingDescription.descriptorCount == 1 &&
        "Binding single descriptor info, but binding expects multiple");
    
    vk::WriteDescriptorSet write{};
    write.setDescriptorType(bindingDescription.descriptorType);
    write.setDstBinding(binding);
    write.setPBufferInfo(bufferInfo);
    write.setDescriptorCount(1);
    
    writes.push_back(write);
    return *this;
}
 
DescriptorWriter &DescriptorWriter::writeImage(uint32_t binding, vk::DescriptorImageInfo *imageInfo) {
    assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");
    
    auto &bindingDescription = setLayout.bindings[binding];
    
    assert(
        bindingDescription.descriptorCount == 1 &&
        "Binding single descriptor info, but binding expects multiple");
    
    vk::WriteDescriptorSet write{};
    write.setDescriptorType(bindingDescription.descriptorType);
    write.setDstBinding(binding);
    write.setPImageInfo(imageInfo);
    write.setDescriptorCount(1);
    
    writes.push_back(write);
    return *this;
}
 
bool DescriptorWriter::build(vk::DescriptorSet &set) {
    bool success = pool.allocateDescriptor(setLayout.getDescriptorSetLayout(), set);
    if (!success) {
        return false;
    }
    overwrite(set);
    return true;
}
 
void DescriptorWriter::overwrite(vk::DescriptorSet &set) {
    for (auto &write : writes) {
        write.dstSet = set;
    }
    pool.device.device().updateDescriptorSets(writes.size(), writes.data(), 0, nullptr);
}
}