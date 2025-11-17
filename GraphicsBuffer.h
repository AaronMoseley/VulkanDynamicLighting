#pragma once

#include "vk_mem_alloc.h"

#include <cstdlib>
#include <stdexcept>
#include <memory>

#include "VulkanCommonFunctions.h"

class GraphicsBuffer {
public:
	struct BufferCreateInfo {
		VmaAllocator allocator;
		VkDeviceSize size;
		VkBufferUsageFlags usage;
		VkMemoryPropertyFlags properties;
		VkDevice device;
		VkCommandPool commandPool;
		VkQueue graphicsQueue;
	};

	GraphicsBuffer(BufferCreateInfo createInfo);

	VkBuffer GetVkBuffer() { return m_buffer; }

	void CopyBuffer(std::shared_ptr<GraphicsBuffer> destintationBuffer, VkDeviceSize copySize);
	void LoadData(void* data, size_t memorySize);
	void DestroyBuffer();

private:
	VkBuffer m_buffer = VK_NULL_HANDLE;
	VmaAllocation m_allocation = VK_NULL_HANDLE;

	VkMemoryPropertyFlags m_properties = 0;

	VmaAllocator m_allocator = VK_NULL_HANDLE;

	VkDevice m_device = VK_NULL_HANDLE;
	VkCommandPool m_commandPool = VK_NULL_HANDLE;
	VkQueue m_graphicsQueue = VK_NULL_HANDLE;

	size_t m_maxSize = 0;

	void* m_mappedData = nullptr;
};