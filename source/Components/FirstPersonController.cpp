#include "FirstPersonController.h"

void FirstPersonController::Start()
{
    //hide cursor and capture it within the window
    if (m_rightClickToLook)
    {
        GetWindowManager()->SetIsTrackingMouse(false);
        GetWindowManager()->SetLockCursor(false);
    }
}

void FirstPersonController::Update(float deltaTime)
{
	std::shared_ptr<Transform> transform = GetOwner()->GetComponent<Transform>();

    glm::vec3 offset = glm::vec3(0.0f);

    float velocity = m_movementSpeed * deltaTime;

    if (GetWindowManager()->KeyPressed(Qt::Key::Key_Shift))
    {
        velocity *= 4.0f;
    }

    if (GetWindowManager()->KeyPressed(Qt::Key::Key_W))
    {
        offset += transform->Forward() * velocity;
    }

    if (GetWindowManager()->KeyPressed(Qt::Key::Key_S))
    {
        offset -= transform->Forward() * velocity;
    }

    if (GetWindowManager()->KeyPressed(Qt::Key::Key_A))
    {
        offset -= transform->Right() * velocity;
    }

    if (GetWindowManager()->KeyPressed(Qt::Key::Key_D))
    {
        offset += transform->Right() * velocity;
    }
	transform->Move(offset);

    if (GetWindowManager()->MouseButtonPressed(Qt::MouseButton::RightButton) && m_rightClickToLook && !GetWindowManager()->IsCursorLocked())
    {
		GetWindowManager()->SetLockCursor(true);
		GetWindowManager()->SetIsTrackingMouse(true);
    }
    else if(m_rightClickToLook && !GetWindowManager()->MouseButtonPressed(Qt::MouseButton::RightButton) && GetWindowManager()->IsCursorLocked()) {
        GetWindowManager()->SetLockCursor(false);
        GetWindowManager()->SetIsTrackingMouse(false);
    }

	glm::vec3 rotationDelta = glm::vec3(0.0f);
	glm::vec2 mouseDelta = GetWindowManager()->GetMouseDelta();

    if (mouseDelta.x > 0.01f)
    {
        int temp = 0;
    }

	rotationDelta.x = mouseDelta.y * m_mouseSensitivity;
    rotationDelta.y = mouseDelta.x * m_mouseSensitivity;
	transform->Rotate(rotationDelta);

    if (transform->GetRotation().x > 89.0f)
    {
		transform->SetRotation(glm::vec3(89.0f, transform->GetRotation().y, transform->GetRotation().z));
    }

    if (transform->GetRotation().x < -89.0f)
    {
		transform->SetRotation(glm::vec3(-89.0f, transform->GetRotation().y, transform->GetRotation().z));
    }

	std::shared_ptr<Camera> camera = GetOwner()->GetComponent<Camera>();
	float newFOV = camera->GetFOV();

    newFOV -= GetWindowManager()->GetScrollDelta().y;
    newFOV = glm::clamp(newFOV, 1.0f, 45.0f);

	camera->SetFOV(newFOV);
}