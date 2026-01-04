#pragma once

#include "source/Objects/ObjectComponent.h"
#include "source/Vulkan Interface/TextureImage.h"
#include "source/Vulkan Interface/VulkanCommonFunctions.h"
#include "source/Components/UIMeshRenderer.h"

class UIImage : public UIMeshRenderer {
public:
	UIImage();
	UIImage(std::string imageFilePath);

	void SetOpacity(float opacity) { m_opacity = opacity; }
	float GetOpacity() { return m_opacity; }

	bool GetTextured() { return m_textured; }
	std::string GetTexturePath() { return m_texturePath; }

	void SetTexture(std::string texturePath) {
		m_texturePath = texturePath;
		m_textured = true;
		m_textureDataDirty = true;
		CalculateMeshInfo();
	};
	void SetTextured(bool textured) { m_textured = textured; }

	void CalculateMeshInfo();

	glm::vec3 GetColor() { return m_color; }
	void SetColor(glm::vec3 color) { m_color = color; }

	void SetTextureDataDirty(bool dirty) { m_textureDataDirty = dirty; }
	bool IsTextureDataDirty() { return m_textureDataDirty; }

	const std::vector<VulkanCommonFunctions::UIVertex>& GetVertices() override { return m_squareVertices; };
	const std::vector<uint16_t>& GetIndices() override { return m_squareIndices; };

private:
	using UIMeshRenderer::SetVertices;
	using UIMeshRenderer::SetIndices;

	alignas(16) std::vector<VulkanCommonFunctions::UIVertex> m_squareVertices = {
		//positions              //texture coords
		{ {-1.0f,  1.0f, 0.0f},  {0.0f, 0.0f} }, //top left
		{ { 1.0f,  1.0f, 0.0f},  {1.0f, 0.0f} }, //top right
		{ { 1.0f, -1.0f, 0.0f},  {1.0f, 1.0f} }, //bottom right
		{ {-1.0f, -1.0f, 0.0f},  {0.0f, 1.0f} }  //bottom left
	};

	alignas(16) std::vector<uint16_t> m_squareIndices = { 0, 1, 2, 2, 3, 0 };

	float m_opacity = 1.0f;

	bool m_textured = false;
	std::string m_texturePath = "";

	bool m_textureDataDirty = false;

	int m_imageWidth = 0;
	int m_imageHeight = 0;

	alignas(16) glm::vec3 m_color = glm::vec3(1.0f);
};