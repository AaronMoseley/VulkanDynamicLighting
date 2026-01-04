#include "VoltEngine.h"

VoltEngine::VoltEngine(QWidget* parent, QVulkanInstance* vulkanInstance, int screenWidth, int screenHeight) : QWidget(parent) {
    //m_mainLayout = new QVBoxLayout(this);

    m_windowManager = new WindowManager(this, screenWidth, screenHeight, "Vulkan Lighting Demo");

    m_vulkanInterface = std::make_shared<VulkanInterface>(m_windowManager);

    m_windowManager->SetVulkanInterface(m_vulkanInterface);

    m_sceneManager = std::make_shared<Scene>(m_windowManager, m_vulkanInterface);

    m_windowManager->SetScene(m_sceneManager);

    m_windowManager->InitializeWindow(vulkanInstance);
    resize(screenWidth, screenHeight);
    //m_mainLayout->addWidget(m_windowManager->GetWrappingWidget());

    //can add custom callbacks that get called each frame
    //auto frameCallback = std::bind(&VulkanLightingDemo::processInput, this, std::placeholders::_1);
    //sceneManager->RegisterUpdateCallback(frameCallback);
}

void VoltEngine::BeginRendering() {
    m_windowManager->BeginRendering();
}

void VoltEngine::RegisterUpdateCallback(std::function<void(float)> callback) {
    m_sceneManager->RegisterUpdateCallback(callback);
}