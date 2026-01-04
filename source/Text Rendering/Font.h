#pragma once

#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <vector>
#include "glm.hpp"

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

	float GetCharacterSpacingMultiplier() { return m_characterSpacingMultiplier; }
	void SetCharacterSpacingMultiplier(float characterSpacingMultiplier) { m_characterSpacingMultiplier = characterSpacingMultiplier; }

	float GetMaximumWidth() { return m_maxCharacterWidth; }
	float GetBaseHeight() { return m_baseHeight; }
	float GetLineHeight() { return m_lineHeight; }

private:
	void LoadFontData();
	void SplitBySpace(const std::string& str, std::vector<std::string>& outTokens);

	std::string m_fontName = "";

	std::string m_fontAtlasFilePath = "";
	std::string m_fontDescriptionFilePath = "";

	int m_fontAtlasTextureWidth = 0;
	int m_fontAtlasTextureHeight = 0;

	float m_characterSpacingMultiplier = 1.0f;
	float m_maxCharacterWidth = 0.0f;
	float m_baseHeight = 0.0f;
	float m_lineHeight = 0.0f;

	std::map<char, GlyphInfo> m_glyphMap;
};