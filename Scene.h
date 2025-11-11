#pragma once

#include "VulkanInterface.h"
#include "VulkanCommonFunctions.h"

#include <memory>
#include <vector>
#include <functional>

class RenderObject;

class Scene {
public:
	Scene(std::shared_ptr<WindowManager> windowManager, std::shared_ptr<VulkanInterface> vulkanInterface);

	void Update();

	void Cleanup();

	VulkanCommonFunctions::ObjectHandle AddObject(std::shared_ptr <RenderObject> newObject);
	bool RemoveObject(VulkanCommonFunctions::ObjectHandle objectToRemove);

	void FinalizeMesh(std::shared_ptr<RenderObject> updatedObject);
	void GenerateInstanceBuffer(std::shared_ptr<RenderObject> updatedObject);

	void UpdateTexture(std::string newTexturePath);

	std::map<VulkanCommonFunctions::ObjectHandle, std::shared_ptr<RenderObject>> GetObjects() { return m_objects; };
	std::map<std::string, std::set<VulkanCommonFunctions::ObjectHandle>> GetMeshNameToObjectMap() { return m_meshNameToObjectMap; }

	std::shared_ptr<RenderObject> GetRenderObject(VulkanCommonFunctions::ObjectHandle handle);

	size_t GetObjectCount() { return m_objects.size(); };

	void RegisterUpdateCallback(std::function<void(float)> callback) {
		m_updateCallbacks.push_back(callback);
	}

private:
	std::map<VulkanCommonFunctions::ObjectHandle, std::shared_ptr<RenderObject>> m_objects;
	std::map<std::string, std::set<VulkanCommonFunctions::ObjectHandle>> m_meshNameToObjectMap;
	std::shared_ptr<WindowManager> m_windowManager;
	std::shared_ptr<VulkanInterface> m_vulkanInterface;

	VulkanCommonFunctions::ObjectHandle m_currentObjectHandle = 0;

	std::vector<std::function<void(float)>> m_updateCallbacks;

	std::vector<std::shared_ptr<GraphicsBuffer>> m_buffersToDestroy;

	float m_deltaTime = 0.0f;	// Time between current frame and last frame
	float m_lastFrame = 0.0f; // Time of last frame
};