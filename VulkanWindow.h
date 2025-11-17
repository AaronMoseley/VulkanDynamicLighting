#pragma once

#include "VulkanWindowRenderer.h"

#include <QVulkanWindow>
#include "Scene.h"

#include <QKeyEvent>
#include <QMouseEvent>

class VulkanInterface;

class VulkanWindow : public QVulkanWindow {

	Q_OBJECT

public:
	VulkanWindow(std::shared_ptr<VulkanInterface> vulkanInterface, std::shared_ptr<Scene> scene);

	QVulkanWindowRenderer* createRenderer() override;

	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;

	// Keyboard events
	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;

	void RegisterMouseMoveCallback(std::function<void(QMouseEvent*)> callback) { m_mouseMoveCallbacks.push_back(callback); };

	void Shutdown();

signals:
	void KeyDown(Qt::Key pressedKey);
	void KeyUp(Qt::Key releasedKey);

	void MouseButtonDown(Qt::MouseButton pressedButton);
	void MouseButtonUp(Qt::MouseButton releasedButton);

	void MouseMoved(float x, float y);

private:
	std::shared_ptr<VulkanInterface> m_vulkanInterface;
	std::shared_ptr<Scene> m_scene;
	VulkanWindowRenderer* m_vulkanWindowRenderer = nullptr;

	std::vector<std::function<void(QMouseEvent*)>> m_mouseMoveCallbacks;

	VkPhysicalDeviceDescriptorIndexingFeatures m_indexingFeatures{};
};