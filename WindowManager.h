#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <string>
#include <set>

#include <glm/glm.hpp>

#include <qwidget.h>
#include <QVBoxLayout>
#include <QVulkanInstance>

class Scene;
class VulkanWindow;
class VulkanInterface;

class WindowManager {
public:
	const char* DEFAULT_TITLE = "GLFW Window";

	WindowManager();

	WindowManager(size_t width, size_t height, std::string title);

	void SetVulkanInterface(std::shared_ptr<VulkanInterface> vulkanInterface) { m_vulkanInterface = vulkanInterface; }
	void SetScene(std::shared_ptr<Scene> scene) { m_scene = scene; }
	void BeginRendering();

	void InitializeWindow(QVulkanInstance* vulkanInstance);

	QWidget* GetWrappingWidget() { return m_wrappingWidget; }

	size_t GetWidth() { return m_width; }
	size_t GetHeight() { return m_height; }

	glm::vec2 GetMouseDelta() { return m_mouseDelta; };
	glm::vec2 GetScrollDelta() { return m_scrollDelta; };

	std::shared_ptr<VulkanWindow> GetVulkanWindow() { return m_vulkanWindow; }

	void NewFrame();

	bool KeyPressed(int keyCode);
	bool KeyPressedThisFrame(int keyCode);

	void SetFrameBufferResized(bool resized) { m_framebufferResized = true; };

private:
	std::shared_ptr<VulkanWindow> m_vulkanWindow;
	std::shared_ptr<VulkanInterface> m_vulkanInterface;
	std::shared_ptr<Scene> m_scene;

	QWidget* m_wrappingWidget;

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
};