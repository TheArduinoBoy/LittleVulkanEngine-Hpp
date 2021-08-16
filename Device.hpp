#pragma once

#include "Window.hpp"

// std lib headers
#include <string>
#include <vector>

namespace Engine {

    struct SwapChainSupportDetails {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentModes;
    };

    struct QueueFamilyIndices {
        uint32_t graphicsFamily;
        uint32_t presentFamily;
        bool graphicsFamilyHasValue = false;
        bool presentFamilyHasValue = false;
        bool isComplete() { return graphicsFamilyHasValue && presentFamilyHasValue; }
    };

    class Device {
        public:
        #ifdef NDEBUG
            const bool enableValidationLayers = false;
        #else
            const bool enableValidationLayers = true;
        #endif

        Device(Window &window);
        ~Device();

        // Not copyable or movable
        Device(const Device &) = delete;
        Device &operator=(const Device &) = delete;
        Device(Device &&) = delete;
        Device &operator=(Device &&) = delete;

        vk::CommandPool getCommandPool() { return commandPool; }
        vk::Device device() { return device_; }
        vk::SurfaceKHR surface() { return surface_; }
        vk::Queue graphicsQueue() { return graphicsQueue_; }
        vk::Queue presentQueue() { return presentQueue_; }

        SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(physicalDevice); }
        uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
        QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(physicalDevice); }
        vk::Format findSupportedFormat(
            const std::vector<vk::Format> &candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);

        // Buffer Helper Functions
        void createBuffer(
            vk::DeviceSize size,
            vk::BufferUsageFlags usage,
            vk::MemoryPropertyFlags properties,
            vk::Buffer &buffer,
            vk::DeviceMemory &bufferMemory);
        vk::CommandBuffer beginSingleTimeCommands();
        void endSingleTimeCommands(vk::CommandBuffer commandBuffer);
        void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);
        void copyBufferToImage(
            vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height, uint32_t layerCount);

        void createImageWithInfo(
            const vk::ImageCreateInfo &imageInfo,
            vk::MemoryPropertyFlags properties,
            vk::Image &image,
            vk::DeviceMemory &imageMemory);

        vk::PhysicalDeviceProperties properties;

        private:
        void createInstance();
        void setupDebugMessenger();
        void createSurface();
        void pickPhysicalDevice();
        void createLogicalDevice();
        void createCommandPool();

        // helper functions
        bool isDeviceSuitable(vk::PhysicalDevice device);
        std::vector<const char *> getRequiredExtensions();
        bool checkValidationLayerSupport();
        QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device);
        void populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT &createInfo);
        void hasGflwRequiredInstanceExtensions();
        bool checkDeviceExtensionSupport(vk::PhysicalDevice device);
        SwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice device);

        vk::Instance instance;
        vk::DebugUtilsMessengerEXT debugMessenger;
        vk::PhysicalDevice physicalDevice;
        Window &window;
        vk::CommandPool commandPool;

        vk::Device device_;
        vk::SurfaceKHR surface_;
        vk::Queue graphicsQueue_;
        vk::Queue presentQueue_;

        const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
        const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    };
}
