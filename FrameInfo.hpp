#pragma once

#include "Camera.hpp"
#include "GameObject.hpp"

#include <vulkan/vulkan.hpp>

namespace Engine {
    struct FrameInfo {
        int frameIndex;
        float frameTime;
        vk::CommandBuffer commandBuffer;
        Camera &camera;
        vk::DescriptorSet globalDescriptorSet;
        GameObject::Map &gameObjects;
    };
}