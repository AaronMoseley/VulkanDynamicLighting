#pragma once

#include "WindowManager.h"

#include <string>

class ObjectComponent {
public:
	ObjectComponent() {};
	virtual ~ObjectComponent() = default;

	virtual void Start() {};
	virtual void Update(float deltaTime) {};

	void* GetOwner() { return m_owner; }
	void SetOwner(void* owner) { m_owner = owner; }

	WindowManager* GetWindowManager() { return m_windowManager; }
	void SetWindowManager(WindowManager* windowManager) { m_windowManager = windowManager; }

private:
	void* m_owner = nullptr;
	WindowManager* m_windowManager = nullptr;
};