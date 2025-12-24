#include "Scene.h"
#include "RenderObject.h"
#include "Cube.h"

Scene::Scene(WindowManager* windowManager, std::shared_ptr<VulkanInterface> vulkanInterface)
{
	m_windowManager = windowManager;
    m_vulkanInterface = vulkanInterface;
	m_fontManager = std::make_shared<FontManager>();
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

        UpdateMeshData(it->second);
    }

    for (auto it = m_uiObjects.begin(); it != m_uiObjects.end(); it++)
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

        UpdateUIData(it->second);
    }

    for (size_t i = 0; i < m_updateCallbacks.size(); i++)
    {
		m_updateCallbacks[i](m_deltaTime);
    }

    m_windowManager->NewFrame();
}

void Scene::UpdateUIData(std::shared_ptr<RenderObject> currentObject)
{
    if (currentObject == nullptr)
    {
        return;
    }

    std::shared_ptr<UIMeshRenderer> meshComponent = currentObject->GetComponent<UIMeshRenderer>();

    if(meshComponent == nullptr)
    {
        return;
    }

    if (meshComponent->IsMeshDataDirty())
    {
        FinalizeUIMesh(currentObject);
        meshComponent->SetDirtyData(false);
    }

    std::shared_ptr<UIImage> imageComponent = currentObject->GetComponent<UIImage>();

    if (imageComponent == nullptr)
    {
        return;
    }

    if (imageComponent->IsTextureDataDirty())
    {
        UpdateTexture(imageComponent->GetTexturePath());
        imageComponent->SetTextureDataDirty(false);
    }
}

void Scene::UpdateMeshData(std::shared_ptr<RenderObject> currentObject)
{
    if (currentObject == nullptr)
    {
        return;
    }

    std::shared_ptr<MeshRenderer> meshComponent = currentObject->GetComponent<MeshRenderer>();

    if (meshComponent == nullptr)
    {
        return;
    }

    if (meshComponent->GetMeshName() != MeshRenderer::kCustomMeshName)
    {
        return;
    }

    if (meshComponent->IsMeshDataDirty())
    {
        FinalizeMesh(currentObject);
        meshComponent->SetDirtyData(false);
    }

    if (meshComponent->IsTextureDataDirty())
    {
        UpdateTexture(meshComponent->GetTexturePath());
        meshComponent->SetTextureDataDirty(false);
    }
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
    newObject->SetWindowManager(m_windowManager);

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

VulkanCommonFunctions::ObjectHandle Scene::AddUIObject(std::shared_ptr <RenderObject> newObject)
{
    if (m_uiObjects.size() >= VulkanCommonFunctions::MAX_OBJECTS)
    {
        return VulkanCommonFunctions::INVALID_OBJECT_HANDLE;
    }

    m_currentUIObjectHandle++;

    m_uiObjects[m_currentUIObjectHandle] = newObject;
    newObject->SetSceneManager(this);
    newObject->SetWindowManager(m_windowManager);

    std::shared_ptr<UIImage> imageComponent = newObject->GetComponent<UIImage>();

    if (imageComponent == nullptr)
    {
        return m_currentUIObjectHandle;
    }

    if (imageComponent->GetTextured())
    {
        UpdateTexture(imageComponent->GetTexturePath());
    }

    return m_currentUIObjectHandle;
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

    std::shared_ptr<GraphicsBuffer> instanceBuffer = currentObject->GetInstanceBuffer({});
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

bool Scene::RemoveUIObject(VulkanCommonFunctions::ObjectHandle objectToRemove)
{
    std::shared_ptr<RenderObject> currentObject = GetUIRenderObject(objectToRemove);

    if (currentObject == nullptr)
    {
        return false;
    }

    currentObject->SetSceneManager(nullptr);

    bool removalSuccessful = true;

    removalSuccessful = m_uiObjects.erase(objectToRemove);

    std::shared_ptr<GraphicsBuffer> instanceBuffer = currentObject->GetInstanceBuffer({});
    if (instanceBuffer != nullptr)
    {
        m_buffersToDestroy.push_back(instanceBuffer);
    }

    std::shared_ptr<UIMeshRenderer> meshComponent = currentObject->GetComponent<UIMeshRenderer>();

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

std::shared_ptr<RenderObject> Scene::GetUIRenderObject(VulkanCommonFunctions::ObjectHandle handle)
{
    if (handle == VulkanCommonFunctions::INVALID_OBJECT_HANDLE)
    {
        return nullptr;
    }
    if (!m_uiObjects.contains(handle))
    {
        return nullptr;
    }
    return m_uiObjects[handle];
}

void Scene::FinalizeUIMesh(std::shared_ptr<RenderObject> updatedObject)
{
    if (updatedObject == nullptr)
    {
        return;
    }

    std::shared_ptr<UIMeshRenderer> meshComponent = updatedObject->GetComponent<UIMeshRenderer>();
    if (meshComponent == nullptr)
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

    std::shared_ptr<GraphicsBuffer> vertexBuffer = m_vulkanInterface->CreateUIVertexBuffer(meshComponent);
    std::shared_ptr<GraphicsBuffer> indexBuffer = m_vulkanInterface->CreateUIIndexBuffer(meshComponent);

    meshComponent->SetVertexBuffer(vertexBuffer);
    meshComponent->SetIndexBuffer(indexBuffer);

    if (updatedObject->GetInstanceBuffer({}) == nullptr)
    {
        GenerateInstanceBuffer(updatedObject);
    }
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

    if (updatedObject->GetInstanceBuffer({}) == nullptr)
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

VulkanCommonFunctions::ObjectHandle Scene::GetObjectByTag(std::string tag)
{
    for (auto it = m_objects.begin(); it != m_objects.end(); it++)
    {
        std::shared_ptr<RenderObject> currentObject = it->second;
        
        if (currentObject->GetTag() == tag)
        {
            return it->first;
        }
    }
    return VulkanCommonFunctions::INVALID_OBJECT_HANDLE;
}

void Scene::AddFont(std::string atlasFilePath, std::string descriptionFilePath)
{
	m_fontManager->AddFont(atlasFilePath, descriptionFilePath);
    m_vulkanInterface->UpdateTextureResources(atlasFilePath);
}

void Scene::Cleanup()
{
    for (auto it = m_objects.begin(); it != m_objects.end(); it++)
    {
        std::shared_ptr<GraphicsBuffer> instanceBuffer = it->second->GetInstanceBuffer({});

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

    for (auto it = m_uiObjects.begin(); it != m_uiObjects.end(); it++)
    {
        std::shared_ptr<GraphicsBuffer> instanceBuffer = it->second->GetInstanceBuffer({});

        if (instanceBuffer != nullptr)
        {
            instanceBuffer->DestroyBuffer();
        }

        std::shared_ptr<Text> textComponent = it->second->GetComponent<Text>();
        if (textComponent != nullptr)
        {
            std::shared_ptr<GraphicsBuffer> textInstanceBuffer = textComponent->GetInstanceBuffer();
            if (textInstanceBuffer != nullptr)
            {
                textInstanceBuffer->DestroyBuffer();
            }
        }

        std::shared_ptr<UIMeshRenderer> uiMeshComponent = it->second->GetComponent<UIMeshRenderer>();
        if (uiMeshComponent != nullptr)
        {
            std::shared_ptr<GraphicsBuffer> vertexBuffer = uiMeshComponent->GetVertexBuffer();
            if (vertexBuffer != nullptr)
            {
                vertexBuffer->DestroyBuffer();
            }

            std::shared_ptr<GraphicsBuffer> indexBuffer = uiMeshComponent->GetIndexBuffer();
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