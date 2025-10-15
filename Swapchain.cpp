#include "Swapchain.h"

SwapChain::SwapChain(SwapChain::SwapChainCreateInfo initializationInfo)
{
	m_device = initializationInfo.device;
	m_physicalDevice = initializationInfo.physicalDevice;
	m_surface = initializationInfo.surface;
    m_windowManager = initializationInfo.windowManager;
	m_allocator = initializationInfo.allocator;
	m_commandPool = initializationInfo.commandPool;
	m_graphicsQueue = initializationInfo.graphicsQueue;

    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_physicalDevice, m_surface);

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
    ChooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = m_swapChainExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VulkanCommonFunctions::QueueFamilyIndices indices = VulkanCommonFunctions::FindQueueFamilies(m_physicalDevice, m_surface);
    uint32_t queueFamilyIndices[] = { indices.m_graphicsFamily.value(), indices.m_presentFamily.value() };

    if (indices.m_graphicsFamily != indices.m_presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapChain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    std::vector<VkImage> rawSwapChainImages;

    vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr);
    m_swapChainImages.resize(imageCount);
    rawSwapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, rawSwapChainImages.data());

    GraphicsImage::GraphicsImageCreateInfo swapChainImageCreateInfo{};
    swapChainImageCreateInfo.allocator = m_allocator;
    swapChainImageCreateInfo.device = m_device;
    swapChainImageCreateInfo.format = surfaceFormat.format;
    swapChainImageCreateInfo.imageSize = { m_swapChainExtent.width, m_swapChainExtent.height };
    swapChainImageCreateInfo.commandPool = m_commandPool;
    swapChainImageCreateInfo.graphicsQueue = m_graphicsQueue;

    for (size_t i = 0; i < m_swapChainImages.size(); i++)
    {
        m_swapChainImages[i] = new GraphicsImage(swapChainImageCreateInfo, rawSwapChainImages[i]);
        m_swapChainImages[i]->CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

SwapChain::SwapChainSupportDetails SwapChain::QuerySwapChainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

void SwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
		m_swapChainExtent = capabilities.currentExtent;
    }
    else {
        int width, height;
        glfwGetFramebufferSize(m_windowManager->GetWindow(), &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        m_swapChainExtent = actualExtent;
    }
}

void SwapChain::DestroySwapChain()
{
    for (auto framebuffer : m_swapChainFramebuffers) {
        vkDestroyFramebuffer(m_device, framebuffer, nullptr);
    }

    for (auto swapChainImage : m_swapChainImages)
    {
        swapChainImage->DestroyImageView();
    }

    vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
}

bool SwapChain::IsSwapChainAdequate(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    SwapChainSupportDetails swapChainSupport = SwapChain::QuerySwapChainSupport(physicalDevice, surface);
	return !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
}

VkSurfaceFormatKHR SwapChain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR SwapChain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

void SwapChain::CreateFrameBuffers(GraphicsImage* depthImage, VkRenderPass renderPass)
{
    m_swapChainFramebuffers.resize(m_swapChainImages.size());

    for (size_t i = 0; i < m_swapChainImages.size(); i++) {
        std::array<VkImageView, 2> attachments = {
            m_swapChainImages[i]->GetImageView(),
            depthImage->GetImageView()
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = m_swapChainExtent.width;
        framebufferInfo.height = m_swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_swapChainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}