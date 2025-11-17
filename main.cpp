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
		m_mainLayout.reset(new QVBoxLayout(this));
        
        windowManager = std::make_shared<WindowManager>(screenWidth, screenHeight, "Vulkan Lighting Demo");

        vulkanInterface = std::make_shared<VulkanInterface>(windowManager);

        windowManager->SetVulkanInterface(vulkanInterface);

        sceneManager = std::make_shared<Scene>(windowManager, vulkanInterface);

        windowManager->SetScene(sceneManager);

        windowManager->InitializeWindow(vulkanInstance);
        resize(screenWidth, screenHeight);
        m_mainLayout->addWidget(windowManager->GetWrappingWidget());

        auto frameCallback = std::bind(&VulkanLightingDemo::processInput, this, std::placeholders::_1);
        sceneManager->RegisterUpdateCallback(frameCallback);

        std::shared_ptr<RenderObject> cameraObject = std::make_shared<RenderObject>(windowManager);

        std::shared_ptr<Transform> cameraTransform = cameraObject->AddComponent<Transform>();
        cameraTransform->SetPosition(glm::vec3(0.0f, 0.0f, 5.0f));
        cameraTransform->SetRotation(glm::vec3(0.0f, -90.0f, 0.0f));
        cameraTransform->SetScale(glm::vec3(1.0f));
        cameraObject->AddComponent<Camera>();
        cameraObject->AddComponent<FirstPersonController>();
        cameraObjectHandle = sceneManager->AddObject(cameraObject);

        //sceneManager->MainLoop();
        windowManager->BeginRendering();
    }

