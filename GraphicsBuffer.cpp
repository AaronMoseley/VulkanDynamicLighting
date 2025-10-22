#include "GraphicsBuffer.h"

GraphicsBuffer::GraphicsBuffer(BufferCreateInfo createInfo)
{
	m_allocator = createInfo.allocator;

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = createInfo.size;
    bufferInfo.usage = createInfo.usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

	m_device = createInfo.device;
	m_commandPool = createInfo.commandPool;
	m_graphicsQueue = createInfo.graphicsQueue;
	m_properties = createInfo.properties;

	m_maxSize = createInfo.size;

    //m_mappedData = malloc(m_maxSize);
	m_mappedData = static_cast<void*>(std::make_unique<char[]>(m_maxSize).get());

    vmaCreateBuffer(m_allocator, &bufferInfo, &allocInfo, &m_buffer, &m_allocation, nullptr);

    vmaMapMemory(m_allocator, m_allocation, &m_mappedData);
}

void GraphicsBuffer::CopyBuffer(std::shared_ptr<GraphicsBuffer> destintationBuffer, VkDeviceSize copySize)
{
    VkCommandBuffer commandBuffer = VulkanCommonFunctions::BeginSingleTimeCommands(m_device, m_commandPool);

    VkBufferCopy copyRegion{};
    copyRegion.size = copySize;
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    vkCmdCopyBuffer(commandBuffer, m_buffer, destintationBuffer->GetVkBuffer(), 1, &copyRegion);

    VulkanCommonFunctions::EndSingleTimeCommands(commandBuffer, m_device, m_commandPool, m_graphicsQueue);
}

void GraphicsBuffer::LoadData(void* data, size_t memorySize)
{
    if (memorySize > m_maxSize)
    {
		throw std::runtime_error("Data size exceeds buffer size!");
    }

    memcpy(m_mappedData, data, memorySize);

	bool hostCoherent = m_properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    if (!hostCoherent)
    {
        vmaFlushAllocation(m_allocator, m_allocation, 0, m_maxSize);
    }
}

void GraphicsBuffer::DestroyBuffer()
{
    vmaUnmapMemory(m_allocator, m_allocation);
    vmaDestroyBuffer(m_allocator, m_buffer, m_allocation);
}