#pragma once

#ifndef VULKAN_COMMON_FUNCTIONS
#define VULKAN_COMMON_FUNCTIONS

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES

#include "vk_mem_alloc.h"

#include "glm/glm.hpp"

#include <optional>
#include <vector>
#include <array>

namespace VulkanCommonFunctions {
    using ObjectHandle = size_t;
    static const VulkanCommonFunctions::ObjectHandle INVALID_OBJECT_HANDLE = 0;
    static const size_t MAX_OBJECTS = 10000;
    
    struct alignas(16) GlobalInfo {
        glm::mat4 view;
        glm::mat4 proj;
        glm::vec4 cameraPosition;
        alignas(4) uint32_t lightCount;
    };

    struct alignas(16) LightInfo {
        glm::vec4 lightPosition;
        glm::vec4 lightColor;

        glm::vec4 lightAmbient;
        glm::vec4 lightDiffuse;
        glm::vec4 lightSpecular;

        alignas(4) float maxLightDistance;
    };

    struct alignas(16) UIGlobalInfo {
        alignas(4) uint32_t screenWidth;
        alignas(4) uint32_t screenHeight;
    };

    struct alignas(16) InstanceInfo {
        glm::mat4 modelMatrix;
        glm::mat4 modelMatrixInverse;

        alignas(16) glm::vec3 scale;

        alignas(16) glm::vec3 ambient;
        alignas(16) glm::vec3 diffuse;
        alignas(16) glm::vec3 specular;
        alignas(4) float opacity;
        alignas(4) float shininess;
        alignas(4) uint32_t lit;

        alignas(4) uint32_t textured;
        alignas(4) uint32_t textureIndex;

        alignas(4) uint32_t isBillboarded;
    };

    struct alignas(16) Vertex {
        alignas(16) glm::vec3 pos;
        alignas(16) glm::vec3 normal;
        alignas(16) glm::vec2 texCoord;

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

        static std::array<VkVertexInputAttributeDescription, 21> GetAttributeDescriptions() {
            std::array<VkVertexInputAttributeDescription, 21> attributeDescriptions{};

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

            attributeDescriptions[11].binding = 1;
            attributeDescriptions[11].location = 11;
            attributeDescriptions[11].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[11].offset = offsetof(InstanceInfo, scale);

            for (uint32_t i = 12; i < 15; i++)
            {
                attributeDescriptions[i].binding = 1;
                attributeDescriptions[i].location = i;
                attributeDescriptions[i].format = VK_FORMAT_R32G32B32_SFLOAT;
                attributeDescriptions[i].offset = (2 * sizeof(glm::mat4)) + (sizeof(glm::vec3) * (i - 11));
            }

            attributeDescriptions[15].binding = 1;
            attributeDescriptions[15].location = 15;
            attributeDescriptions[15].format = VK_FORMAT_R32_SFLOAT;
            attributeDescriptions[15].offset = offsetof(InstanceInfo, opacity);

            attributeDescriptions[16].binding = 1;
            attributeDescriptions[16].location = 16;
            attributeDescriptions[16].format = VK_FORMAT_R32_SFLOAT;
            attributeDescriptions[16].offset = offsetof(InstanceInfo, shininess);

            attributeDescriptions[17].binding = 1;
            attributeDescriptions[17].location = 17;
            attributeDescriptions[17].format = VK_FORMAT_R32_UINT;
            attributeDescriptions[17].offset = offsetof(InstanceInfo, lit);

            attributeDescriptions[18].binding = 1;
            attributeDescriptions[18].location = 18;
            attributeDescriptions[18].format = VK_FORMAT_R32_UINT;
            attributeDescriptions[18].offset = offsetof(InstanceInfo, textured);

            attributeDescriptions[19].binding = 1;
            attributeDescriptions[19].location = 19;
            attributeDescriptions[19].format = VK_FORMAT_R32_UINT;
            attributeDescriptions[19].offset = offsetof(InstanceInfo, textureIndex);

            attributeDescriptions[20].binding = 1;
            attributeDescriptions[20].location = 20;
            attributeDescriptions[20].format = VK_FORMAT_R32_UINT;
            attributeDescriptions[20].offset = offsetof(InstanceInfo, isBillboarded);

            return attributeDescriptions;
        }
    };

    struct alignas(16) UIInstanceInfo {
        alignas(16) glm::vec3 objectPosition;
        alignas(16) glm::vec3 scale;
        alignas(16) glm::vec3 color;
        alignas(8) glm::vec2 textureOffset;
        alignas(8) glm::vec2 characterTextureSize;
        alignas(8) glm::vec2 characterOffset;
        alignas(4) float opacity;
        alignas(4) uint32_t textured;
        alignas(4) uint32_t textureIndex;
        alignas(4) uint32_t isTextCharacter;
    };

    struct alignas(16) UIVertex {
        alignas(16) glm::vec3 position;
        alignas(16) glm::vec2 texCoord;

        static std::array<VkVertexInputBindingDescription, 2> GetBindingDescriptions() {
            std::array<VkVertexInputBindingDescription, 2> result;

            result[0].binding = 0;
            result[0].stride = sizeof(UIVertex);
            result[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            result[1].binding = 1;
            result[1].stride = sizeof(UIInstanceInfo);
            result[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

            return result;
        }

        static std::array<VkVertexInputAttributeDescription, 12> GetAttributeDescriptions() {
            std::array<VkVertexInputAttributeDescription, 12> attributeDescriptions{};

            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(UIVertex, position);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(UIVertex, texCoord);

            attributeDescriptions[2].binding = 1;
            attributeDescriptions[2].location = 2;
            attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[2].offset = offsetof(UIInstanceInfo, objectPosition);

            attributeDescriptions[3].binding = 1;
            attributeDescriptions[3].location = 3;
            attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[3].offset = offsetof(UIInstanceInfo, scale);

            attributeDescriptions[4].binding = 1;
            attributeDescriptions[4].location = 4;
            attributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[4].offset = offsetof(UIInstanceInfo, color);

            attributeDescriptions[5].binding = 1;
            attributeDescriptions[5].location = 5;
            attributeDescriptions[5].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[5].offset = offsetof(UIInstanceInfo, textureOffset);

            attributeDescriptions[6].binding = 1;
            attributeDescriptions[6].location = 6;
            attributeDescriptions[6].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[6].offset = offsetof(UIInstanceInfo, characterTextureSize);

            attributeDescriptions[7].binding = 1;
            attributeDescriptions[7].location = 7;
            attributeDescriptions[7].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[7].offset = offsetof(UIInstanceInfo, characterOffset);

            attributeDescriptions[8].binding = 1;
            attributeDescriptions[8].location = 8;
            attributeDescriptions[8].format = VK_FORMAT_R32_SFLOAT;
            attributeDescriptions[8].offset = offsetof(UIInstanceInfo, opacity);

            attributeDescriptions[9].binding = 1;
            attributeDescriptions[9].location = 9;
            attributeDescriptions[9].format = VK_FORMAT_R32_UINT;
            attributeDescriptions[9].offset = offsetof(UIInstanceInfo, textured);

            attributeDescriptions[10].binding = 1;
            attributeDescriptions[10].location = 10;
            attributeDescriptions[10].format = VK_FORMAT_R32_UINT;
            attributeDescriptions[10].offset = offsetof(UIInstanceInfo, textureIndex);

            attributeDescriptions[11].binding = 1;
            attributeDescriptions[11].location = 11;
            attributeDescriptions[11].format = VK_FORMAT_R32_UINT;
            attributeDescriptions[11].offset = offsetof(UIInstanceInfo, isTextCharacter);

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