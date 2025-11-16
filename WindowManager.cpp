#include "WindowManager.h"
#include <iostream>
#include "Scene.h"
#include "VulkanWindow.h"
#include "VulkanInterface.h"

WindowManager::WindowManager() :
    m_width(0), m_height(0), m_title("Application")
{
    
}

WindowManager::WindowManager(size_t width, size_t height, std::string title) :
	m_width(width), m_height(height), m_title(title)
{
    
}

void WindowManager::BeginRendering()
{
    m_vulkanWindow->requestUpdate();
}

void WindowManager::InitializeWindow(QVulkanInstance* vulkanInstance)
{
    m_vulkanWindow = std::make_shared<VulkanWindow>(m_vulkanInterface, m_scene);
    m_vulkanWindow->setVulkanInstance(vulkanInstance);

    m_wrappingWidget = QWidget::createWindowContainer(m_vulkanWindow.get());
    m_wrappingWidget->resize(m_width, m_height);
    //m_wrappingWidget->show();
}

void WindowManager::NewFrame()
{
    m_keysPressedThisFrame.clear();

	m_mouseDelta = glm::vec2(0.0f, 0.0f);
	m_scrollDelta = glm::vec2(0.0f, 0.0f);
}

bool WindowManager::KeyPressed(int keyCode)
{
    return m_pressedKeys.contains(keyCode);
}

bool WindowManager::KeyPressedThisFrame(int keyCode)
{
    return m_keysPressedThisFrame.contains(keyCode);
}

/*void WindowManager::CursorMoved(double xpos, double ypos)
{
    if (m_firstMouseMovement)
    {
        m_lastMousePos = glm::vec2(xpos, ypos);
		m_firstMouseMovement = false;
        return;
    }

	glm::vec2 currentPos = glm::vec2(xpos, ypos);
	m_mouseDelta = currentPos - m_lastMousePos;
	m_mouseDelta.y = -m_mouseDelta.y;

	m_lastMousePos = currentPos;
}*/