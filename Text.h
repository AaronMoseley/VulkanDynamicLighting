#pragma once

#include "UIMeshRenderer.h"
#include "VulkanCommonFunctions.h"
#include "Font.h"

class Text : public UIMeshRenderer {
public:
	Text();

	void SetTextString(const std::string& newText) { m_textString = newText; m_textDataDirty = true; }
	std::string GetTextString() const { return m_textString; }

	void SetFontName(const std::string& fontName) { m_fontName = fontName; m_textDataDirty = true; }
	std::string GetFontName() const { return m_fontName; }

	void SetFontSize(float fontSize) { m_fontSize = fontSize; m_textDataDirty = true; }
	float GetFontSize() { return m_fontSize; }

	const std::vector<VulkanCommonFunctions::UIVertex>& GetVertices() override { return m_squareVertices; };
	const std::vector<uint16_t>& GetIndices() override { return m_squareIndices; };

	void UpdateInstanceBuffer(std::pair<size_t, size_t> screenSize, std::shared_ptr<Font> currentFont, size_t textureIndex, GraphicsBuffer::BufferCreateInfo bufferCreateInfo);
	std::shared_ptr<GraphicsBuffer> GetInstanceBuffer() { return m_instanceBuffer; }

	void SetReferenceResolution(glm::vec2 referenceResolution) { m_referenceResolution = referenceResolution; m_textDataDirty = true; }
	glm::vec2 GetReferenceResolution() { return m_referenceResolution; }
	glm::vec2 GetPixelToScreen() { return 1.0f / m_referenceResolution; }

private:
	using UIMeshRenderer::SetVertices;
	using UIMeshRenderer::SetIndices;

	void GetCharacterInstanceInfo(std::pair<size_t, size_t> screenSize, std::shared_ptr<Font> currentFont, std::vector<VulkanCommonFunctions::UIInstanceInfo>& outCharacterInfo);

	std::vector<VulkanCommonFunctions::UIVertex> m_squareVertices = {
		//positions              //texture coords
		{ {-1.0f,  1.0f, 0.0f},  {-0.5f, -0.5f} }, //top left
		{ { 1.0f,  1.0f, 0.0f},  {0.5f, -0.5f} }, //top right
		{ { 1.0f, -1.0f, 0.0f},  {0.5f, 0.5f} }, //bottom right
		{ {-1.0f, -1.0f, 0.0f},  {-0.5f, 0.5f} }  //bottom left
	};

	std::vector<uint16_t> m_squareIndices = { 0, 1, 2, 2, 3, 0 };

	glm::vec2 m_referenceResolution = { 1920.0f, 1080.0f };

	std::shared_ptr<GraphicsBuffer> m_instanceBuffer = nullptr;

	std::string m_textString = "";
	std::string m_fontName = "";

	glm::vec4 m_color = glm::vec4(1.0f);

	float m_fontSize = 40.0f;

	float m_additionalCharacterSpacing = 0.0f;
	float m_additionalLineSpacing = 0.0f;

	bool m_textDataDirty = false;
};