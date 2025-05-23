#pragma once

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>
#include <vector>
#include <string>
#include <vulkan/vulkan_core.h>

struct GlobalInfo {
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec3 cameraPosition;

    glm::vec3 lightPosition;
    glm::vec3 lightColor;

    glm::vec3 lightAmbient;
    glm::vec3 lightDiffuse;
    glm::vec3 lightSpecular;
};

struct InstanceInfo {
    glm::mat4 modelMatrix;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
    uint32_t lit;
};

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;

    static std::array<VkVertexInputBindingDescription, 2> getBindingDescriptions() {
        std::array<VkVertexInputBindingDescription, 2> result;
        
        result[0].binding = 0;
        result[0].stride = sizeof(Vertex);
        result[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        result[1].binding = 1;
        result[1].stride = sizeof(InstanceInfo);
        result[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

        return result;
    }

    static std::array<VkVertexInputAttributeDescription, 11> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 11> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, normal);

        for (uint32_t i = 2; i < 6; i++) {
            attributeDescriptions[i].binding = 1; // instance buffer binding index
            attributeDescriptions[i].location = i; // consecutive locations (e.g., 2,3,4,5)
            attributeDescriptions[i].format = VK_FORMAT_R32G32B32A32_SFLOAT; // vec4 per row
            attributeDescriptions[i].offset = sizeof(glm::vec4) * (i - 2);
        }

        for (uint32_t i = 6; i < 9; i++)
        {
            attributeDescriptions[i].binding = 1;
            attributeDescriptions[i].location = i;
            attributeDescriptions[i].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[i].offset = sizeof(glm::mat4) + (sizeof(glm::vec3) * (i - 6));
        }

        attributeDescriptions[9].binding = 1;
        attributeDescriptions[9].location = 9;
        attributeDescriptions[9].format = VK_FORMAT_R32_SFLOAT;
        attributeDescriptions[9].offset = offsetof(InstanceInfo, shininess);
        
        attributeDescriptions[10].binding = 1;
        attributeDescriptions[10].location = 10;
        attributeDescriptions[10].format = VK_FORMAT_R32_UINT;
        attributeDescriptions[10].offset = offsetof(InstanceInfo, lit);

        return attributeDescriptions;
    }
};

class RenderObject {
public:
	RenderObject();

	RenderObject(glm::vec3 position);

	RenderObject(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);

    RenderObject(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, glm::vec3 color);

	void SetRotation(glm::vec3 rotation);
	void SetPosition(glm::vec3 position);

	void Rotate(glm::vec3 amountToRotate);
	void Move(glm::vec3 amountToMove);

    std::vector<Vertex> getVertices();
    std::vector<uint16_t> getIndices();

	glm::vec3 getPosition();

	glm::vec3 getRotation();

    std::string getName();

    bool getLit();
    void setLit(bool lit);

    bool isIndexed();

    uint32_t getVerticesSize();
    uint32_t getIndicesSize();

    InstanceInfo getInstanceInfo();

protected:
	glm::vec3 m_position;
	glm::vec3 m_rotation;
    glm::vec3 m_scale;
    glm::vec3 m_color;
    std::vector<Vertex> m_vertices;
    std::vector<uint16_t> m_indices;
    std::string m_name = "EmptyObject";
    bool m_useIndices = false;
    bool m_lit = true;
};