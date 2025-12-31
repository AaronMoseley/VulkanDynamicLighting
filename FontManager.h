#pragma once

#include "Font.h"

class FontManager {
public:
	FontManager() {};

	std::shared_ptr<Font> AddFont(std::string atlasFilePath, std::string descriptionFilePath);

	std::shared_ptr<Font> GetFontByName(const std::string& fontName);

private:
	std::map<std::string, std::shared_ptr<Font>> m_fonts;
};