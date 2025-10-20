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

private:
	RenderObject* m_owner;
	std::shared_ptr<WindowManager> m_windowManager = nullptr;
};