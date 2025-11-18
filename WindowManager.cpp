#include "WindowManager.h"
#include <iostream>
#include "Scene.h"
#include "VulkanWindow.h"
#include "VulkanInterface.h"

WindowManager::WindowManager() :
    m_parentProgram(nullptr), m_width(0), m_height(0), m_title("Application")
{
    
}

WindowManager::WindowManager(QWidget* parentProgram, size_t width, size_t height, std::string title) :
	m_parentProgram(parentProgram), m_width(width), m_height(height), m_title(title)
{
    
}

void WindowManager::BeginRendering()
{
    m_vulkanWindow->requestUpdate();
}

void WindowManager::InitializeWindow(QVulkanInstance* vulkanInstance)
{
    m_vulkanWindow = new VulkanWindow(m_vulkanInterface, m_scene);
    m_vulkanWindow->setVulkanInstance(vulkanInstance);

    QObject::connect(m_vulkanWindow, &VulkanWindow::KeyDown, this, &WindowManager::AddKeyDown);
    QObject::connect(m_vulkanWindow, &VulkanWindow::KeyUp, this, &WindowManager::AddKeyUp);

    QObject::connect(m_vulkanWindow, &VulkanWindow::MouseButtonDown, this, &WindowManager::AddMouseButtonDown);
    QObject::connect(m_vulkanWindow, &VulkanWindow::MouseButtonUp, this, &WindowManager::AddMouseButtonUp);

    QObject::connect(m_vulkanWindow, &VulkanWindow::MouseMoved, this, &WindowManager::CursorMoved);

    m_wrappingWidget = QWidget::createWindowContainer(m_vulkanWindow);
    m_wrappingWidget->resize(m_width, m_height);
}

void WindowManager::NewFrame()
{
    m_keysPressedThisFrame.clear();

	m_mouseDelta = glm::vec2(0.0f, 0.0f);
	m_scrollDelta = glm::vec2(0.0f, 0.0f);
}

bool WindowManager::KeyPressed(Qt::Key keyCode)
{
    return m_pressedKeys.contains(keyCode);
}

bool WindowManager::KeyPressedThisFrame(Qt::Key keyCode)
{
    return m_keysPressedThisFrame.contains(keyCode);
}

bool WindowManager::MouseButtonPressed(Qt::MouseButton mouseButton)
{
    return m_pressedMouseButtons.contains(mouseButton);
}

bool WindowManager::MouseButtonPressedThisFrame(Qt::MouseButton mouseButton)
{
    return m_pressedMouseButtonsThisFrame.contains(mouseButton);
}

void WindowManager::AddKeyDown(Qt::Key pressedKey)
{
    m_keysPressedThisFrame.insert(pressedKey);
    m_pressedKeys.insert(pressedKey);
}

void WindowManager::AddKeyUp(Qt::Key releasedKey)
{
    m_keysPressedThisFrame.erase(releasedKey);
    m_pressedKeys.erase(releasedKey);
}

void WindowManager::AddMouseButtonDown(Qt::MouseButton pressedButton)
{
    m_pressedMouseButtons.insert(pressedButton);
    m_pressedMouseButtonsThisFrame.insert(pressedButton);
}

void WindowManager::AddMouseButtonUp(Qt::MouseButton releasedButton)
{
    m_pressedMouseButtons.erase(releasedButton);
    m_pressedMouseButtonsThisFrame.erase(releasedButton);
}

void WindowManager::CursorMoved(float xpos, float ypos)
{
    m_mouseDelta = {xpos, ypos};
	m_mouseDelta.y = -m_mouseDelta.y;
}

void WindowManager::Shutdown()
{
    m_vulkanWindow->Shutdown();
    QMetaObject::invokeMethod(m_parentProgram, "close", Qt::QueuedConnection);
}