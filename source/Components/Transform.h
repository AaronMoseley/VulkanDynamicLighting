#pragma once

#include "source/Objects/ObjectComponent.h"
#include <glm.hpp>
#include <gtc/quaternion.hpp>

class Transform : public ObjectComponent {
public:
	Transform() {};

	glm::vec3 GetPosition() { return m_position; }
	glm::vec3 GetRotation() { return m_rotation; }
	glm::vec3 GetScale() { return m_scale; }

	glm::vec3 GetWorldPosition() 
	{
		std::shared_ptr<Transform> parent = GetParent();
		if (parent == nullptr)
		{
			return GetPosition();
		}

		glm::quat parentRotation = glm::quat(glm::radians(parent->GetWorldRotation()));

		glm::vec3 rotatedLocalPos = parentRotation * GetPosition();

		return parent->GetWorldPosition() + rotatedLocalPos;
	}

	glm::vec3 GetWorldRotation()
	{
		std::shared_ptr<Transform> parent = GetParent();
		if (parent == nullptr)
		{
			return GetRotation();
		}

		return parent->GetWorldRotation() + GetRotation();
	}

	glm::vec3 GetWorldScale()
	{
		std::shared_ptr<Transform> parent = GetParent();
		if (parent == nullptr)
		{
			return GetScale();
		}

		return parent->GetWorldScale() * GetScale();
	}

	void SetParent(std::shared_ptr<Transform> parentTransform) { m_parentTransform = parentTransform; }
	std::shared_ptr<Transform> GetParent() { return m_parentTransform; }

	void SetRotation(glm::vec3 rotation) { m_rotation = glm::vec4(rotation, 1.0f); }
	void SetPosition(glm::vec3 position) { m_position = glm::vec4(position, 1.0f); }
	void SetScale(glm::vec3 scale) { m_scale = glm::vec4(scale, 1.0f); }

	void Rotate(glm::vec3 amountToRotate) { m_rotation += glm::vec4(amountToRotate, 0.0f); }
	void Move(glm::vec3 amountToMove) { m_position += glm::vec4(amountToMove, 0.0f); }
	void Scale(glm::vec3 amountToScale) { m_scale += glm::vec4(amountToScale, 0.0f); }

	glm::vec3 Forward()
	{
		glm::vec3 forward;
		forward.x = cos(glm::radians(m_rotation.y)) * cos(glm::radians(m_rotation.x));
		forward.y = sin(glm::radians(m_rotation.x));
		forward.z = sin(glm::radians(m_rotation.y)) * cos(glm::radians(m_rotation.x));
		return glm::normalize(forward);
	}

	glm::vec3 Right()
	{
		return glm::normalize(glm::cross(Forward(), glm::vec3(0.0f, 1.0f, 0.0f)));
	}

	glm::vec3 Up()
	{
		return glm::normalize(glm::cross(Right(), Forward()));
	}

private:
	using ObjectComponent::SetEnabled;

	std::shared_ptr<Transform> m_parentTransform = nullptr;

	glm::vec4 m_position = glm::vec4(0.0f);
	glm::vec4 m_rotation = glm::vec4(0.0f);
	glm::vec4 m_scale = glm::vec4(1.0f);
};