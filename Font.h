#pragma once

#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <vector>
#include "glm/glm.hpp"

//generate fonts at: https://fonts.varg.dev/
//font file/rendering documentation at: https://www.angelcode.com/products/bmfont/

class Font {
public:
	struct GlyphInfo {
		char character;

		float width;
		float height;

		float locationX;
		float locationY;

		float scaleMultiplierX;
		float scaleMultiplierY;

		float xOffset;
		float yOffset;

		float xAdvance;
	};

	Font(std::string fontAtlasFilePath, std::string fontDescriptionFilePath);

	std::string GetFontName() const { return m_fontName; }
	GlyphInfo GetCharacterInfo(char character);

	std::string GetAtlasFilePath() { return m_fontAtlasFilePath; }

	glm::vec2 GetPixelToScreen() { return m_pixelToScreen; }

	float GetCharacterSpacingMultiplier() { return m_characterSpacingMultiplier; }
	void SetCharacterSpacingMultiplier(float characterSpacingMultiplier) { m_characterSpacingMultiplier = characterSpacingMultiplier; }

private:
	void LoadFontData();
	void SplitBySpace(const std::string& str, std::vector<std::string>& outTokens);

	std::string m_fontName = "";

	std::string m_fontAtlasFilePath = "";
	std::string m_fontDescriptionFilePath = "";

	int m_fontAtlasTextureWidth = 0;
	int m_fontAtlasTextureHeight = 0;

	glm::vec2 m_referenceResolution = { 1920.0f, 1080.0f };
	glm::vec2 m_pixelToScreen = { 1.0f, 1.0f };
	float m_characterSpacingMultiplier = 1.0f;

	std::map<char, GlyphInfo> m_glyphMap;
};