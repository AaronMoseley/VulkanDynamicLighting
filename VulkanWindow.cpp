#include "VulkanWindow.h"
#include "VulkanInterface.h"

VulkanWindow::VulkanWindow(std::shared_ptr<VulkanInterface> vulkanInterface, std::shared_ptr<Scene> scene) : QVulkanWindow(nullptr)
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

	//setMouseGrabEnabled(true);
	setCursor(Qt::BlankCursor);
}

void VulkanWindow::mousePressEvent(QMouseEvent* event)
{
	Qt::MouseButton button = event->button();

	emit MouseButtonDown(button);
}

void VulkanWindow::mouseReleaseEvent(QMouseEvent* event)
{
	Qt::MouseButton button = event->button();

	emit MouseButtonUp(button);
}

void VulkanWindow::mouseMoveEvent(QMouseEvent* event)
{
	emit MouseMoved(((float)event->pos().x() / size().width()) - 0.5f, ((float)event->pos().y() / size().height()) - 0.5f);

	QPoint center = mapToGlobal(QPoint(width() / 2, height() / 2));
	QCursor::setPos(center);
}

void VulkanWindow::keyPressEvent(QKeyEvent* event)
{
	Qt::Key key = static_cast<Qt::Key>(event->key());

	emit KeyDown(key);
}

void VulkanWindow::keyReleaseEvent(QKeyEvent* event)
{
	Qt::Key key = static_cast<Qt::Key>(event->key());

	emit KeyUp(key);
}

QVulkanWindowRenderer* VulkanWindow::createRenderer()
{
	m_vulkanWindowRenderer = new VulkanWindowRenderer(m_vulkanInterface, m_scene);

	return m_vulkanWindowRenderer;
}

void VulkanWindow::Shutdown()
{
	m_vulkanWindowRenderer->Shutdown();
}