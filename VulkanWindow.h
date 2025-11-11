#pragma once

#include "VulkanWindowRenderer.h"

#include <QVulkanWindow>
#include "Scene.h"

class VulkanInterface;

class VulkanWindow : public QVulkanWindow {

	//Q_OBJECT

public:
	VulkanWindow(std::shared_ptr<VulkanInterface> vulkanInterface, std::shared_ptr<Scene> scene);

	QVulkanWindowRenderer* createRenderer() override;

	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;

	void RegisterMouseMoveCallback(std::function<void(QMouseEvent*)> callback) { m_mouseMoveCallbacks.push_back(callback); };

private:
	std::shared_ptr<VulkanInterface> m_vulkanInterface;
	std::shared_ptr<Scene> m_scene;
	std::shared_ptr<VulkanWindowRenderer> m_vulkanWindowRenderer = nullptr;

	std::vector<std::function<void(QMouseEvent*)>> m_mouseMoveCallbacks;
};