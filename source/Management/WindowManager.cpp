#include "WindowManager.h"
#include "source/Management/Scene.h"
#include "source/Vulkan Interface/VulkanWindow.h"
#include "source/Vulkan Interface/VulkanInterface.h"

#include <iostream>

WindowManager::WindowManager(QWidget* parentProgram, size_t width, size_t height, std::string title) :
	QVBoxLayout(parentProgram), m_parentProgram(parentProgram), m_width(width), m_height(height), m_title(title)
{
    
}

void WindowManager::BeginRendering()
{
    m_vulkanWindow->requestUpdate();
}

void WindowManager::InitializeWindow(QVulkanInstance* vulkanInstance)
{
	m_buttonLayout = new QHBoxLayout();
	addLayout(m_buttonLayout);

    m_vulkanWindow = new VulkanWindow(m_vulkanInterface, m_scene);
    m_vulkanWindow->setVulkanInstance(vulkanInstance);

    QObject::connect(m_vulkanWindow, &VulkanWindow::KeyDown, this, &WindowManager::AddKeyDown);
    QObject::connect(m_vulkanWindow, &VulkanWindow::KeyUp, this, &WindowManager::AddKeyUp);

    QObject::connect(m_vulkanWindow, &VulkanWindow::MouseButtonDown, this, &WindowManager::AddMouseButtonDown);
    QObject::connect(m_vulkanWindow, &VulkanWindow::MouseButtonUp, this, &WindowManager::AddMouseButtonUp);

    QObject::connect(m_vulkanWindow, &VulkanWindow::MouseMoved, this, &WindowManager::CursorMoved);

    m_wrappingWidget = QWidget::createWindowContainer(m_vulkanWindow);
    m_wrappingWidget->resize(m_width, m_height);

	addWidget(m_wrappingWidget);
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

void WindowManager::SetLockCursor(bool lockCursor)
{
    m_vulkanWindow->SetLockCursor(lockCursor);
}

bool WindowManager::IsCursorLocked()
{
    return m_vulkanWindow->IsCursorLocked();
}

void WindowManager::SetIsTrackingMouse(bool isTrackingMouse)
{
    m_vulkanWindow->SetTrackingMouse(isTrackingMouse);
}

bool WindowManager::IsTrackingMouse()
{
    return m_vulkanWindow->IsTrackingMouse();
}

void WindowManager::AddButton(std::string title, const std::function<void()>& callback)
{
    if (m_buttons.contains(title))
    {
		qDebug() << "Button with title " << title << " already exists!";
    }

    QPushButton* newButton = new QPushButton(QString::fromStdString(title));
    connect(newButton, &QPushButton::clicked, [callback]() {
        callback();
    });

	m_buttonLayout->addWidget(newButton);
    m_buttons[title] = newButton;
}

void WindowManager::RemoveButton(std::string title)
{
    if (!m_buttons.contains(title))
    {
        qDebug() << "Button with title " << title << " does not exist!";
        return;
    }
    QPushButton* buttonToRemove = m_buttons[title];
    m_buttonLayout->removeWidget(buttonToRemove);
    delete buttonToRemove;
    m_buttons.erase(title);
}