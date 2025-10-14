#pragma once

#include "vk_mem_alloc.h"

#include <cstdlib>
#include <stdexcept>

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

	void CopyBuffer(GraphicsBuffer* destintationBuffer, VkDeviceSize copySize);
	void LoadData(void* data, size_t memorySize);
	void DestroyBuffer();

private:
	VkBuffer m_buffer = VK_NULL_HANDLE;
	VmaAllocation m_allocation = nullptr;

	VkMemoryPropertyFlags m_properties;

	VmaAllocator m_allocator;

	VkDevice m_device;
	VkCommandPool m_commandPool;
	VkQueue m_graphicsQueue;
	void* m_mappedData;

	size_t m_maxSize;
};