#pragma once

#include "GraphicsImage.h"
#include "WindowManager.h"
#include "VulkanCommonFunctions.h"

#include <vector>
#include <array>
#include <algorithm>
#include <memory>

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

	VkExtent2D GetSwapChainExtent() { return m_swapChainExtent; }
	VkFramebuffer GetFrameBuffer(size_t index) { return m_swapChainFramebuffers[index]; }
	GraphicsImage* GetImage(size_t index) { return m_swapChainImages[index]; }
	VkSwapchainKHR GetVkSwapChain() { return m_swapChain; }

	static bool IsSwapChainAdequate(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

	static SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

	void CreateFrameBuffers(std::shared_ptr<GraphicsImage> depthImage, VkRenderPass renderPass);
	void ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	void DestroySwapChain();

private:
	std::vector<GraphicsImage*> m_swapChainImages;
	VkSwapchainKHR m_swapChain;
	VkExtent2D m_swapChainExtent;
	std::vector<VkFramebuffer> m_swapChainFramebuffers;

	VkDevice m_device;
	VkPhysicalDevice m_physicalDevice;
	VkSurfaceKHR m_surface;

	VmaAllocator m_allocator;
	VkCommandPool m_commandPool;
	VkQueue m_graphicsQueue;

	std::shared_ptr<WindowManager> m_windowManager;
};