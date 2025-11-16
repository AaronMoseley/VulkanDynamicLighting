#include "VulkanWindow.h"
#include "VulkanInterface.h"

VulkanWindow::VulkanWindow(std::shared_ptr<VulkanInterface> vulkanInterface, std::shared_ptr<Scene> scene)
{
	m_vulkanInterface = vulkanInterface;
	m_scene = scene;

	m_indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
	m_indexingFeatures.runtimeDescriptorArray = VK_TRUE;
	m_indexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;

	setEnabledFeaturesModifier([=](VkPhysicalDeviceFeatures2& features2) {
		// Chain it to the pNext of the main features struct
		m_indexingFeatures.pNext = features2.pNext;
		features2.pNext = &m_indexingFeatures;

		features2.features.samplerAnisotropy = VK_TRUE;
		});

	requestUpdate();
}

void VulkanWindow::mousePressEvent(QMouseEvent* event)
{

}

void VulkanWindow::mouseReleaseEvent(QMouseEvent* event)
{

}

void VulkanWindow::mouseMoveEvent(QMouseEvent* event)
{
	for (size_t i = 0; i < m_mouseMoveCallbacks.size(); i++)
	{
		m_mouseMoveCallbacks[i](event);
	}
}

QVulkanWindowRenderer* VulkanWindow::createRenderer()
{
	m_vulkanWindowRenderer = std::make_shared<VulkanWindowRenderer>(m_vulkanInterface, m_scene);

	return m_vulkanWindowRenderer.get();
}