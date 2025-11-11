#include "VulkanWindow.h"
#include "VulkanInterface.h"

VulkanWindow::VulkanWindow(std::shared_ptr<VulkanInterface> vulkanInterface, std::shared_ptr<Scene> scene)
{
	m_vulkanInterface = vulkanInterface;
	m_scene = scene;
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