private:
    std::shared_ptr<WindowManager> windowManager;
	std::shared_ptr<Scene> sceneManager;
    std::shared_ptr<VulkanInterface> vulkanInterface;
    
    VulkanCommonFunctions::ObjectHandle lightObjectHandle = 0;
    std::set<VulkanCommonFunctions::ObjectHandle> objectHandles;
    VulkanCommonFunctions::ObjectHandle cameraObjectHandle = 0;

    size_t maxObjects = 10000;

    float lightOrbitRadius = 5.0f;
    float lightOrbitSpeed = 1.0f;

    float currentTime = 0.0f;

    float deltaTime = 0.0f;	// Time between current frame and last frame
    float lastFrame = 0.0f; // Time of last frame

    alignas(16) std::vector<glm::vec3> objectPositions = {
        glm::vec3(0.0f,  0.0f,  0.0f),
        glm::vec3(2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3(2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3(1.3f, -2.0f, -2.5f),
        glm::vec3(1.5f,  2.0f, -2.5f),
        glm::vec3(1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
    };

    std::vector<VulkanCommonFunctions::Vertex> squareVertices = {
        //positions          //normals           //texture coords
        {{-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, //top left
        {{ 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, //top right
        {{ 0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}, //bottom right
        {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}  //bottom left
	};

    std::vector<uint16_t> squareIndices = {
        0, 1, 2, 2, 3, 0
	};

    bool temp = false;

    QScopedPointer<QVBoxLayout> m_mainLayout;

    void CreateObjects()
    {
        std::srand(std::time(0));

        std::shared_ptr<RenderObject> lightCube = std::make_shared<RenderObject>(windowManager);

		std::shared_ptr<Transform> lightTransform = lightCube->AddComponent<Transform>();
		lightTransform->SetPosition(glm::vec3(lightOrbitRadius, lightOrbitRadius, lightOrbitRadius));
		lightTransform->SetRotation(glm::vec3(0.0f));
		lightTransform->SetScale(glm::vec3(0.25f));
        //std::shared_ptr<Cube> lightMesh = lightCube->AddComponent<Cube>();
		std::shared_ptr<Cube> lightMesh = lightCube->AddComponent<Cube>();
        //lightMesh->SetVertices(squareVertices);
		//lightMesh->SetIndices(squareIndices);
		lightMesh->SetLit(false);
		lightMesh->SetColor(glm::vec3(1.0f, 1.0f, 1.0f));
		lightCube->AddComponent<LightSource>();

        lightObjectHandle = sceneManager->AddObject(lightCube);
        objectHandles.insert(lightObjectHandle);

        glm::vec3 color = glm::vec3(0.6588f, 0.2235f, 0.0392f);
        //glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);

        for (int i = 0; i < objectPositions.size(); i++)
        {
            std::shared_ptr<RenderObject> newObject = std::make_shared<RenderObject>(windowManager);

			std::shared_ptr<Transform> newObjectTransform = newObject->AddComponent<Transform>();
            newObjectTransform->SetPosition(objectPositions[i]);
            newObjectTransform->SetRotation(glm::vec3(((double)rand() / (RAND_MAX)) * 360.0f, ((double)rand() / (RAND_MAX)) * 360.0f, ((double)rand() / (RAND_MAX)) * 360.0f));
            newObjectTransform->SetScale(glm::vec3(0.5f));

            if ((double)rand() / (RAND_MAX) >= 0.5f)
            {
				newObject->AddComponent<Cube>();
            }
            else {
				newObject->AddComponent<Tetrahedron>();
            }

			std::shared_ptr<MeshRenderer> currentMesh = newObject->GetComponent<MeshRenderer>();

            currentMesh->SetOpacity(((double)rand() / (RAND_MAX)));

            if ((double)rand() / (RAND_MAX) >= 0.5f)
            {
                currentMesh->SetIsBillboarded(true);
                newObject->GetComponent<Transform>()->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));
            }

			currentMesh->SetColor(color);

            //currentMesh->SetColor({ 1.0f, 1.0f, 1.0f });

            if (sceneManager->GetObjectCount() % 3 == 0)
            {
                currentMesh->SetTexture("textures\\SandTexture.png");
            }
            else if (sceneManager->GetObjectCount() % 3 == 1)
            {
                currentMesh->SetTexture("textures\\OtherTexture.png");
            }
            else {
                currentMesh->SetTextured(false);
            }

            VulkanCommonFunctions::ObjectHandle newObjectHandle = sceneManager->AddObject(newObject);
            objectHandles.insert(newObjectHandle);
        }
    }

    void processInput(float deltaTime)
    {
        if (!temp)
        {
            CreateObjects();
            temp = true;
        }

        currentTime += deltaTime;

        std::shared_ptr<RenderObject> lightObject = sceneManager->GetRenderObject(lightObjectHandle);
        if (lightObject != nullptr)
        {
            lightObject->GetComponent<Transform>()->SetPosition(glm::vec3(lightOrbitRadius * cos(currentTime), lightOrbitRadius * sin(currentTime), lightOrbitRadius * cos(currentTime)));
        }

        if (windowManager->KeyPressedThisFrame(Qt::Key::Key_T))
        {
            std::vector<uint16_t> triangleIndices = {
            0, 1, 2
            };

            lightObject->GetComponent<MeshRenderer>()->SetIndices(triangleIndices);
        }

        for (auto it = objectHandles.begin(); it != objectHandles.end(); it++)
        {
            std::shared_ptr<RenderObject> currentObject = sceneManager->GetRenderObject(*it);

            if (currentObject == nullptr)
            {
                continue;
            }

            currentObject->GetComponent<Transform>()->Rotate(glm::vec3(16.0f * deltaTime));
        }

        if (windowManager->KeyPressed(Qt::Key::Key_R) && sceneManager->GetObjectCount() < maxObjects)
        {
            float positionRange = 100.0f;

            std::shared_ptr<RenderObject> newObject = std::make_shared<RenderObject>(windowManager);

            std::shared_ptr<Transform> newObjectTransform = newObject->AddComponent<Transform>();
            newObjectTransform->SetPosition(glm::vec3(((double)rand() / (RAND_MAX)) * positionRange, ((double)rand() / (RAND_MAX)) * positionRange, ((double)rand() / (RAND_MAX)) * positionRange));
            newObjectTransform->SetRotation(glm::vec3(((double)rand() / (RAND_MAX)) * 360.0f, ((double)rand() / (RAND_MAX)) * 360.0f, ((double)rand() / (RAND_MAX)) * 360.0f));
            newObjectTransform->SetScale(glm::vec3(0.5f));

            if ((double)rand() / (RAND_MAX) >= 0.5f)
            {
				newObject->AddComponent<Cube>();
            }
            else {
				newObject->AddComponent<Tetrahedron>();
            }

			std::shared_ptr<MeshRenderer> currentMesh = newObject->GetComponent<MeshRenderer>();

            currentMesh->SetOpacity(((double)rand() / (RAND_MAX)));

            if ((double)rand() / (RAND_MAX) >= 0.5f)
            {
                currentMesh->SetIsBillboarded(true);
				newObject->GetComponent<Transform>()->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));
            }

            currentMesh->SetColor(glm::vec3(0.9f));

            if (sceneManager->GetObjectCount() % 3 == 0)
            {
                currentMesh->SetTexture("textures\\SandTexture.png");
            }
            else if (sceneManager->GetObjectCount() % 3 == 1)
            {
                currentMesh->SetTexture("textures\\OtherTexture.png");
            }
            else {
                currentMesh->SetTextured(false);
            }

            if ((double)rand() / (RAND_MAX) >= 0.95f)
            {
                std::shared_ptr<LightSource> newLightSource = newObject->AddComponent<LightSource>();

				glm::vec3 lightColor = glm::vec3(((double)rand() / (RAND_MAX)), ((double)rand() / (RAND_MAX)), ((double)rand() / (RAND_MAX)));
				//glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);

				newLightSource->SetColor(lightColor);
				newObject->GetComponent<MeshRenderer>()->SetColor(lightColor);
				newObject->GetComponent<MeshRenderer>()->SetLit(false);
            }

            VulkanCommonFunctions::ObjectHandle newObjectHandle = sceneManager->AddObject(newObject);
            objectHandles.insert(newObjectHandle);
        }

        if (windowManager->KeyPressed(Qt::Key::Key_E) && sceneManager->GetObjectCount() > 1)
        {
            VulkanCommonFunctions::ObjectHandle removeObjectHandle = *objectHandles.rbegin();
            bool correctlyRemoved = sceneManager->RemoveObject(removeObjectHandle);
            objectHandles.erase(removeObjectHandle);

            if (!correctlyRemoved)
            {
                std::cout << "Error removing object, handle: " << removeObjectHandle << std::endl;
            }
        }

        if (windowManager->KeyPressedThisFrame(Qt::Key::Key_Escape))
        {
            windowManager->Shutdown();
            QCoreApplication::quit();
        }
    }
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