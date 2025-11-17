#pragma once

#include "GraphicsImage.h"
#include "VulkanCommonFunctions.h"

#include <vector>
#include <array>
#include <algorithm>
#include <memory>

class WindowManager;
class VulkanWindow;

class SwapChain {
public:
	struct SwapChainCreateInfo {
		VkPhysicalDevice physicalDevice;
		VkDevice device;
		VkSurfaceKHR surface;
		std::shared_ptr<WindowManager> windowManager;
		VmaAllocator allocator;
		VkCommandPool commandPool;
		VkQueue graphicsQueue;
	};

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	SwapChain(SwapChainCreateInfo initializationInfo);

	VkExtent2D GetSwapChainExtent();

	void DestroySwapChain();

private:
	VkDevice m_device;
	VkPhysicalDevice m_physicalDevice;
	VkSurfaceKHR m_surface;

	VmaAllocator m_allocator;
	VkCommandPool m_commandPool;
	VkQueue m_graphicsQueue;

	std::shared_ptr<WindowManager> m_windowManager;
	VulkanWindow* m_vulkanWindow;
};