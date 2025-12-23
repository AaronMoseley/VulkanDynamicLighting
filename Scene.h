#pragma once

#include "VulkanInterface.h"
#include "VulkanCommonFunctions.h"
#include "UIImage.h"

#include <memory>
#include <vector>
#include <functional>
#include <chrono>

class RenderObject;

class alignas(16) Scene {
public:
	Scene(WindowManager* windowManager, std::shared_ptr<VulkanInterface> vulkanInterface);

	void Update();

	void Cleanup();

	VulkanCommonFunctions::ObjectHandle AddObject(std::shared_ptr <RenderObject> newObject);
	bool RemoveObject(VulkanCommonFunctions::ObjectHandle objectToRemove);

	VulkanCommonFunctions::ObjectHandle AddUIObject(std::shared_ptr <RenderObject> newObject);
	bool RemoveUIObject(VulkanCommonFunctions::ObjectHandle objectToRemove);

	void FinalizeMesh(std::shared_ptr<RenderObject> updatedObject);
	void GenerateInstanceBuffer(std::shared_ptr<RenderObject> updatedObject);

	void FinalizeUIMesh(std::shared_ptr<RenderObject> updatedObject);

	void UpdateTexture(std::string newTexturePath);

	std::map<VulkanCommonFunctions::ObjectHandle, std::shared_ptr<RenderObject>> GetObjects() { return m_objects; };
	std::map<VulkanCommonFunctions::ObjectHandle, std::shared_ptr<RenderObject>> GetUIObjects() { return m_uiObjects; };
	std::map<std::string, std::set<VulkanCommonFunctions::ObjectHandle>> GetMeshNameToObjectMap() { return m_meshNameToObjectMap; }

	VulkanCommonFunctions::ObjectHandle GetObjectByTag(std::string tag);
	std::shared_ptr<RenderObject> GetRenderObject(VulkanCommonFunctions::ObjectHandle handle);
	std::shared_ptr<RenderObject> GetUIRenderObject(VulkanCommonFunctions::ObjectHandle handle);

	size_t GetObjectCount() { return m_objects.size(); };

	void RegisterUpdateCallback(std::function<void(float)> callback) {
		m_updateCallbacks.push_back(callback);
	}

private:
	void UpdateMeshData(std::shared_ptr<RenderObject> currentObject);
	void UpdateUIData(std::shared_ptr<RenderObject> currentObject);

	alignas(16) std::map<VulkanCommonFunctions::ObjectHandle, std::shared_ptr<RenderObject>> m_objects = {};
	alignas(16) std::map<std::string, std::set<VulkanCommonFunctions::ObjectHandle>> m_meshNameToObjectMap;

	alignas(16) std::map<VulkanCommonFunctions::ObjectHandle, std::shared_ptr<RenderObject>> m_uiObjects = {};

	alignas(16) WindowManager* m_windowManager;
	alignas(16) std::shared_ptr<VulkanInterface> m_vulkanInterface;

	alignas(16) VulkanCommonFunctions::ObjectHandle m_currentObjectHandle = 0;
	alignas(16) VulkanCommonFunctions::ObjectHandle m_currentUIObjectHandle = 0;

	alignas(16) std::vector<std::function<void(float)>> m_updateCallbacks;

	alignas(16) std::vector<std::shared_ptr<GraphicsBuffer>> m_buffersToDestroy;

	alignas(16) double m_deltaTime = 0.0f;	// Time between current frame and last frame
	alignas(16) double m_lastFrame = -1.0f; // Time of last frame

	bool temp = false;
};