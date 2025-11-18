#define VMA_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#include <vector>
#include <set>
#include <memory>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

#include "RenderObject.h"
#include "Cube.h"
#include "Tetrahedron.h"
#include "Camera.h"
#include "WindowManager.h"
#include "FirstPersonController.h"
#include "Scene.h"
#include "VulkanWindow.h"
#include "DemoBehavior.h"

#include <QApplication>
#include <QLoggingCategory>
#include <QScreen>
#include <QVulkanInstance>
#include <QVBoxLayout>
#include <vulkan/vulkan.h>

#include <qwidget.h>

bool DebugFilter(QVulkanInstance::DebugMessageSeverityFlags severity, QVulkanInstance::DebugMessageTypeFlags type, const void* message)
{
    auto* data = reinterpret_cast<const VkDebugUtilsMessengerCallbackDataEXT*>(message);

    qDebug().noquote()
        << "[VK]"
        << "Severity:" << severity
        << "Type:" << type
        << data->pMessage;

    return true;   // return true to stop Qt from printing it as well
}

class VulkanLightingDemo : public QWidget {
public:
    VulkanLightingDemo(QWidget* parent, QVulkanInstance* vulkanInstance, int screenWidth, int screenHeight) : QWidget(parent) {
		m_mainLayout = new QVBoxLayout(this);
        
        windowManager = std::make_shared<WindowManager>(this, screenWidth, screenHeight, "Vulkan Lighting Demo");

        vulkanInterface = std::make_shared<VulkanInterface>(windowManager);

        windowManager->SetVulkanInterface(vulkanInterface);

        sceneManager = std::make_shared<Scene>(windowManager, vulkanInterface);

        windowManager->SetScene(sceneManager);

        windowManager->InitializeWindow(vulkanInstance);
        resize(screenWidth, screenHeight);
        m_mainLayout->addWidget(windowManager->GetWrappingWidget());

		//can add custom callbacks that get called each frame
        //auto frameCallback = std::bind(&VulkanLightingDemo::processInput, this, std::placeholders::_1);
        //sceneManager->RegisterUpdateCallback(frameCallback);

        //need to create a camera objects before initializing rendering
        std::shared_ptr<RenderObject> cameraObject = std::make_shared<RenderObject>(windowManager);

        std::shared_ptr<Transform> cameraTransform = cameraObject->AddComponent<Transform>();
        cameraTransform->SetPosition(glm::vec3(0.0f, 0.0f, 5.0f));
        cameraTransform->SetRotation(glm::vec3(0.0f, -90.0f, 0.0f));
        cameraTransform->SetScale(glm::vec3(1.0f));
        cameraObject->AddComponent<Camera>();
        cameraObject->AddComponent<FirstPersonController>();
		cameraObject->AddComponent<DemoBehavior>();
        sceneManager->AddObject(cameraObject);

        windowManager->BeginRendering();
    }

private:
    std::shared_ptr<WindowManager> windowManager;
	std::shared_ptr<Scene> sceneManager;
    std::shared_ptr<VulkanInterface> vulkanInterface;
    
    QVBoxLayout* m_mainLayout;
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    QVulkanInstance instance;
    instance.setLayers({ "VK_LAYER_KHRONOS_validation" });
    instance.setApiVersion(QVersionNumber(1, 3, 0));
    instance.setExtensions({
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
#ifdef _DEBUG
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#endif
    });
    instance.installDebugOutputFilter(DebugFilter);

    bool success = instance.create();

    if (!success)
    {
        qDebug() << "Failed to create Vulkan instance";
        return -1;
    }

    auto screenRect = QRect(0, 0, 915, 400);
    qDebug() << "Screen Size: " << screenRect.width() << " x " << screenRect.height();

    VulkanLightingDemo renderingApp(nullptr, &instance, screenRect.width(), screenRect.height());
    renderingApp.show();

    return app.exec();
}