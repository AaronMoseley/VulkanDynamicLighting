#pragma once

#include "source/Vulkan Interface/GraphicsImage.h"

class TextureImage : public GraphicsImage {
public:
	TextureImage(GraphicsImageCreateInfo imageCreateInfo) : GraphicsImage(imageCreateInfo) {};

	void CreateTextureSampler(float maxAnisotropy);
	VkSampler GetTextureSampler() { return m_textureSampler; }

	void DestroyTextureImage();

private:
	VkSampler m_textureSampler = VK_NULL_HANDLE;
};