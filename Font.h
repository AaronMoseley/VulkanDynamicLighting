#pragma once

#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <vector>

//generate fonts at: https://fonts.varg.dev/

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
	};

	Font(std::string fontAtlasFilePath, std::string fontDescriptionFilePath);

	std::string GetFontName() const { return m_fontName; }
	GlyphInfo GetCharacterInfo(char character);

	std::string GetAtlasFilePath() { return m_fontAtlasFilePath; }

private:
	void LoadFontData();
	void SplitBySpace(const std::string& str, std::vector<std::string>& outTokens);

	std::string m_fontName = "";

	std::string m_fontAtlasFilePath = "";
	std::string m_fontDescriptionFilePath = "";

	int m_fontAtlasTextureWidth = 0;
	int m_fontAtlasTextureHeight = 0;

	std::map<char, GlyphInfo> m_glyphMap;
};