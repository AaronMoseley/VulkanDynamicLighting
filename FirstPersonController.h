#pragma once

#include "ObjectComponent.h"
#include "Transform.h"
#include "RenderObject.h"
#include "Camera.h"

class FirstPersonController : public ObjectComponent {
public:
    FirstPersonController() {};

    void Start() override;
    void Update(float deltaTime) override;

private:
    // euler Angles
	// yaw is y rotation, pitch is x rotation
    // camera options

    float m_movementSpeed = 4.0f;
    float m_mouseSensitivity = 100.0f;

    bool m_rightClickToLook = true;
};