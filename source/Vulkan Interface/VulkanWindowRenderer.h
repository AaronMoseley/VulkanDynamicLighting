#pragma once

#include <QVulkanWindowRenderer>
#include "source/Management/Scene.h"

class VulkanInterface;

class VulkanWindowRenderer : public QVulkanWindowRenderer {
public:
	VulkanWindowRenderer(std::shared_ptr<VulkanInterface> vulkanInterface, std::shared_ptr<Scene> scene);

	void preInitResources() override;
	void initResources() override;
	void initSwapChainResources() override;
	void releaseSwapChainResources() override;
	void releaseResources() override;
	void startNextFrame() override;

	void Shutdown();

private:
	std::shared_ptr<VulkanInterface> m_vulkanInterface;
	std::shared_ptr<Scene> m_scene;

	bool m_isShuttingDown = false;
};