#pragma once

#include "WindowManager.h"

#include <string>
#include <memory>

class RenderObject;
class Scene;

class ObjectComponent {
public:
	ObjectComponent() {};
	virtual ~ObjectComponent() = default;

	virtual void Start() {};
	virtual void Update(float deltaTime) {};

	std::shared_ptr<RenderObject> GetOwner() { return m_owner; }
	void SetOwner(std::shared_ptr<RenderObject> owner) { m_owner = std::shared_ptr<RenderObject>(owner); }
	Scene* GetScene();

	std::shared_ptr<WindowManager> GetWindowManager();
	void SetWindowManager(std::shared_ptr<WindowManager> windowManager) { m_windowManager = windowManager; }

	void SetEnabled(bool enabled) { m_enabled = enabled; }
	bool IsEnabled() { return m_enabled; }

	bool HasStarted() { return m_started; }
	void SetStarted(bool started) { m_started = started; }

private:
	std::shared_ptr<RenderObject> m_owner = nullptr;
	std::shared_ptr<WindowManager> m_windowManager = nullptr;
	bool m_enabled = true;
	bool m_started = false;
};