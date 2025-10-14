#pragma once

#ifndef VULKAN_COMMON_FUNCTIONS
#define VULKAN_COMMON_FUNCTIONS

#include "vk_mem_alloc.h"

namespace VulkanCommonFunctions {
    VkCommandBuffer BeginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);
    void EndSingleTimeCommands(VkCommandBuffer commandBuffer, VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue);
    bool HasStencilComponent(VkFormat format);
}

#endif