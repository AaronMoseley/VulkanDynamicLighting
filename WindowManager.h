#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <string>
#include <set>

#include <glm/glm.hpp>

class WindowManager {
public:
	const size_t DEFAULT_WIDTH = 800;
	const size_t DEFAULT_HEIGHT = 600;
	const char* DEFAULT_TITLE = "GLFW Window";

	WindowManager();

	WindowManager(size_t width, size_t height, std::string title);

	void InitializeWindow();

	glm::vec2 GetMouseDelta() { return m_mouseDelta; };
	glm::vec2 GetScrollDelta() { return m_scrollDelta; };

	void NewFrame();

	bool KeyPressed(int keyCode);
	bool KeyPressedThisFrame(int keyCode);

	GLFWwindow* GetWindow() { return m_window; };

	void SetFrameBufferResized(bool resized) { m_framebufferResized = true; };

private:
	//callbacks
	static void StaticCursorMovedCallback(GLFWwindow* window, double xpos, double ypos)
	{
		WindowManager* windowManager = static_cast<WindowManager*>(glfwGetWindowUserPointer(window));
		if (windowManager != nullptr)
		{
			windowManager->CursorMoved(xpos, ypos);
		}
	};
	void CursorMoved(double xpos, double ypos);

	static void FrameBufferResizedCallback(GLFWwindow* window, int width, int height)
	{
		WindowManager* windowManager = reinterpret_cast<WindowManager*>(glfwGetWindowUserPointer(window));
		windowManager->SetFrameBufferResized(true);
	};

	static void StaticScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
		WindowManager* windowManager = static_cast<WindowManager*>(glfwGetWindowUserPointer(window));
		if (windowManager != nullptr)
		{
			windowManager->ScrollCallback(xoffset, yoffset);
		}
	};
	void ScrollCallback(double xoffset, double yoffset);

	static void StaticKeyPressedCallback(GLFWwindow* window, int key, int scanCode, int action, int mods) {
		WindowManager* windowManager = static_cast<WindowManager*>(glfwGetWindowUserPointer(window));
		if (windowManager != nullptr)
		{
			windowManager->KeyPressedCallback(key, scanCode, action, mods);
		}
	}
	void KeyPressedCallback(int key, int scanCode, int action, int mods);

	//members
	bool m_framebufferResized = false;
	size_t m_width = 0;
	size_t m_height = 0;

	glm::vec2 m_lastMousePos = glm::vec2(0.0f, 0.0f);
	glm::vec2 m_mouseDelta = glm::vec2(0.0f, 0.0f);
	bool m_firstMouseMovement = true;

	glm::vec2 m_scrollDelta = glm::vec2(0.0f, 0.0f);

	std::set<int> m_pressedKeys;
	std::set<int> m_keysPressedThisFrame;

	std::string m_title = "";

	GLFWwindow* m_window;
};