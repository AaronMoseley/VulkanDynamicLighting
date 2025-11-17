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
}

void SwapChain::DestroySwapChain()
{
    
}

VkExtent2D SwapChain::GetSwapChainExtent()
{ 
    return { (uint32_t)m_vulkanWindow->swapChainImageSize().width(), (uint32_t)m_vulkanWindow->swapChainImageSize().height() }; 
}