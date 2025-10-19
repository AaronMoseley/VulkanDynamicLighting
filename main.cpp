#define VMA_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#include <vector>
#include <set>

#include "RenderObject.h"
#include "Cube.h"
#include "Tetrahedron.h"
#include "Camera.h"
#include "WindowManager.h"
#include "VulkanInterface.h"
#include "Factory.h"
#include "FirstPersonController.h"

class VulkanLightingDemo {
public:
    void run() {
        windowManager = new WindowManager(800, 600, "Vulkan Demo");

        //init vulkan
        vulkanInterface = new VulkanInterface(windowManager);

        CreateObjects();
        mainLoop();

        vulkanInterface->Cleanup();
    }

private:
    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;

    WindowManager* windowManager;
    VulkanInterface* vulkanInterface;
    
    VulkanInterface::ObjectHandle lightObjectHandle;
    std::set<VulkanInterface::ObjectHandle> objectHandles;
	VulkanInterface::ObjectHandle cameraObjectHandle;

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

    bool partyMode = false;

    void CreateObjects()
    {
        std::srand(std::time(0));

        RenderObject* cameraObject = new RenderObject(
            windowManager,
            glm::vec3(0.0f, 0.0f, -5.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(1.0f)
		);
		cameraObject->AddComponent<Camera>();
        cameraObject->AddComponent<FirstPersonController>();
		cameraObjectHandle = vulkanInterface->AddObject(cameraObject);

        RenderObject* lightCube = new RenderObject(
            windowManager,
            glm::vec3(lightOrbitRadius),
            glm::vec3(0.0f),
            glm::vec3(0.25f)
        );

        lightCube->AddComponent<Cube>();
		MeshRenderer* lightMesh = lightCube->GetComponent<MeshRenderer>();
		lightMesh->SetLit(false);
		lightMesh->SetColor(glm::vec3(1.0f, 1.0f, 1.0f));
		lightCube->AddComponent<LightSource>();

        lightObjectHandle = vulkanInterface->AddObject(lightCube);
        objectHandles.insert(lightObjectHandle);

        glm::vec3 color = glm::vec3(0.6588f, 0.2235f, 0.0392f);
        //glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);

        for (int i = 0; i < objectPositions.size(); i++)
        {
            RenderObject* newObject = new RenderObject(
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

			MeshRenderer* currentMesh = newObject->GetComponent<MeshRenderer>();

			currentMesh->SetColor(color);

            if (vulkanInterface->GetObjectCount() % 3 == 0)
            {
                currentMesh->SetTexture(0);
            }
            else if (vulkanInterface->GetObjectCount() % 3 == 1)
            {
                currentMesh->SetTexture(1);
            }
            else {
                currentMesh->SetTextured(false);
            }

            VulkanInterface::ObjectHandle newObjectHandle = vulkanInterface->AddObject(newObject);
            objectHandles.insert(newObjectHandle);
        }
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(windowManager->GetWindow())) {
            float currentFrameTime = glfwGetTime();
            deltaTime = currentFrameTime - lastFrame;
            lastFrame = currentFrameTime;

            glfwPollEvents();
            processInput(windowManager->GetWindow());

            for (auto it = objectHandles.begin(); it != objectHandles.end(); it++)
            {
                std::vector<ObjectComponent*> components = vulkanInterface->GetRenderObject(*it)->GetAllComponents();

                for (size_t i = 0; i < components.size(); i++)
                {
                    components[i]->Update(deltaTime);
                }
            }

            vulkanInterface->DrawFrame(deltaTime);
            windowManager->NewFrame();
        }
    }

    void processInput(GLFWwindow* window)
    {
        float currentFrameTime = glfwGetTime();

        RenderObject* lightObject = vulkanInterface->GetRenderObject(lightObjectHandle);
        if (lightObject != nullptr)
        {
            lightObject->GetComponent<Transform>()->SetPosition(glm::vec3(lightOrbitRadius * cos(currentFrameTime), lightOrbitRadius * sin(currentFrameTime), lightOrbitRadius * cos(currentFrameTime)));
        }

        for (auto it = objectHandles.begin(); it != objectHandles.end(); it++)
        {
            RenderObject* currentObject = vulkanInterface->GetRenderObject(*it);

            if (currentObject == nullptr)
            {
                continue;
            }

            currentObject->GetComponent<Transform>()->Rotate(glm::vec3(16.0f * deltaTime));
        }

        if (!vulkanInterface->HasRenderedFirstFrame())
            return;

        if (windowManager->KeyPressed(GLFW_KEY_ESCAPE))
            glfwSetWindowShouldClose(window, true);

        if (windowManager->KeyPressed(GLFW_KEY_R) && vulkanInterface->GetObjectCount() < maxObjects)
        {
            float positionRange = 100.0f;

            RenderObject* newObject = new RenderObject(
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

			MeshRenderer* currentMesh = newObject->GetComponent<MeshRenderer>();

            currentMesh->SetColor(glm::vec3(0.9f));

            if (vulkanInterface->GetObjectCount() % 3 == 0)
            {
                currentMesh->SetTextured(true);
                currentMesh->SetTexture(0);
            }
            else if (vulkanInterface->GetObjectCount() % 3 == 1)
            {
                currentMesh->SetTextured(true);
                currentMesh->SetTexture(1);
            }
            else {
                currentMesh->SetTextured(false);
            }

            if ((double)rand() / (RAND_MAX) >= 0.99f)
            {
                LightSource* newLightSource = newObject->AddComponent<LightSource>();

				glm::vec3 lightColor = glm::vec3(((double)rand() / (RAND_MAX)), ((double)rand() / (RAND_MAX)), ((double)rand() / (RAND_MAX)));

				newLightSource->SetColor(lightColor);
				newObject->GetComponent<MeshRenderer>()->SetColor(lightColor);
				newObject->GetComponent<MeshRenderer>()->SetLit(false);
            }

            VulkanInterface::ObjectHandle newObjectHandle = vulkanInterface->AddObject(newObject);
            objectHandles.insert(newObjectHandle);
        }

        if (windowManager->KeyPressed(GLFW_KEY_E) && vulkanInterface->GetObjectCount() > 1)
        {
            VulkanInterface::ObjectHandle removeObjectHandle = *objectHandles.rbegin();
            bool correctlyRemoved = vulkanInterface->RemoveObject(removeObjectHandle);
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