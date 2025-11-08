#include "RenderObject.h"
#include "Transform.h"
#include "Scene.h"

RenderObject::RenderObject(std::shared_ptr<WindowManager> windowManager)
{
	m_windowManager = windowManager;
}

VulkanCommonFunctions::InstanceInfo RenderObject::GetInstanceInfo()
{
	VulkanCommonFunctions::InstanceInfo result {};

	std::shared_ptr<Transform> transform = GetComponent<Transform>();

	if (transform == nullptr)
	{
		return result;
	}

	std::shared_ptr<MeshRenderer> meshRenderer = GetComponent<MeshRenderer>();

	if (meshRenderer == nullptr)
	{
		return result;
	}

	result.modelMatrix = glm::mat4(1.0f);
	result.modelMatrix = glm::translate(result.modelMatrix, transform->GetPosition());

	result.modelMatrix = glm::scale(result.modelMatrix, transform->GetScale());

	glm::vec3 rotation = transform->GetRotation();

	result.modelMatrix = glm::rotate(result.modelMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	result.modelMatrix = glm::rotate(result.modelMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	result.modelMatrix = glm::rotate(result.modelMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));


	//need to transpose the matrix because hlsl expects column major matrices
	result.modelMatrix = glm::transpose(result.modelMatrix);

	result.modelMatrixInverse = glm::inverse(result.modelMatrix);

	result.scale = transform->GetScale();

	result.ambient = meshRenderer->GetColor();
	result.diffuse = meshRenderer->GetColor();
	result.specular = glm::vec3(0.5f, 0.5f, 0.5f);
	result.shininess = 32.0f;

	result.lit = (meshRenderer->GetLit()) ? 1 : 0;

	result.textured = (meshRenderer->GetTextured()) ? 1 : 0;

	result.isBillboarded = (meshRenderer->IsBillboarded()) ? 1 : 0;

	return result;
}

std::shared_ptr<GraphicsBuffer> RenderObject::GetInstanceBuffer()
{
	if (m_instanceBuffer == nullptr)
	{
		return nullptr;
	}

	VulkanCommonFunctions::InstanceInfo info = GetInstanceInfo();
	std::array<VulkanCommonFunctions::InstanceInfo, 1> infoArray = { info };

	m_instanceBuffer->LoadData(infoArray.data(), sizeof(VulkanCommonFunctions::InstanceInfo));

	return m_instanceBuffer;
}