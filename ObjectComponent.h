#pragma once

#include "WindowManager.h"

#include <string>
#include <memory>

class RenderObject;

class ObjectComponent {
public:
	ObjectComponent() {};
	virtual ~ObjectComponent() = default;

	virtual void Start() {};
	virtual void Update(float deltaTime) {};

	RenderObject* GetOwner() { return m_owner; }
	void SetOwner(RenderObject* owner) { m_owner = owner; }

	std::shared_ptr<WindowManager> GetWindowManager() { return m_windowManager; }
	void SetWindowManager(std::shared_ptr<WindowManager> windowManager) { m_windowManager = windowManager; }

	void SetEnabled(bool enabled) { m_enabled = enabled; }
	bool IsEnabled() { return m_enabled; }

	bool HasStarted() { return m_started; }
	void SetStarted(bool started) { m_started = started; }

private:
	RenderObject* m_owner = nullptr;
	std::shared_ptr<WindowManager> m_windowManager = nullptr;
	alignas(16) bool m_enabled = true;
	alignas(16) bool m_started = false;
};