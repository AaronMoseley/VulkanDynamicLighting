#include "Scene.h"
#include "RenderObject.h"

Scene::Scene(std::shared_ptr<WindowManager> windowManager)
{
	m_windowManager = windowManager;
	m_vulkanInterface = std::make_unique<VulkanInterface>(windowManager);
}

void Scene::MainLoop()
{
    while (!glfwWindowShouldClose(m_windowManager->GetWindow())) {
        float currentFrameTime = glfwGetTime();
        m_deltaTime = currentFrameTime - m_lastFrame;
        m_lastFrame = currentFrameTime;

        glfwPollEvents();

        for (auto it = m_objects.begin(); it != m_objects.end(); it++)
        {
			std::vector<std::shared_ptr<ObjectComponent>> components = it->second->GetAllComponents();

            for (size_t i = 0; i < components.size(); i++)
            {
                if (!components[i]->IsEnabled())
                {
                    continue;
                }

                if (!components[i]->HasStarted())
                {
					components[i]->Start();
					components[i]->SetStarted(true);
                }

				components[i]->Update(m_deltaTime);
            }

			std::shared_ptr<MeshRenderer> meshComponent = it->second->GetComponent<MeshRenderer>();
            if (meshComponent == nullptr)
            {
                continue;
            }

            if (meshComponent->GetMeshName() != MeshRenderer::kCustomMeshName)
            {
                continue;
            }

            if (meshComponent->IsMeshDataDirty())
            {
                FinalizeMesh(it->second);
                meshComponent->SetDirtyData(false);
			}

            if (meshComponent->IsTextureDataDirty())
            {
                UpdateTexture(meshComponent->GetTexturePath());
				meshComponent->SetTextureDataDirty(false);
            }
        }

        for (size_t i = 0; i < updateCallbacks.size(); i++)
        {
			updateCallbacks[i](m_deltaTime);
        }

        m_vulkanInterface->DrawFrame(m_deltaTime, m_meshNameToObjectMap, m_objects);
        m_windowManager->NewFrame();

        if (m_windowManager->KeyPressed(GLFW_KEY_ESCAPE))
        {
            glfwSetWindowShouldClose(m_windowManager->GetWindow(), true);
        }
    }

    m_vulkanInterface->Cleanup();
}

VulkanCommonFunctions::ObjectHandle Scene::AddObject(std::shared_ptr <RenderObject> newObject)
{
    if (m_objects.size() >= VulkanCommonFunctions::MAX_OBJECTS)
    {
		return VulkanCommonFunctions::INVALID_OBJECT_HANDLE;
    }

    m_currentObjectHandle++;

    m_objects[m_currentObjectHandle] = newObject;
    newObject->SetSceneManager(this);

    std::shared_ptr<MeshRenderer> meshComponent = newObject->GetComponent<MeshRenderer>();

    if (meshComponent == nullptr)
    {
        return m_currentObjectHandle;
    }

	m_vulkanInterface->UpdateObjectBuffers(meshComponent);

    std::string objectName = meshComponent->GetMeshName();
    m_meshNameToObjectMap[objectName].insert(m_currentObjectHandle);

    if (meshComponent->GetTextured())
    {
        UpdateTexture(meshComponent->GetTexturePath());
    }

    return m_currentObjectHandle;
}

bool Scene::RemoveObject(VulkanCommonFunctions::ObjectHandle objectToRemove)
{
    std::shared_ptr<RenderObject> currentObject = GetRenderObject(objectToRemove);

    if (currentObject == nullptr)
    {
        return false;
    }

    currentObject->SetSceneManager(nullptr);

    bool removalSuccessful = true;

    removalSuccessful = m_objects.erase(objectToRemove);

    std::shared_ptr<MeshRenderer> meshComponent = currentObject->GetComponent<MeshRenderer>();

    if (meshComponent == nullptr)
    {
        return removalSuccessful;
    }

    std::string objectName = meshComponent->GetMeshName();

    if (!m_meshNameToObjectMap.contains(objectName))
        return false;

    removalSuccessful = removalSuccessful && m_meshNameToObjectMap[objectName].erase(objectToRemove);

    return removalSuccessful;
}

std::shared_ptr<RenderObject> Scene::GetRenderObject(VulkanCommonFunctions::ObjectHandle handle)
{
    if (handle == VulkanCommonFunctions::INVALID_OBJECT_HANDLE)
    {
        return nullptr;
    }
    if (!m_objects.contains(handle))
    {
        return nullptr;
    }
    return m_objects[handle];
}

void Scene::FinalizeMesh(std::shared_ptr<RenderObject> updatedObject)
{
    if (updatedObject == nullptr)
    {
        return;
	}

	std::shared_ptr<MeshRenderer> meshComponent = updatedObject->GetComponent<MeshRenderer>();
    if (meshComponent == nullptr)
    {
        return;
	}

    if (meshComponent->GetMeshName() != MeshRenderer::kCustomMeshName)
    {
        return;
    }

    std::shared_ptr<GraphicsBuffer> vertexBuffer = m_vulkanInterface->CreateVertexBuffer(meshComponent);
    std::shared_ptr<GraphicsBuffer> indexBuffer = m_vulkanInterface->CreateIndexBuffer(meshComponent);

	meshComponent->SetVertexBuffer(vertexBuffer);
	meshComponent->SetIndexBuffer(indexBuffer);

    if (updatedObject->GetInstanceBuffer() == nullptr)
    {
		GenerateInstanceBuffer(updatedObject);
    }
}

void Scene::GenerateInstanceBuffer(std::shared_ptr<RenderObject> newObject)
{
    if (newObject == nullptr)
    {
        return;
    }

    std::shared_ptr<MeshRenderer> meshComponent = newObject->GetComponent<MeshRenderer>();
    if (meshComponent == nullptr)
    {
        return;
    }

	std::shared_ptr<GraphicsBuffer> instanceBuffer = m_vulkanInterface->CreateInstanceBuffer(1);
    newObject->SetInstanceBuffer(instanceBuffer);
}

void Scene::UpdateTexture(std::string newTexturePath)
{
    if (m_vulkanInterface->HasTexture(newTexturePath))
    {
        return;
    }

    m_vulkanInterface->UpdateTextureResources(newTexturePath);
}