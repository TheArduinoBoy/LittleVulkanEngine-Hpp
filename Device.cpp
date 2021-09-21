#include "Device.hpp"

// std headers
#include <cstring>
#include <iostream>
#include <set>
#include <unordered_set>

namespace Engine {

    // local callback functions
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return false;
    }

    // class member functions
    Device::Device(Window &window) : window{window} {
        createInstance();
        setupDebugMessenger();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createCommandPool();
    }

    Device::~Device() {
        device_.destroyCommandPool(commandPool, nullptr);
        device_.destroy(nullptr);

        if (enableValidationLayers) {
            instance.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, vk::DispatchLoaderDynamic(instance, vkGetInstanceProcAddr));
        }

        instance.destroySurfaceKHR(surface_, nullptr);
        instance.destroy(nullptr);
    }

    void Device::createInstance() {
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        vk::ApplicationInfo appInfo{"Game", VK_MAKE_VERSION(1, 0, 0), "Engine", VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_2};

        vk::InstanceCreateInfo createInfo{{}, &appInfo};

        auto extensions = getRequiredExtensions();
        createInfo.setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()));
        createInfo.setPpEnabledExtensionNames(extensions.data());

        vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo;
        if (enableValidationLayers) {
            createInfo.setEnabledLayerCount(static_cast<uint32_t>(validationLayers.size()));
            createInfo.setPpEnabledLayerNames(validationLayers.data());

            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.setPNext((VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo);
        } else {
            createInfo.setEnabledLayerCount(0);
            createInfo.setPNext(nullptr);
        }

        if (vk::createInstance(&createInfo, nullptr, &instance) != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create instance!");
        }

        hasSdlRequiredInstanceExtensions();
    }

    void Device::pickPhysicalDevice() {
        uint32_t deviceCount = 0;
        instance.enumeratePhysicalDevices(&deviceCount, nullptr);
        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }
        std::cout << "Device count: " << deviceCount << std::endl;
        std::vector<vk::PhysicalDevice> devices(deviceCount);
        instance.enumeratePhysicalDevices(&deviceCount, devices.data());

        for (const auto &device : devices) {
            if (isDeviceSuitable(device)) {
                physicalDevice = device;
                break;
            }
        }

        if (!physicalDevice) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }

        physicalDevice.getProperties(&properties);
        std::cout << "physical device: " << properties.deviceName << std::endl;
    }

    void Device::createLogicalDevice() {
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily, indices.presentFamily};

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            vk::DeviceQueueCreateInfo queueCreateInfo{{}, queueFamily, 1, &queuePriority};
            queueCreateInfos.push_back(queueCreateInfo);
        }

        vk::PhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.setSamplerAnisotropy(true);

        vk::DeviceCreateInfo createInfo{};

        createInfo.setQueueCreateInfoCount(static_cast<uint32_t>(queueCreateInfos.size()));
        createInfo.setPQueueCreateInfos(queueCreateInfos.data());

        createInfo.setPEnabledFeatures(&deviceFeatures);
        createInfo.setEnabledExtensionCount(static_cast<uint32_t>(deviceExtensions.size()));
        createInfo.setPpEnabledExtensionNames(deviceExtensions.data());

        // might not really be necessary anymore because device specific validation layers
        // have been deprecated
        if (enableValidationLayers) {
            createInfo.setEnabledLayerCount(static_cast<uint32_t>(validationLayers.size()));
            createInfo.setPpEnabledLayerNames(validationLayers.data());
        } else {
            createInfo.setEnabledLayerCount(0);
        }

        if (physicalDevice.createDevice(&createInfo, nullptr, &device_) != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create logical device!");
        }

        device_.getQueue(indices.graphicsFamily, 0, &graphicsQueue_);
        device_.getQueue(indices.presentFamily, 0, &presentQueue_);
    }

    void Device::createCommandPool() {
        QueueFamilyIndices queueFamilyIndices = findPhysicalQueueFamilies();

        vk::CommandPoolCreateInfo poolInfo{{vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer}, queueFamilyIndices.graphicsFamily};

        if (device_.createCommandPool(&poolInfo, nullptr, &commandPool) != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create command pool!");
        }
    }

    void Device::createSurface() { window.createWindowSurface(instance, &surface_); }

    bool Device::isDeviceSuitable(vk::PhysicalDevice device) {
        QueueFamilyIndices indices = findQueueFamilies(device);

        bool extensionsSupported = checkDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        vk::PhysicalDeviceFeatures supportedFeatures;
        device.getFeatures(&supportedFeatures);

        return indices.isComplete() && extensionsSupported && swapChainAdequate &&
                supportedFeatures.samplerAnisotropy;
    }

    void Device::populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT &createInfo) {
        createInfo.setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
        createInfo.setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance);
        createInfo.setPfnUserCallback((PFN_vkDebugUtilsMessengerCallbackEXT)debugCallback);
        createInfo.setPUserData(nullptr);
    }

    void Device::setupDebugMessenger() {
        if (!enableValidationLayers) return;
        vk::DebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);
        if (instance.createDebugUtilsMessengerEXT(&createInfo, nullptr, &debugMessenger, vk::DispatchLoaderDynamic(instance, vkGetInstanceProcAddr)) != vk::Result::eSuccess) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    bool Device::checkValidationLayerSupport() {
        uint32_t layerCount;
        vk::enumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<vk::LayerProperties> availableLayers(layerCount);
        vk::enumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char *layerName : validationLayers) {
            bool layerFound = false;

            for (const auto &layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    std::vector<const char *> Device::getRequiredExtensions() {
        uint32_t extensionCount = 0;
        SDL_Vulkan_GetInstanceExtensions(window.getSDLwindow(), &extensionCount, nullptr);
        std::vector<const char *> extensions(extensionCount);
        SDL_Vulkan_GetInstanceExtensions(window.getSDLwindow(), &extensionCount, extensions.data());


        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    void Device::hasSdlRequiredInstanceExtensions() {
        uint32_t extensionCount = 0;
        vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<vk::ExtensionProperties> extensions(extensionCount);
        vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

        std::cout << "available extensions:" << std::endl;
        std::unordered_set<std::string> available;
        for (const auto &extension : extensions) {
            std::cout << "\t" << extension.extensionName << std::endl;
            available.insert(extension.extensionName);
        }

        std::cout << "required extensions:" << std::endl;
        auto requiredExtensions = getRequiredExtensions();
        for (const auto &required : requiredExtensions) {
            std::cout << "\t" << required << std::endl;
            if (available.find(required) == available.end()) {
                throw std::runtime_error("Missing required sdl extension");
            }
        }
    }

    bool Device::checkDeviceExtensionSupport(vk::PhysicalDevice device) {
        uint32_t extensionCount;
        device.enumerateDeviceExtensionProperties(nullptr, &extensionCount, nullptr);

        std::vector<vk::ExtensionProperties> availableExtensions(extensionCount);
        device.enumerateDeviceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto &extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    QueueFamilyIndices Device::findQueueFamilies(vk::PhysicalDevice device) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        device.getQueueFamilyProperties(&queueFamilyCount, nullptr);

        std::vector<vk::QueueFamilyProperties> queueFamilies(queueFamilyCount);
        device.getQueueFamilyProperties(&queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto &queueFamily : queueFamilies) {
            if (queueFamily.queueCount > 0 && queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
                indices.graphicsFamily = i;
                indices.graphicsFamilyHasValue = true;
            }
            vk::Bool32 presentSupport = false;
            device.getSurfaceSupportKHR(i, surface_, &presentSupport);
            if (queueFamily.queueCount > 0 && presentSupport) {
                indices.presentFamily = i;
                indices.presentFamilyHasValue = true;
            }
            if (indices.isComplete()) {
                break;
            }

            i++;
        }

        return indices;
    }

    SwapChainSupportDetails Device::querySwapChainSupport(vk::PhysicalDevice device) {
        SwapChainSupportDetails details;
        device.getSurfaceCapabilitiesKHR(surface_, &details.capabilities);

        uint32_t formatCount;
        device.getSurfaceFormatsKHR(surface_, &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            device.getSurfaceFormatsKHR(surface_, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        device.getSurfacePresentModesKHR(surface_, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            device.getSurfacePresentModesKHR(surface_, &presentModeCount, details.presentModes.data());
        }
        return details;
    }

    vk::Format Device::findSupportedFormat(
        const std::vector<vk::Format> &candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) {
        for (vk::Format format : candidates) {
            vk::FormatProperties props;
            physicalDevice.getFormatProperties(format, &props);

            if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
                return format;
            } else if (
                tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }
        throw std::runtime_error("failed to find supported format!");
    }

    uint32_t Device::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
        vk::PhysicalDeviceMemoryProperties memProperties;
        physicalDevice.getMemoryProperties(&memProperties);
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) &&
                (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

    void Device::createBuffer(
        vk::DeviceSize size,
        vk::BufferUsageFlags usage,
        vk::MemoryPropertyFlags properties,
        vk::Buffer &buffer,
        vk::DeviceMemory &bufferMemory) {
        vk::BufferCreateInfo bufferInfo{{}, size, usage, vk::SharingMode::eExclusive};

        if (device_.createBuffer(&bufferInfo, nullptr, &buffer) != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create vertex buffer!");
        }

        vk::MemoryRequirements memRequirements;
        device_.getBufferMemoryRequirements(buffer, &memRequirements);

        vk::MemoryAllocateInfo allocInfo{memRequirements.size, findMemoryType(memRequirements.memoryTypeBits, properties)};

        if (device_.allocateMemory(&allocInfo, nullptr, &bufferMemory) != vk::Result::eSuccess) {
            throw std::runtime_error("failed to allocate vertex buffer memory!");
        }

        device_.bindBufferMemory(buffer, bufferMemory, 0);
    }

    vk::CommandBuffer Device::beginSingleTimeCommands() {
        vk::CommandBufferAllocateInfo allocInfo{commandPool, vk::CommandBufferLevel::ePrimary, 1};

        vk::CommandBuffer commandBuffer;
        device_.allocateCommandBuffers(&allocInfo, &commandBuffer);

        vk::CommandBufferBeginInfo beginInfo{{vk::CommandBufferUsageFlagBits::eOneTimeSubmit}};
        commandBuffer.begin(&beginInfo);
        return commandBuffer;
    }

    void Device::endSingleTimeCommands(vk::CommandBuffer commandBuffer) {
        commandBuffer.end();

        vk::SubmitInfo submitInfo{0, nullptr, {}, 1, &commandBuffer};

        graphicsQueue_.submit(1, &submitInfo, nullptr);
        graphicsQueue_.waitIdle();

        device_.freeCommandBuffers(commandPool, 1, &commandBuffer);
    }

    void Device::copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size) {
        vk::CommandBuffer commandBuffer = beginSingleTimeCommands();

        vk::BufferCopy copyRegion{0, 0, size};
        commandBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);

        endSingleTimeCommands(commandBuffer);
    }

    void Device::copyBufferToImage(
        vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height, uint32_t layerCount) {
        vk::CommandBuffer commandBuffer = beginSingleTimeCommands();

        vk::BufferImageCopy region{0, 0, 0, {vk::ImageAspectFlagBits::eColor, 0, 0, layerCount}, {0, 0, 0}, {width, height, 1}};

        commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, 1, &region);

        endSingleTimeCommands(commandBuffer);
    }

    void Device::createImageWithInfo(
        const vk::ImageCreateInfo &imageInfo,
        vk::MemoryPropertyFlags properties,
        vk::Image &image,
        vk::DeviceMemory &imageMemory) {
        if (device_.createImage(&imageInfo, nullptr, &image) != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create image!");
        }

        vk::MemoryRequirements memRequirements;
        device_.getImageMemoryRequirements(image, &memRequirements);

        vk::MemoryAllocateInfo allocInfo{memRequirements.size, findMemoryType(memRequirements.memoryTypeBits, properties)};

        if (device_.allocateMemory(&allocInfo, nullptr, &imageMemory) != vk::Result::eSuccess) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        device_.bindImageMemory(image, imageMemory, 0);
    }
}