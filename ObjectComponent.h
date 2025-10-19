#pragma once

#include <string>

class ObjectComponent {
public:
	ObjectComponent() {};
	virtual ~ObjectComponent() = default;

	virtual void Start() {};
	virtual void Update(float deltaTime) {};

	void* GetOwner() { return m_owner; }
	void SetOwner(void* owner) { m_owner = owner; }

private:
	void* m_owner = nullptr;
};