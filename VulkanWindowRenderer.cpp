#include "VulkanWindowRenderer.h"
#include "VulkanInterface.h"

VulkanWindowRenderer::VulkanWindowRenderer(std::shared_ptr<VulkanInterface> vulkanInterface, std::shared_ptr<Scene> scene)
{
	m_vulkanInterface = vulkanInterface;
	m_scene = scene;
}

void VulkanWindowRenderer::preInitResources()
{
	
}

void VulkanWindowRenderer::initResources()
{
	m_vulkanInterface->InitializeVulkan();
}

void VulkanWindowRenderer::initSwapChainResources()
{
	m_vulkanInterface->InitializeSwapChain();
}

void VulkanWindowRenderer::releaseResources()
{
	m_scene->Cleanup();
	m_vulkanInterface->Cleanup();
}

void VulkanWindowRenderer::releaseSwapChainResources()
{
	m_vulkanInterface->CleanupSwapChain();
}

void VulkanWindowRenderer::startNextFrame()
{
	m_scene->Update();
	m_vulkanInterface->DrawFrame(0.0f, m_scene);
}