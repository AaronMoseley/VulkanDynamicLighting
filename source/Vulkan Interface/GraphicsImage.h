#pragma once

#include "source/Vulkan Interface/GraphicsBuffer.h"
#include "source/Vulkan Interface/VulkanCommonFunctions.h"

class GraphicsImage {
public:
	struct GraphicsImageCreateInfo {
		std::pair<size_t, size_t> imageSize;
		VkFormat format;
		VkImageTiling tiling;
		VkImageUsageFlags usage;
		VkMemoryPropertyFlags properties;
		VmaAllocator allocator;
		VkDevice device;
		VkCommandPool commandPool;
		VkQueue graphicsQueue;
	};

	GraphicsImage(GraphicsImageCreateInfo imageCreateInfo);
	GraphicsImage(GraphicsImageCreateInfo imageCreateInfo, VkImage rawImage, VkImageView imageView);

	VkImageView GetImageView() { return (m_createdImageView) ? m_imageView : VK_NULL_HANDLE; }
	VkFormat GetImageFormat() { return m_imageFormat; }

	void CreateImageView(VkImageAspectFlags aspectFlags);
	void CopyFromBuffer(GraphicsBuffer* buffer);
	void TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
	VkImage GetVkImage() { return m_image; }
	void DestroyImage();
	void DestroyImageView();

protected:
	VkImage m_image;
	VmaAllocation m_imageMemory;

	VkFormat m_imageFormat;
	VkImageView m_imageView = nullptr;

	VmaAllocator m_allocator;
	VkDevice m_device;
	VkCommandPool m_commandPool;
	VkQueue m_graphicsQueue;

	bool m_createdImageView = false;

	std::pair<size_t, size_t> m_imageSize;
};