#include "VoltEngine.h"

VoltEngine::VoltEngine(QWidget* parent, QVulkanInstance* vulkanInstance, int screenWidth, int screenHeight) : QWidget(parent) {
    m_mainLayout = new QVBoxLayout(this);

    m_windowManager = std::make_shared<WindowManager>(this, screenWidth, screenHeight, "Vulkan Lighting Demo");

    m_vulkanInterface = std::make_shared<VulkanInterface>(m_windowManager);

    m_windowManager->SetVulkanInterface(m_vulkanInterface);

    m_sceneManager = std::make_shared<Scene>(m_windowManager, m_vulkanInterface);

    m_windowManager->SetScene(m_sceneManager);

    m_windowManager->InitializeWindow(vulkanInstance);
    resize(screenWidth, screenHeight);
    m_mainLayout->addWidget(m_windowManager->GetWrappingWidget());

    //can add custom callbacks that get called each frame
    //auto frameCallback = std::bind(&VulkanLightingDemo::processInput, this, std::placeholders::_1);
    //sceneManager->RegisterUpdateCallback(frameCallback);

    //need to create a camera objects before initializing rendering
    std::shared_ptr<RenderObject> cameraObject = std::make_shared<RenderObject>(m_windowManager);

    std::shared_ptr<Transform> cameraTransform = cameraObject->AddComponent<Transform>();
    cameraTransform->SetPosition(glm::vec3(0.0f, 0.0f, 5.0f));
    cameraTransform->SetRotation(glm::vec3(0.0f, -90.0f, 0.0f));
    cameraTransform->SetScale(glm::vec3(1.0f));
    cameraObject->AddComponent<Camera>();
    cameraObject->AddComponent<FirstPersonController>();
    cameraObject->AddComponent<DemoBehavior>();
    cameraObject->SetTag("Player");
    m_sceneManager->AddObject(cameraObject);

    m_windowManager->BeginRendering();
}