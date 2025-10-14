#include "GraphicsImage.h"

GraphicsImage::GraphicsImage(GraphicsImageCreateInfo imageCreateInfo)
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = imageCreateInfo.imageSize.first;
    imageInfo.extent.height = imageCreateInfo.imageSize.second;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = imageCreateInfo.format;
    imageInfo.tiling = imageCreateInfo.tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = imageCreateInfo.usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocationInfo{};
    allocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocationInfo.requiredFlags = imageCreateInfo.properties;

	m_allocator = imageCreateInfo.allocator;
	m_device = imageCreateInfo.device;
	m_imageSize = imageCreateInfo.imageSize;
	m_commandPool = imageCreateInfo.commandPool;
	m_graphicsQueue = imageCreateInfo.graphicsQueue;
	m_imageFormat = imageCreateInfo.format;

    if (vmaCreateImage(m_allocator, &imageInfo, &allocationInfo, &m_image, &m_imageMemory, nullptr) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create image!");
    }
}

GraphicsImage::GraphicsImage(GraphicsImageCreateInfo imageCreateInfo, VkImage rawImage)
{
    m_allocator = imageCreateInfo.allocator;
    m_device = imageCreateInfo.device;
    m_imageSize = imageCreateInfo.imageSize;
    m_commandPool = imageCreateInfo.commandPool;
    m_graphicsQueue = imageCreateInfo.graphicsQueue;
    m_imageFormat = imageCreateInfo.format;
	m_image = rawImage;
	m_imageMemory = nullptr;
}

void GraphicsImage::CreateImageView(VkImageAspectFlags aspectFlags)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = m_imageFormat;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(m_device, &viewInfo, nullptr, &m_imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }

	m_createdImageView = true;
}

void GraphicsImage::CopyFromBuffer(GraphicsBuffer* buffer)
{
    VkCommandBuffer commandBuffer = VulkanCommonFunctions::BeginSingleTimeCommands(m_device, m_commandPool);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = {
        (uint32_t)m_imageSize.first,
        (uint32_t)m_imageSize.second,
        1
    };

    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer->GetVkBuffer(),
        m_image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    VulkanCommonFunctions::EndSingleTimeCommands(commandBuffer, m_device, m_commandPool, m_graphicsQueue);
}

void GraphicsImage::TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkCommandBuffer commandBuffer = VulkanCommonFunctions::BeginSingleTimeCommands(m_device, m_commandPool);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = 0;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (VulkanCommonFunctions::HasStencilComponent(m_imageFormat)) {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    VulkanCommonFunctions::EndSingleTimeCommands(commandBuffer, m_device, m_commandPool, m_graphicsQueue);
}

void GraphicsImage::DestroyImage()
{
    if (m_createdImageView)
    {
        vkDestroyImageView(m_device, m_imageView, nullptr);
    }

    vmaDestroyImage(m_allocator, m_image, m_imageMemory);
}

void GraphicsImage::DestroyImageView()
{
    if (m_createdImageView)
    {
        vkDestroyImageView(m_device, m_imageView, nullptr);
    }
}