#pragma once

#ifndef VULKAN_COMMON_FUNCTIONS
#define VULKAN_COMMON_FUNCTIONS

#include "vk_mem_alloc.h"

#include "glm/glm.hpp"

#include <optional>
#include <vector>
#include <array>

namespace VulkanCommonFunctions {
    struct GlobalInfo {
        glm::mat4 view;
        glm::mat4 proj;
        glm::vec3 cameraPosition;
        uint32_t lightCount;
    };

    struct LightInfo {
        glm::vec4 lightPosition;
        glm::vec4 lightColor;

        glm::vec4 lightAmbient;
        glm::vec4 lightDiffuse;
        glm::vec4 lightSpecular;

        float maxLightDistance;
    };

    struct InstanceInfo {
        glm::mat4 modelMatrix;
        glm::mat4 modelMatrixInverse;

        glm::vec3 ambient;
        glm::vec3 diffuse;
        glm::vec3 specular;
        float shininess;
        uint32_t lit;

        uint32_t textured;
        uint32_t textureIndex;
    };

    struct Vertex {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec2 texCoord;

        static std::array<VkVertexInputBindingDescription, 2> GetBindingDescriptions() {
            std::array<VkVertexInputBindingDescription, 2> result;

            result[0].binding = 0;
            result[0].stride = sizeof(Vertex);
            result[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            result[1].binding = 1;
            result[1].stride = sizeof(InstanceInfo);
            result[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

            return result;
        }

        static std::array<VkVertexInputAttributeDescription, 18> GetAttributeDescriptions() {
            std::array<VkVertexInputAttributeDescription, 18> attributeDescriptions{};

            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(Vertex, pos);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(Vertex, normal);

            attributeDescriptions[2].binding = 0;
            attributeDescriptions[2].location = 2;
            attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

            for (uint32_t i = 3; i < 7; i++) {
                attributeDescriptions[i].binding = 1; // instance buffer binding index
                attributeDescriptions[i].location = i; // consecutive locations (e.g., 2,3,4,5)
                attributeDescriptions[i].format = VK_FORMAT_R32G32B32A32_SFLOAT; // vec4 per row
                attributeDescriptions[i].offset = sizeof(glm::vec4) * (i - 3);
            }

            for (uint32_t i = 7; i < 11; i++) {
                attributeDescriptions[i].binding = 1; // instance buffer binding index
                attributeDescriptions[i].location = i; // consecutive locations (e.g., 2,3,4,5)
                attributeDescriptions[i].format = VK_FORMAT_R32G32B32A32_SFLOAT; // vec4 per row
                attributeDescriptions[i].offset = sizeof(glm::mat4) + sizeof(glm::vec4) * (i - 7);
            }

            for (uint32_t i = 11; i < 14; i++)
            {
                attributeDescriptions[i].binding = 1;
                attributeDescriptions[i].location = i;
                attributeDescriptions[i].format = VK_FORMAT_R32G32B32_SFLOAT;
                attributeDescriptions[i].offset = (2 * sizeof(glm::mat4)) + (sizeof(glm::vec3) * (i - 11));
            }

            attributeDescriptions[14].binding = 1;
            attributeDescriptions[14].location = 14;
            attributeDescriptions[14].format = VK_FORMAT_R32_SFLOAT;
            attributeDescriptions[14].offset = offsetof(InstanceInfo, shininess);

            attributeDescriptions[15].binding = 1;
            attributeDescriptions[15].location = 15;
            attributeDescriptions[15].format = VK_FORMAT_R32_UINT;
            attributeDescriptions[15].offset = offsetof(InstanceInfo, lit);

            attributeDescriptions[16].binding = 1;
            attributeDescriptions[16].location = 16;
            attributeDescriptions[16].format = VK_FORMAT_R32_UINT;
            attributeDescriptions[16].offset = offsetof(InstanceInfo, textured);

            attributeDescriptions[17].binding = 1;
            attributeDescriptions[17].location = 17;
            attributeDescriptions[17].format = VK_FORMAT_R32_UINT;
            attributeDescriptions[17].offset = offsetof(InstanceInfo, textureIndex);

            return attributeDescriptions;
        }
    };

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