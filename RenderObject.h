#pragma once

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "VulkanCommonFunctions.h"
#include "Factory.h"
#include "MeshRenderer.h"
#include "ObjectComponent.h"
#include "GraphicsBuffer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>
#include <vector>
#include <string>
#include <vulkan/vulkan_core.h>
#include <map>
#include <memory>

class Transform;
class Scene;

class RenderObject {
public:
	RenderObject(std::shared_ptr<WindowManager> windowManager);
	RenderObject(std::shared_ptr<WindowManager> windowManager, glm::vec3 position);
	RenderObject(std::shared_ptr<WindowManager> windowManager, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);

	template <typename T>
	std::shared_ptr<T> AddComponent()
	{
		std::shared_ptr<T> newComponent = std::make_shared<T>();
		m_components.push_back(newComponent);

		newComponent->SetOwner(static_cast<void*>(this));
		newComponent->SetWindowManager(m_windowManager);

		newComponent->Start();

		return newComponent;
	}

	template <typename T>
	std::shared_ptr<T> GetComponent()
	{
		for (size_t i = 0; i < m_components.size(); i++)
		{
			std::shared_ptr<T> component = std::dynamic_pointer_cast<T>(m_components[i]);

			if (component != nullptr)
			{
				return component;
			}
		}

		return nullptr;
	}

	std::vector<std::shared_ptr<ObjectComponent>> GetAllComponents() { return m_components; }

    VulkanCommonFunctions::InstanceInfo GetInstanceInfo();
	std::shared_ptr<GraphicsBuffer> GetInstanceBuffer();
	void SetInstanceBuffer(std::shared_ptr<GraphicsBuffer> instanceBuffer) { m_instanceBuffer = instanceBuffer; }

private:
	std::vector<std::shared_ptr<ObjectComponent>> m_components;
	std::shared_ptr<WindowManager> m_windowManager = nullptr;
	std::shared_ptr<Scene> m_sceneManager = nullptr;
	std::shared_ptr<GraphicsBuffer> m_instanceBuffer = nullptr;
};