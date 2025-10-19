#include "FirstPersonController.h"

void FirstPersonController::Update(float deltaTime)
{
	std::shared_ptr<Transform> transform = static_cast<RenderObject*>(GetOwner())->GetComponent<Transform>();

    glm::vec3 offset = glm::vec3(0.0f);

    float velocity = m_movementSpeed * deltaTime;
    if (GetWindowManager()->KeyPressed(GLFW_KEY_W))
    {
        offset += transform->Forward() * velocity;
    }

    if (GetWindowManager()->KeyPressed(GLFW_KEY_S))
    {
        offset -= transform->Forward() * velocity;
    }

    if (GetWindowManager()->KeyPressed(GLFW_KEY_A))
    {
        offset -= transform->Right() * velocity;
    }

    if (GetWindowManager()->KeyPressed(GLFW_KEY_D))
    {
        offset += transform->Right() * velocity;
    }
	transform->Move(offset);

	glm::vec3 rotationDelta = glm::vec3(0.0f);
	glm::vec2 mouseDelta = GetWindowManager()->GetMouseDelta();

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

	std::shared_ptr<Camera> camera = static_cast<RenderObject*>(GetOwner())->GetComponent<Camera>();
	float newFOV = camera->GetFOV();

    newFOV -= GetWindowManager()->GetScrollDelta().y;
    newFOV = glm::clamp(newFOV, 1.0f, 45.0f);

	camera->SetFOV(newFOV);
}