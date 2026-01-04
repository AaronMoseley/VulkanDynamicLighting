#pragma once

#include "source/Objects/RenderObject.h"
#include "source/Components/Camera.h"
#include "source/Management/WindowManager.h"
#include "source/Components/FirstPersonController.h"
#include "source/Management/Scene.h"
#include "source/Vulkan Interface/VulkanWindow.h"
#include "source/Components/DemoBehavior.h"

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