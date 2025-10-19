#pragma once

#include "ObjectComponent.h"
#include <glm/glm.hpp>

#include "Factory.h"

class Transform : public ObjectComponent {
public:
	Transform() {};

	glm::vec3 GetPosition() { return m_position; }
	glm::vec3 GetRotation() { return m_rotation; }
	glm::vec3 GetScale() { return m_scale; }

	void SetRotation(glm::vec3 rotation) { m_rotation = rotation; }
	void SetPosition(glm::vec3 position) { m_position = position; }
	void SetScale(glm::vec3 scale) { m_scale = scale; }

	void Rotate(glm::vec3 amountToRotate) { m_rotation += amountToRotate; }
	void Move(glm::vec3 amountToMove) { m_position += amountToMove; }
	void Scale(glm::vec3 amountToScale) { m_scale += amountToScale; }

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
	inline static const std::string m_name = "Transform";

	glm::vec3 m_position = glm::vec3(0.0f);
	glm::vec3 m_rotation = glm::vec3(0.0f);
	glm::vec3 m_scale = glm::vec3(0.0f);
};