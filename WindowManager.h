#pragma once

#include <string>
#include <set>

#include <glm/glm.hpp>

#include <qwidget.h>
#include <QVBoxLayout>
#include <QVulkanInstance>
#include <QMetaMethod>

class Scene;
class VulkanWindow;
class VulkanInterface;

class WindowManager : public QObject {

	Q_OBJECT

public:
	const char* DEFAULT_TITLE = "GLFW Window";

	WindowManager();

	WindowManager(QWidget* parentProgram, size_t width, size_t height, std::string title);

	void SetVulkanInterface(std::shared_ptr<VulkanInterface> vulkanInterface) { m_vulkanInterface = vulkanInterface; }
	void SetScene(std::shared_ptr<Scene> scene) { m_scene = scene; }
	void BeginRendering();

	void InitializeWindow(QVulkanInstance* vulkanInstance);

	QWidget* GetWrappingWidget() { return m_wrappingWidget; }

	size_t GetWidth() { return m_width; }
	size_t GetHeight() { return m_height; }

	glm::vec2 GetMouseDelta() { return m_mouseDelta; };
	glm::vec2 GetScrollDelta() { return m_scrollDelta; };

	VulkanWindow* GetVulkanWindow() { return m_vulkanWindow; }

	void NewFrame();

	bool KeyPressed(Qt::Key keyCode);
	bool KeyPressedThisFrame(Qt::Key keyCode);

	bool MouseButtonPressed(Qt::MouseButton mouseButton);
	bool MouseButtonPressedThisFrame(Qt::MouseButton mouseButton);

	void SetFrameBufferResized(bool resized) { m_framebufferResized = true; };

	void Shutdown();

	void SetLockCursor(bool lockCursor);
	bool IsCursorLocked();

	void SetIsTrackingMouse(bool isTracking);
	bool IsTrackingMouse();

public slots:
	void AddKeyDown(Qt::Key pressedKey);
	void AddKeyUp(Qt::Key releasedKey);

	void AddMouseButtonDown(Qt::MouseButton pressedButton);
	void AddMouseButtonUp(Qt::MouseButton releasedButton);

	void CursorMoved(float xpos, float ypos);

private:
	VulkanWindow* m_vulkanWindow = nullptr;
	std::shared_ptr<VulkanInterface> m_vulkanInterface = nullptr;
	std::shared_ptr<Scene> m_scene = nullptr;

	QWidget* m_parentProgram;
	QWidget* m_wrappingWidget;

	//members
	bool m_framebufferResized = false;
	size_t m_width = 0;
	size_t m_height = 0;

	glm::vec2 m_mouseDelta = glm::vec2(0.0f, 0.0f);
	bool m_firstMouseMovement = true;

	glm::vec2 m_scrollDelta = glm::vec2(0.0f, 0.0f);

	std::set<Qt::Key> m_pressedKeys;
	std::set<Qt::Key> m_keysPressedThisFrame;

	std::set<Qt::MouseButton> m_pressedMouseButtons;
	std::set<Qt::MouseButton> m_pressedMouseButtonsThisFrame;

	std::string m_title = "";
};