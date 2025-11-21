#include "DemoBehavior.h"
#include "Scene.h"

void DemoBehavior::Start()
{
    std::srand(std::time(0));

    std::shared_ptr<RenderObject> lightCube = std::make_shared<RenderObject>(GetWindowManager());

    std::shared_ptr<Transform> lightTransform = lightCube->AddComponent<Transform>();
    lightTransform->SetPosition(glm::vec3(lightOrbitRadius, lightOrbitRadius, lightOrbitRadius));
    lightTransform->SetRotation(glm::vec3(0.0f));
    lightTransform->SetScale(glm::vec3(0.25f));
    std::shared_ptr<Cube> lightMesh = lightCube->AddComponent<Cube>();

    //used for testing dynamic mesh updates
	//std::shared_ptr<MeshRenderer> lightMesh = lightCube->AddComponent<MeshRenderer>();
    //lightMesh->SetVertices(squareVertices);
    //lightMesh->SetIndices(squareIndices);

    lightMesh->SetLit(false);
    lightMesh->SetColor(glm::vec3(1.0f, 1.0f, 1.0f));
    lightCube->AddComponent<LightSource>();

    lightObjectHandle = GetScene()->AddObject(lightCube);
    objectHandles.insert(lightObjectHandle);

    glm::vec3 color = glm::vec3(0.6588f, 0.2235f, 0.0392f);
    //glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);

    for (int i = 0; i < objectPositions.size(); i++)
    {
        std::shared_ptr<RenderObject> newObject = std::make_shared<RenderObject>(GetWindowManager());

        std::shared_ptr<Transform> newObjectTransform = newObject->AddComponent<Transform>();
        newObjectTransform->SetPosition(objectPositions[i]);
        newObjectTransform->SetRotation(glm::vec3(((double)rand() / (RAND_MAX)) * 360.0f, ((double)rand() / (RAND_MAX)) * 360.0f, ((double)rand() / (RAND_MAX)) * 360.0f));
        newObjectTransform->SetScale(glm::vec3(0.5f));

        newObjectTransform->SetParent(lightTransform);

        if ((double)rand() / (RAND_MAX) >= 0.5f)
        {
            newObject->AddComponent<Cube>();
        }
        else {
            newObject->AddComponent<Tetrahedron>();
        }

        std::shared_ptr<MeshRenderer> currentMesh = newObject->GetComponent<MeshRenderer>();

        //currentMesh->SetOpacity(((double)rand() / (RAND_MAX)));

        if ((double)rand() / (RAND_MAX) >= 0.5f)
        {
            currentMesh->SetIsBillboarded(true);
            newObject->GetComponent<Transform>()->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));
        }

        currentMesh->SetColor(color);

        //currentMesh->SetColor({ 1.0f, 1.0f, 1.0f });

        if (GetScene()->GetObjectCount() % 3 == 0)
        {
            currentMesh->SetTexture("textures\\SandTexture.png");
        }
        else if (GetScene()->GetObjectCount() % 3 == 1)
        {
            currentMesh->SetTexture("textures\\OtherTexture.png");
        }
        else {
            currentMesh->SetTextured(false);
        }

        currentMesh->SetLit(false);

        VulkanCommonFunctions::ObjectHandle newObjectHandle = GetScene()->AddObject(newObject);
        objectHandles.insert(newObjectHandle);
    }
}

void DemoBehavior::Update(float deltaTime)
{
    currentTime += deltaTime;

    std::shared_ptr<RenderObject> lightObject = GetScene()->GetRenderObject(lightObjectHandle);
    if (lightObject != nullptr)
    {
        //lightObject->GetComponent<Transform>()->SetPosition(glm::vec3(lightOrbitRadius * cos(currentTime), lightOrbitRadius * sin(currentTime), lightOrbitRadius * cos(currentTime)));
        lightObject->GetComponent<Transform>()->Rotate(glm::vec3(0.0f, deltaTime * 10.0f, 0.0f));
        //auto temp = lightObject->GetComponent<Transform>()->GetWorldRotation();
        //int temp1 = 0;
    }

	//used for testing dynamic mesh updates
    /*if (GetWindowManager()->KeyPressedThisFrame(Qt::Key::Key_T))
    {
        std::vector<uint16_t> triangleIndices = {
        0, 1, 2
        };

        lightObject->GetComponent<MeshRenderer>()->SetIndices(triangleIndices);
    }*/

    for (auto it = objectHandles.begin(); it != objectHandles.end(); it++)
    {
        std::shared_ptr<RenderObject> currentObject = GetScene()->GetRenderObject(*it);

        if (currentObject == nullptr)
        {
            continue;
        }

        currentObject->GetComponent<Transform>()->Rotate(glm::vec3(16.0f * deltaTime));
    }

    if (GetWindowManager()->KeyPressed(Qt::Key::Key_R))
    {
        float positionRange = 100.0f;

        std::shared_ptr<RenderObject> newObject = std::make_shared<RenderObject>(GetWindowManager());

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

        currentMesh->SetOpacity((double)rand() / (RAND_MAX));

        if ((double)rand() / (RAND_MAX) >= 0.5f)
        {
            currentMesh->SetIsBillboarded(true);
            newObject->GetComponent<Transform>()->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));
        }

        currentMesh->SetColor(glm::vec3(0.9f));

        if (GetScene()->GetObjectCount() % 3 == 0)
        {
            currentMesh->SetTexture("textures\\SandTexture.png");
        }
        else if (GetScene()->GetObjectCount() % 3 == 1)
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

        VulkanCommonFunctions::ObjectHandle newObjectHandle = GetScene()->AddObject(newObject);
        objectHandles.insert(newObjectHandle);
    }

    if (GetWindowManager()->KeyPressed(Qt::Key::Key_E) && GetScene()->GetObjectCount() > 1)
    {
        VulkanCommonFunctions::ObjectHandle removeObjectHandle = *objectHandles.rbegin();
        bool correctlyRemoved = GetScene()->RemoveObject(removeObjectHandle);
        objectHandles.erase(removeObjectHandle);

        if (!correctlyRemoved)
        {
            std::cout << "Error removing object, handle: " << removeObjectHandle << std::endl;
        }
    }

    if (GetWindowManager()->KeyPressedThisFrame(Qt::Key::Key_Escape))
    {
        GetWindowManager()->Shutdown();
    }
}