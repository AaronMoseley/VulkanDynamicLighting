#pragma once

#include "WindowManager.h"

#include <string>
#include <memory>

class ObjectComponent {
public:
	ObjectComponent() {};
	virtual ~ObjectComponent() = default;

	virtual void Start() {};
	virtual void Update(float deltaTime) {};

	void* GetOwner() { return m_owner; }
	void SetOwner(void* owner) { m_owner = owner; }

	std::shared_ptr<WindowManager> GetWindowManager() { return m_windowManager; }
	void SetWindowManager(std::shared_ptr<WindowManager> windowManager) { m_windowManager = windowManager; }

private:
	void* m_owner = nullptr;
	std::shared_ptr<WindowManager> m_windowManager = nullptr;
};