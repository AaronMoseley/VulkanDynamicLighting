#pragma once

#include "RenderObject.h"
#include "Camera.h"
#include "WindowManager.h"
#include "FirstPersonController.h"
#include "Scene.h"
#include "VulkanWindow.h"
#include "DemoBehavior.h"

#include <QApplication>
#include <QScreen>
#include <QVulkanInstance>
#include <QVBoxLayout>

#include <qwidget.h>

class VoltEngine : public QWidget
{
public:
	VoltEngine(QWidget* parent, QVulkanInstance* vulkanInstance, int screenWidth, int screenHeight);

	void BeginRendering();
	void RegisterUpdateCallback(std::function<void(float)> callback);

	WindowManager* GetWindowManager() { return m_windowManager; }
	std::shared_ptr<Scene> GetCurrentScene() { return m_sceneManager; }
	std::shared_ptr<VulkanInterface> GetVulkanInterface() { return m_vulkanInterface; }

private:
	WindowManager* m_windowManager;
	std::shared_ptr<Scene> m_sceneManager;
	std::shared_ptr<VulkanInterface> m_vulkanInterface;

	//QVBoxLayout* m_mainLayout;
};