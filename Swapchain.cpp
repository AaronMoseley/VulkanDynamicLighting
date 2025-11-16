#include "Swapchain.h"
#include "WindowManager.h"
#include "VulkanWindow.h"

SwapChain::SwapChain(SwapChain::SwapChainCreateInfo initializationInfo)
{
	m_device = initializationInfo.device;
	m_physicalDevice = initializationInfo.physicalDevice;
	m_surface = initializationInfo.surface;
    m_windowManager = initializationInfo.windowManager;
	m_allocator = initializationInfo.allocator;
	m_commandPool = initializationInfo.commandPool;
	m_graphicsQueue = initializationInfo.graphicsQueue;

	m_vulkanWindow = m_windowManager->GetVulkanWindow();

    int imageCount = m_vulkanWindow->swapChainImageCount();
    m_swapChainImages.resize(imageCount);

    GraphicsImage::GraphicsImageCreateInfo swapChainImageCreateInfo{};
    swapChainImageCreateInfo.allocator = m_allocator;
    swapChainImageCreateInfo.device = m_device;
	swapChainImageCreateInfo.imageSize = { GetSwapChainExtent().width, GetSwapChainExtent().height };
    swapChainImageCreateInfo.commandPool = m_commandPool;
    swapChainImageCreateInfo.graphicsQueue = m_graphicsQueue;
	swapChainImageCreateInfo.format = m_vulkanWindow->colorFormat();

    for (size_t i = 0; i < m_swapChainImages.size(); i++)
    {
        m_swapChainImages[i] = std::make_shared<GraphicsImage>(swapChainImageCreateInfo, 
            m_windowManager->GetVulkanWindow()->swapChainImage(i), 
            m_windowManager->GetVulkanWindow()->swapChainImageView(i)
        );
    }
}

void SwapChain::DestroySwapChain()
{
    
}

VkExtent2D SwapChain::GetSwapChainExtent()
{ 
    return { (uint32_t)m_vulkanWindow->swapChainImageSize().width(), (uint32_t)m_vulkanWindow->swapChainImageSize().height() }; 
}