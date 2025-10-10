#include "WindowManager.h"
#include <iostream>

WindowManager::WindowManager() :
	m_width(DEFAULT_WIDTH), m_height(DEFAULT_HEIGHT), m_title(DEFAULT_TITLE)
{
    InitializeWindow();
}

WindowManager::WindowManager(size_t width, size_t height, std::string title) :
	m_width(width), m_height(height), m_title(title)
{
    InitializeWindow();
}

void WindowManager::InitializeWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, FrameBufferResizedCallback);

    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetCursorPosCallback(m_window, StaticCursorMovedCallback);

	glfwSetKeyCallback(m_window, StaticKeyPressedCallback);

    glfwSetScrollCallback(m_window, StaticScrollCallback);
}

void WindowManager::KeyPressedCallback(int key, int scanCode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        m_pressedKeys.insert(key);
		m_keysPressedThisFrame.insert(key);
    }
    else if (action == GLFW_RELEASE)
    {
        m_pressedKeys.erase(key);
		m_keysPressedThisFrame.erase(key);
    }
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

void WindowManager::CursorMoved(double xpos, double ypos)
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
}

void WindowManager::ScrollCallback(double xoffset, double yoffset)
{
	m_scrollDelta = glm::vec2(xoffset, yoffset);
}