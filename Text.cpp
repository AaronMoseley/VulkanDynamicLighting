#include "Text.h"
#include "RenderObject.h"
#include "Transform.h"

Text::Text()
{
	SetVertices(m_squareVertices);
	SetIndices(m_squareIndices);
}

void Text::UpdateInstanceBuffer(std::pair<size_t, size_t> screenSize, std::shared_ptr<Font> currentFont, size_t textureIndex, GraphicsBuffer::BufferCreateInfo bufferCreateInfo)
{
	if (m_textDataDirty == false)
	{
		return;
	}

	m_textDataDirty = false;

	std::vector<VulkanCommonFunctions::UIInstanceInfo> characterInfos;
	GetCharacterInstanceInfo(screenSize, currentFont, characterInfos);

	for (size_t i = 0; i < characterInfos.size(); i++)
	{
		characterInfos[i].textureIndex = textureIndex;
	}

	bufferCreateInfo.size = sizeof(VulkanCommonFunctions::UIInstanceInfo) * characterInfos.size();

	//will need to account for resizing the buffer when the number of characters changes
	if (m_instanceBuffer == nullptr)
	{
		m_instanceBuffer = std::make_shared<GraphicsBuffer>(bufferCreateInfo);
	}

	m_instanceBuffer->LoadData(characterInfos.data(), bufferCreateInfo.size);
}

void Text::GetCharacterInstanceInfo(std::pair<size_t, size_t> screenSize, std::shared_ptr<Font> currentFont, std::vector<VulkanCommonFunctions::UIInstanceInfo>& outCharacterInfo)
{
	glm::vec3 componentPosition = GetOwner()->GetComponent<Transform>()->GetWorldPosition();
	glm::vec3 scale = GetOwner()->GetComponent<Transform>()->GetWorldScale();

	size_t charactersInCurrentLine = 0;

	glm::vec2 cursorPosition = componentPosition;

	//foreach character
	for (size_t i = 0; i < m_textString.size(); i++)
	{
		char currentCharacter = m_textString.c_str()[i];

		if (currentCharacter == '\n')
		{
			charactersInCurrentLine = 0;

			cursorPosition.y -= (currentFont->GetLineHeight() / currentFont->GetBaseHeight()) * m_fontSize * currentFont->GetPixelToScreen().y;
			cursorPosition.x = componentPosition.x;
			continue;
		}

		Font::GlyphInfo currentGlyphInfo = currentFont->GetCharacterInfo(currentCharacter);

		float heightScale = (currentGlyphInfo.scaleMultiplierY * m_fontSize) * currentFont->GetPixelToScreen().y;
		float widthScale = (currentGlyphInfo.scaleMultiplierX * m_fontSize) * currentFont->GetPixelToScreen().x;

		//create new instance info
		VulkanCommonFunctions::UIInstanceInfo currentCharacterInfo = {};

		//set color and opacity
		currentCharacterInfo.color = glm::vec3(m_color);
		currentCharacterInfo.opacity = m_color.a;

		//set textured to 1, texture index set in VulkanInterface
		currentCharacterInfo.textured = 1;

		//get object position from transform component

		//use object position as the "left" end of the text
		//use character spacing to determine the next character's position
		//on newline, increment the y position
		glm::vec2 currentCharacterPosition = cursorPosition;

		if (charactersInCurrentLine > 0)
		{
			
		}

		charactersInCurrentLine++;

		currentCharacterInfo.objectPosition = glm::vec3(currentCharacterPosition, 0.0f);
		currentCharacterInfo.scale = glm::vec3(widthScale * scale.x, heightScale * scale.y, scale.z);

		currentCharacterInfo.isTextCharacter = 1;
		currentCharacterInfo.characterTextureSize = glm::vec2(currentGlyphInfo.width, currentGlyphInfo.height);
		currentCharacterInfo.textureOffset = glm::vec2(currentGlyphInfo.locationX, currentGlyphInfo.locationY);

		currentCharacterInfo.characterOffset = glm::vec2(
			((currentGlyphInfo.xOffset / currentFont->GetMaximumWidth()) * m_fontSize) * currentFont->GetPixelToScreen().x, 
			((currentGlyphInfo.yOffset / currentFont->GetBaseHeight()) * m_fontSize) * currentFont->GetPixelToScreen().y
		);

		cursorPosition.x += (((currentGlyphInfo.xAdvance / currentFont->GetMaximumWidth()) * m_fontSize) * currentFont->GetPixelToScreen().x);

		outCharacterInfo.push_back(currentCharacterInfo);
	}
}