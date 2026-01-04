#pragma once

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "source/Vulkan Interface/VulkanCommonFunctions.h"
#include "source/Components/MeshRenderer.h"
#include "source/Objects/ObjectComponent.h"
#include "source/Vulkan Interface/GraphicsBuffer.h"
#include "source/Components/Transform.h"

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

class alignas(16) RenderObject : public std::enable_shared_from_this<RenderObject> {
public:
	RenderObject();

	template <typename T>
	std::shared_ptr<T> AddComponent()
	{
		std::shared_ptr<T> newComponent = std::make_shared<T>();
		m_components.push_back(newComponent);

		newComponent->SetOwner(shared_from_this());

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

    VulkanCommonFunctions::InstanceInfo GetInstanceInfo(const std::vector<std::string>& textureFilePaths);
	VulkanCommonFunctions::UIInstanceInfo GetUIInstanceInfo(const std::vector<std::string>& textureFilePaths);
	std::shared_ptr<GraphicsBuffer> GetInstanceBuffer(const std::vector<std::string>& textureFilePaths);
	std::shared_ptr<GraphicsBuffer> GetUIInstanceBuffer(const std::vector<std::string>& textureFilePaths);
	void SetInstanceBuffer(std::shared_ptr<GraphicsBuffer> instanceBuffer) { m_instanceBuffer = instanceBuffer; }

	void SetSceneManager(Scene* sceneManager) { m_sceneManager = sceneManager; }
	Scene* GetSceneManager() { return m_sceneManager; }

	void SetWindowManager(WindowManager* windowManager) { m_windowManager = windowManager; }
	WindowManager* GetWindowManager() { return m_windowManager; }

	bool IsInitialized() { return m_initialized; }

	void SetTag(std::string tag) { m_tag = tag; }
	std::string GetTag() { return m_tag; }

private:
	std::vector<std::shared_ptr<ObjectComponent>> m_components;
	WindowManager* m_windowManager = nullptr;
	std::shared_ptr<GraphicsBuffer> m_instanceBuffer = nullptr;
	
	Scene* m_sceneManager = nullptr;

	std::string m_tag = "";

	bool m_initialized = false;
};