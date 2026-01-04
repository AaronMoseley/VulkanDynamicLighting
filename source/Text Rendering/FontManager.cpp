#include "FontManager.h"

std::shared_ptr<Font> FontManager::AddFont(std::string atlasFilePath, std::string descriptionFilePath)
{
	std::shared_ptr<Font> newFont = std::make_shared<Font>(atlasFilePath, descriptionFilePath);
	m_fonts[newFont->GetFontName()] = newFont;

	return newFont;
}

std::shared_ptr<Font> FontManager::GetFontByName(const std::string& fontName)
{
	auto it = m_fonts.find(fontName);
	if (it != m_fonts.end())
	{
		return it->second;
	}
	return nullptr;
}