#include "Scene.h"
#include "RenderObject.h"
#include "Cube.h"

Scene::Scene(std::shared_ptr<WindowManager> windowManager, std::shared_ptr<VulkanInterface> vulkanInterface)
{
	m_windowManager = windowManager;
    m_vulkanInterface = vulkanInterface;
}

void Scene::Update()
{
    auto now = std::chrono::high_resolution_clock::now();
    auto epoch = now.time_since_epoch();

    double currentFrameTime = std::chrono::duration<double>(epoch).count();

    if (m_lastFrame > 0.0f)
    {
        m_deltaTime = currentFrameTime - m_lastFrame;
    }

    m_lastFrame = currentFrameTime;

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

    for (size_t i = 0; i < m_updateCallbacks.size(); i++)
    {
		m_updateCallbacks[i](m_deltaTime);
    }

    m_windowManager->NewFrame();
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

	std::shared_ptr<GraphicsBuffer> instanceBuffer = currentObject->GetInstanceBuffer();
    if (instanceBuffer != nullptr)
    {
		m_buffersToDestroy.push_back(instanceBuffer);
    }

    std::shared_ptr<MeshRenderer> meshComponent = currentObject->GetComponent<MeshRenderer>();

    if (meshComponent == nullptr)
    {
        return removalSuccessful;
    }

	std::shared_ptr<GraphicsBuffer> vertexBuffer = meshComponent->GetVertexBuffer();
    if (vertexBuffer != nullptr)
    {
		m_buffersToDestroy.push_back(vertexBuffer);
    }

	std::shared_ptr<GraphicsBuffer> indexBuffer = meshComponent->GetIndexBuffer();
    if (indexBuffer != nullptr)
	{
        m_buffersToDestroy.push_back(indexBuffer);
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

	std::shared_ptr<GraphicsBuffer> oldVertexBuffer = meshComponent->GetVertexBuffer();
    if (oldVertexBuffer != nullptr)
	{
		m_buffersToDestroy.push_back(oldVertexBuffer);
	}

	std::shared_ptr<GraphicsBuffer> oldIndexBuffer = meshComponent->GetIndexBuffer();
	if (oldIndexBuffer != nullptr)
	{
		m_buffersToDestroy.push_back(oldIndexBuffer);
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

void Scene::Cleanup()
{
    for (auto it = m_objects.begin(); it != m_objects.end(); it++)
    {
        std::shared_ptr<GraphicsBuffer> instanceBuffer = it->second->GetInstanceBuffer();

        if (instanceBuffer != nullptr)
        {
            instanceBuffer->DestroyBuffer();
        }

        std::shared_ptr<MeshRenderer> meshComponent = it->second->GetComponent<MeshRenderer>();
        if (meshComponent != nullptr)
        {
            std::shared_ptr<GraphicsBuffer> vertexBuffer = meshComponent->GetVertexBuffer();
            if (vertexBuffer != nullptr)
            {
                vertexBuffer->DestroyBuffer();
            }

            std::shared_ptr<GraphicsBuffer> indexBuffer = meshComponent->GetIndexBuffer();
            if (indexBuffer != nullptr)
            {
                indexBuffer->DestroyBuffer();
            }
        }
    }

    for (size_t i = 0; i < m_buffersToDestroy.size(); i++)
    {
        if (m_buffersToDestroy[i] != nullptr)
        {
            m_buffersToDestroy[i]->DestroyBuffer();
        }
    }
}