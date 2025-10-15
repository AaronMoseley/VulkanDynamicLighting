#pragma once

#ifndef VULKAN_COMMON_FUNCTIONS
#define VULKAN_COMMON_FUNCTIONS

#include "vk_mem_alloc.h"

#include <optional>
#include <vector>

namespace VulkanCommonFunctions {
    struct QueueFamilyIndices {
        std::optional<uint32_t> m_graphicsFamily;
        std::optional<uint32_t> m_presentFamily;

        bool IsComplete() {
            return m_graphicsFamily.has_value() && m_presentFamily.has_value();
        }
    };

    VkCommandBuffer BeginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);
    void EndSingleTimeCommands(VkCommandBuffer commandBuffer, VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue);
    bool HasStencilComponent(VkFormat format);
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
}

#endif