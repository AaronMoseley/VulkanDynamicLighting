#pragma once

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "VulkanCommonFunctions.h"
#include "ObjectComponent.h"
#include "Factory.h"
#include "Transform.h"
#include "MeshRenderer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>
#include <vector>
#include <string>
#include <vulkan/vulkan_core.h>
#include <map>
#include <memory>

class RenderObject {
public:
	RenderObject();
	RenderObject(glm::vec3 position);
	RenderObject(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);

	template <typename T>
	T* AddComponent()
	{
		T* newComponent = new T();
		m_components.push_back(newComponent);

		return newComponent;
	}

	template <typename T>
	T* GetComponent()
	{
		for (size_t i = 0; i < m_components.size(); i++)
		{
			T* component = dynamic_cast<T*>(m_components[i]);

			if (component != nullptr)
			{
				return component;
			}
		}

		return nullptr;
	}

    VulkanCommonFunctions::InstanceInfo GetInstanceInfo();

private:
	std::vector<ObjectComponent*> m_components;
};