#pragma once

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "VulkanCommonFunctions.h"
#include "MeshRenderer.h"
#include "ObjectComponent.h"
#include "GraphicsBuffer.h"
#include "Transform.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>
#include <vector>
#include <string>
#include <vulkan/vulkan_core.h>
#include <map>
#include <memory>

class Scene;
class WindowManager;

class RenderObject : public std::enable_shared_from_this<RenderObject> {
public:
	RenderObject(std::shared_ptr<WindowManager> windowManager);

	template <typename T>
	std::shared_ptr<T> AddComponent()
	{
		std::shared_ptr<T> newComponent = std::make_shared<T>();
		m_components.push_back(newComponent);

		newComponent->SetOwner(shared_from_this());

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

	void SetSceneManager(Scene* sceneManager) { m_sceneManager = sceneManager; }
	Scene* GetSceneManager() { return m_sceneManager; }

	bool IsInitialized() { return m_initialized; }

private:
	std::vector<std::shared_ptr<ObjectComponent>> m_components;
	std::shared_ptr<WindowManager> m_windowManager = nullptr;
	std::shared_ptr<GraphicsBuffer> m_instanceBuffer = nullptr;
	
	Scene* m_sceneManager = nullptr;

	bool m_initialized = false;
};