#include "Font.h"
#include "stb_image.h"

Font::Font(std::string fontAtlasFilePath, std::string fontDescriptionFilePath)
	: m_fontAtlasFilePath(fontAtlasFilePath), m_fontDescriptionFilePath(fontDescriptionFilePath)
{
	LoadFontData();
}

void Font::LoadFontData()
{
	//early exit if either file path doesn't exist
	std::ifstream descFile(m_fontDescriptionFilePath);
	if (!descFile.good())
	{
		std::cerr << "Error: Font description file not found: " << m_fontDescriptionFilePath << std::endl;
		return;
	}

	//use stb to read the image height and width, use to normalize locations and width/height
	int channels;
	if (!stbi_info(m_fontAtlasFilePath.c_str(), &m_fontAtlasTextureWidth, &m_fontAtlasTextureHeight, &channels))
	{
		std::cerr << "Error: Font atlas file not found: " << m_fontAtlasFilePath << std::endl;
		return;
	}

	//read the description file as a vector of lines
	std::string line;

	float maxWidth = 0;
	float maxHeight = 0;

	float minXOffset = std::numeric_limits<float>().max();
	float minYOffset = std::numeric_limits<float>().max();

	//foreach line
	while (std::getline(descFile, line))
	{
		std::vector<std::string> lineParts;
		SplitBySpace(line, lineParts);

		if (lineParts[0] == "info")
		{
			//if starts with info, parse font name and anything else, continue
			for (size_t i = 1; i < lineParts.size(); i++)
			{
				size_t equalPos = lineParts[i].find('=');
				std::string variableName = lineParts[i].substr(0, equalPos);
				std::string value = lineParts[i].substr(equalPos + 1);
				if (variableName == "face")
				{
					m_fontName = value.substr(1, value.size() - 2);
				}
			}
		}
		else if (lineParts[0] == "common")
		{
			for (size_t i = 1; i < lineParts.size(); i++)
			{
				size_t equalPos = lineParts[i].find('=');
				std::string variableName = lineParts[i].substr(0, equalPos);
				std::string value = lineParts[i].substr(equalPos + 1);
				if (variableName == "base")
				{
					m_baseHeight = std::stof(value);
				}
				else if (variableName == "lineHeight")
				{
					m_lineHeight = std::stof(value);
				}
			}
		}
		else if (lineParts[0] == "char")
		{
			GlyphInfo newGlyph;

			//if starts with char, parse char id, x, y, width, height
			for (size_t i = 1; i < lineParts.size(); i++)
			{
				size_t equalPos = lineParts[i].find('=');
				std::string variableName = lineParts[i].substr(0, equalPos);
				std::string value = lineParts[i].substr(equalPos + 1);
				if (variableName == "id")
				{
					char glyphChar = static_cast<char>(std::stoi(lineParts[i].substr(equalPos + 1)));
					newGlyph.character = glyphChar;
				}
				else if (variableName == "x")
				{
					int x = std::stoi(value);
					newGlyph.locationX = static_cast<float>(x) / static_cast<float>(m_fontAtlasTextureWidth);
				}
				else if (variableName == "y")
				{
					int y = std::stoi(value);
					newGlyph.locationY = static_cast<float>(y) / static_cast<float>(m_fontAtlasTextureHeight);
				}
				else if (variableName == "width")
				{
					int width = std::stoi(value);
					m_maxCharacterWidth = std::max(m_maxCharacterWidth, static_cast<float>(width));
					newGlyph.width = static_cast<float>(width) / static_cast<float>(m_fontAtlasTextureWidth);
					maxWidth = std::max(newGlyph.width, maxWidth);
				}
				else if (variableName == "height")
				{
					int height = std::stoi(value);
					newGlyph.height = static_cast<float>(height) / static_cast<float>(m_fontAtlasTextureHeight);
					maxHeight = std::max(newGlyph.height, maxHeight);
				}
				else if (variableName == "xoffset")
				{
					float xOffset = std::stof(value);
					newGlyph.xOffset = xOffset;
					minXOffset = std::min(xOffset, minXOffset);
				}
				else if (variableName == "yoffset")
				{
					float yOffset = std::stof(value);
					newGlyph.yOffset = yOffset;
					minYOffset = std::min(yOffset, minYOffset);
				}
				else if (variableName == "xadvance")
				{
					float xAdvance = std::stof(value);
					newGlyph.xAdvance = xAdvance;
				}
			}

			//add to glyph map
			newGlyph.locationX += newGlyph.width / 2.0f;
			newGlyph.locationY += newGlyph.height / 2.0f;
			m_glyphMap[newGlyph.character] = newGlyph;
		}
	}

	for (auto it = m_glyphMap.begin(); it != m_glyphMap.end(); it++)
	{
		it->second.scaleMultiplierX = it->second.width / maxWidth;
		it->second.scaleMultiplierY = it->second.height / maxHeight;
	}
}

void Font::SplitBySpace(const std::string& str, std::vector<std::string>& outTokens)
{
	size_t start = 0;
	size_t end = str.find(' ');

	bool containedInQuotes = false;
	std::string currentSubstring = "";

	while (end != std::string::npos)
	{
		std::string newSubstring = str.substr(start, end - start);

		start = end + 1;
		end = str.find(' ', start);

		if (!containedInQuotes)
		{
			if (newSubstring.find("\"") == std::string::npos)
			{
				outTokens.push_back(newSubstring);
			}
			else {
				containedInQuotes = true;
				currentSubstring = newSubstring;
			}
			continue;
		} else
		{
			if (newSubstring.find("\"") == std::string::npos)
			{
				currentSubstring += " " + newSubstring;
			}
			else {
				containedInQuotes = false;
				currentSubstring += " " + newSubstring;
				outTokens.push_back(currentSubstring);
			}
			continue;
		}
	}
	outTokens.push_back(str.substr(start, end));
}

Font::GlyphInfo Font::GetCharacterInfo(char character)
{
	if (!m_glyphMap.contains(character))
	{
		GlyphInfo defaultInfo = {};
		return defaultInfo;
	}

	return m_glyphMap[character];
}