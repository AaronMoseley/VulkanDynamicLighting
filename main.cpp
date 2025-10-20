#define VMA_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#include <vector>
#include <set>
#include <memory>

#include "RenderObject.h"
#include "Cube.h"
#include "Tetrahedron.h"
#include "Camera.h"
#include "WindowManager.h"
#include "FirstPersonController.h"
#include "Scene.h"

class VulkanLightingDemo {
public:
    void run() {
        windowManager = std::make_shared<WindowManager>(800, 600, "Vulkan Demo");

		sceneManager = std::make_shared<Scene>(windowManager);

        auto frameCallback = std::bind(&VulkanLightingDemo::processInput, this, std::placeholders::_1);
        sceneManager->RegisterUpdateCallback(frameCallback);

        CreateObjects();
		sceneManager->MainLoop();
    }

private:
    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;

    std::shared_ptr<WindowManager> windowManager;
	std::shared_ptr<Scene> sceneManager;
    
    VulkanCommonFunctions::ObjectHandle lightObjectHandle;
    std::set<VulkanCommonFunctions::ObjectHandle> objectHandles;
    VulkanCommonFunctions::ObjectHandle cameraObjectHandle;

    size_t maxObjects = 10000;

    float lightOrbitRadius = 5.0f;
    float lightOrbitSpeed = 1.0f;

    float deltaTime = 0.0f;	// Time between current frame and last frame
    float lastFrame = 0.0f; // Time of last frame

    std::vector<glm::vec3> objectPositions = {
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

    void CreateObjects()
    {
        std::srand(std::time(0));

        std::shared_ptr<RenderObject> cameraObject = std::make_shared<RenderObject>(
            windowManager,
            glm::vec3(0.0f, 0.0f, 5.0f),
            glm::vec3(0.0f, -90.0f, 0.0f),
            glm::vec3(1.0f)
		);
		cameraObject->AddComponent<Camera>();
        cameraObject->AddComponent<FirstPersonController>();
		cameraObjectHandle = sceneManager->AddObject(cameraObject);

        std::shared_ptr<RenderObject> lightCube = std::make_shared<RenderObject>(
            windowManager,
            glm::vec3(lightOrbitRadius),
            glm::vec3(0.0f),
            glm::vec3(0.25f)
        );

        lightCube->AddComponent<Cube>();
		std::shared_ptr<MeshRenderer> lightMesh = lightCube->GetComponent<MeshRenderer>();
		lightMesh->SetLit(false);
		lightMesh->SetColor(glm::vec3(1.0f, 1.0f, 1.0f));
		lightCube->AddComponent<LightSource>();

        lightObjectHandle = sceneManager->AddObject(lightCube);
        objectHandles.insert(lightObjectHandle);

        glm::vec3 color = glm::vec3(0.6588f, 0.2235f, 0.0392f);
        //glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);

        for (int i = 0; i < objectPositions.size(); i++)
        {
            std::shared_ptr<RenderObject> newObject = std::make_shared<RenderObject>(
                windowManager,
                objectPositions[i],
                glm::vec3(((double)rand() / (RAND_MAX)) * 360.0f, ((double)rand() / (RAND_MAX)) * 360.0f, ((double)rand() / (RAND_MAX)) * 360.0f),
                glm::vec3(0.5f, 0.5f, 0.5f)
            );

            if ((double)rand() / (RAND_MAX) >= 0.5f)
            {
				newObject->AddComponent<Cube>();
            }
            else {
				newObject->AddComponent<Tetrahedron>();
            }

			std::shared_ptr<MeshRenderer> currentMesh = newObject->GetComponent<MeshRenderer>();

			currentMesh->SetColor(color);

            if (sceneManager->GetObjectCount() % 3 == 0)
            {
                currentMesh->SetTexture(0);
            }
            else if (sceneManager->GetObjectCount() % 3 == 1)
            {
                currentMesh->SetTexture(1);
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
        float currentFrameTime = glfwGetTime();

        std::shared_ptr<RenderObject> lightObject = sceneManager->GetRenderObject(lightObjectHandle);
        if (lightObject != nullptr)
        {
            lightObject->GetComponent<Transform>()->SetPosition(glm::vec3(lightOrbitRadius * cos(currentFrameTime), lightOrbitRadius * sin(currentFrameTime), lightOrbitRadius * cos(currentFrameTime)));
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

        if (windowManager->KeyPressed(GLFW_KEY_R) && sceneManager->GetObjectCount() < maxObjects)
        {
            float positionRange = 100.0f;

            std::shared_ptr<RenderObject> newObject = std::make_shared<RenderObject>(
                windowManager,
                glm::vec3(((double)rand() / (RAND_MAX)) * positionRange, ((double)rand() / (RAND_MAX)) * positionRange, ((double)rand() / (RAND_MAX)) * positionRange),
                glm::vec3(((double)rand() / (RAND_MAX)) * 360.0f, ((double)rand() / (RAND_MAX)) * 360.0f, ((double)rand() / (RAND_MAX)) * 360.0f),
                glm::vec3(0.5f, 0.5f, 0.5f)
            );

            if ((double)rand() / (RAND_MAX) >= 0.5f)
            {
				newObject->AddComponent<Cube>();
            }
            else {
				newObject->AddComponent<Tetrahedron>();
            }

			std::shared_ptr<MeshRenderer> currentMesh = newObject->GetComponent<MeshRenderer>();

            currentMesh->SetColor(glm::vec3(0.9f));

            if (sceneManager->GetObjectCount() % 3 == 0)
            {
                currentMesh->SetTextured(true);
                currentMesh->SetTexture(0);
            }
            else if (sceneManager->GetObjectCount() % 3 == 1)
            {
                currentMesh->SetTextured(true);
                currentMesh->SetTexture(1);
            }
            else {
                currentMesh->SetTextured(false);
            }

            if ((double)rand() / (RAND_MAX) >= 0.99f)
            {
                std::shared_ptr<LightSource> newLightSource = newObject->AddComponent<LightSource>();

				glm::vec3 lightColor = glm::vec3(((double)rand() / (RAND_MAX)), ((double)rand() / (RAND_MAX)), ((double)rand() / (RAND_MAX)));

				newLightSource->SetColor(lightColor);
				newObject->GetComponent<MeshRenderer>()->SetColor(lightColor);
				newObject->GetComponent<MeshRenderer>()->SetLit(false);
            }

            VulkanCommonFunctions::ObjectHandle newObjectHandle = sceneManager->AddObject(newObject);
            objectHandles.insert(newObjectHandle);
        }

        if (windowManager->KeyPressed(GLFW_KEY_E) && sceneManager->GetObjectCount() > 1)
        {
            VulkanCommonFunctions::ObjectHandle removeObjectHandle = *objectHandles.rbegin();
            bool correctlyRemoved = sceneManager->RemoveObject(removeObjectHandle);
            objectHandles.erase(removeObjectHandle);

            if (!correctlyRemoved)
            {
                std::cout << "Error removing object, handle: " << removeObjectHandle << std::endl;
            }
        }
    }
};

int main() {
    VulkanLightingDemo app;

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}