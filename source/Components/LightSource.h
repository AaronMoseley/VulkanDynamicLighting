#pragma once

#include "source/Objects/ObjectComponent.h"
#include "source/Vulkan Interface/VulkanCommonFunctions.h"
#include "source/Components/Transform.h"

#include "glm/glm.hpp"

class LightSource : public ObjectComponent {
public:
	LightSource() {};

	glm::vec3 GetColor() { return m_color; }
	void SetColor(glm::vec3 color) { m_color = glm::vec4(color, 1.0f); }

	glm::vec3 GetSpecularColor() { return m_specularColor; }
	void SetSpecularColor(glm::vec3 color) { m_specularColor = glm::vec4(color, 1.0f); }

	float GetMaxDistance() { return m_maxDistance; }
	void SetMaxDistance(float distance) { m_maxDistance = distance; }

	VulkanCommonFunctions::LightInfo GetLightInfo()
	{
		VulkanCommonFunctions::LightInfo lightInfo;
		lightInfo.lightColor = m_color;
		lightInfo.lightAmbient = m_color * 0.2f;
		lightInfo.lightDiffuse = m_color * 0.5f;
		lightInfo.lightSpecular = m_specularColor;
		lightInfo.lightPosition = glm::vec4(GetOwner()->GetComponent<Transform>()->GetPosition(), 1.0);
		lightInfo.maxLightDistance = m_maxDistance;
		return lightInfo;
	}

private:
	glm::vec4 m_color = glm::vec4(1.0f);
	glm::vec4 m_specularColor = glm::vec4(1.0f);
	float m_maxDistance = 40.0f;
};