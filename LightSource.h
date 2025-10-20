#pragma once

#include "ObjectComponent.h"
#include "VulkanCommonFunctions.h"
#include "Transform.h"

#include "glm/glm.hpp"

class LightSource : public ObjectComponent {
public:
	LightSource() {};

	glm::vec3 GetColor() { return m_color; }
	void SetColor(glm::vec3 color) { m_color = color; }

	glm::vec3 GetSpecularColor() { return m_specularColor; }
	void SetSpecularColor(glm::vec3 color) { m_specularColor = color; }

	float GetMaxDistance() { return m_maxDistance; }
	void SetMaxDistance(float distance) { m_maxDistance = distance; }

	VulkanCommonFunctions::LightInfo GetLightInfo()
	{
		VulkanCommonFunctions::LightInfo lightInfo;
		lightInfo.lightColor = glm::vec4(m_color, 1.0);
		lightInfo.lightAmbient = glm::vec4(m_color * 0.2f, 1.0);
		lightInfo.lightDiffuse = glm::vec4(m_color * 0.5f, 1.0);
		lightInfo.lightSpecular = glm::vec4(m_specularColor, 1.0);
		lightInfo.lightPosition = glm::vec4(GetOwner()->GetComponent<Transform>()->GetPosition(), 1.0);
		lightInfo.maxLightDistance = m_maxDistance;
		return lightInfo;
	}

private:
	glm::vec3 m_color = glm::vec3(1.0f);
	glm::vec3 m_specularColor = glm::vec3(1.0f);
	float m_maxDistance = 20.0f;
};