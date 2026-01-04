#include "RenderObject.h"
#include "source/Management/Scene.h"
#include "source/Management/WindowManager.h"

RenderObject::RenderObject()
{

}

VulkanCommonFunctions::InstanceInfo RenderObject::GetInstanceInfo(const std::vector<std::string>& textureFilePaths)
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
	result.modelMatrix = glm::translate(result.modelMatrix, transform->GetWorldPosition());

	result.modelMatrix = glm::scale(result.modelMatrix, transform->GetWorldScale());

	glm::vec3 rotation = transform->GetWorldRotation();

	result.modelMatrix = glm::rotate(result.modelMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	result.modelMatrix = glm::rotate(result.modelMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	result.modelMatrix = glm::rotate(result.modelMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

	//need to transpose the matrix because hlsl expects column major matrices
	result.modelMatrix = glm::transpose(result.modelMatrix);

	result.modelMatrixInverse = glm::inverse(result.modelMatrix);

	result.scale = transform->GetWorldScale();

	result.ambient = meshRenderer->GetColor();
	result.diffuse = meshRenderer->GetColor();
	result.specular = glm::vec3(0.5f, 0.5f, 0.5f);
	result.shininess = std::pow(2.0f, meshRenderer->GetShininess());

	result.opacity = meshRenderer->GetOpacity();

	result.lit = (meshRenderer->GetLit()) ? 1 : 0;

	result.textured = (meshRenderer->GetTextured()) ? 1 : 0;

	result.isBillboarded = (meshRenderer->IsBillboarded()) ? 1 : 0;

	auto iterator = std::find(textureFilePaths.begin(), textureFilePaths.end(), meshRenderer->GetTexturePath());

	result.textureIndex = std::distance(textureFilePaths.begin(), iterator);
	if (result.textureIndex > textureFilePaths.size())
	{
		result.textureIndex = 0;
	}

	return result;
}

VulkanCommonFunctions::UIInstanceInfo RenderObject::GetUIInstanceInfo(const std::vector<std::string>& textureFilePaths)
{
	VulkanCommonFunctions::UIInstanceInfo result {};
	std::shared_ptr<Transform> transform = GetComponent<Transform>();
	if (transform == nullptr)
	{
		return result;
	}
	std::shared_ptr<UIImage> imageComponent = GetComponent<UIImage>();
	if (imageComponent == nullptr)
	{
		return result;
	}
	result.objectPosition = transform->GetWorldPosition();
	result.scale = transform->GetWorldScale();
	result.color = imageComponent->GetColor();
	result.opacity = imageComponent->GetOpacity();
	result.textured = (imageComponent->GetTextured()) ? 1 : 0;
	result.isTextCharacter = 0;

	auto iterator = std::find(textureFilePaths.begin(), textureFilePaths.end(), imageComponent->GetTexturePath());

	result.textureIndex = std::distance(textureFilePaths.begin(), iterator);
	if (result.textureIndex > textureFilePaths.size())
	{
		result.textureIndex = 0;
	}

	return result;
}

std::shared_ptr<GraphicsBuffer> RenderObject::GetInstanceBuffer(const std::vector<std::string>& textureFilePaths)
{
	if (m_instanceBuffer == nullptr)
	{
		return nullptr;
	}

	VulkanCommonFunctions::InstanceInfo info = GetInstanceInfo(textureFilePaths);

	std::array<VulkanCommonFunctions::InstanceInfo, 1> infoArray = { info };

	m_instanceBuffer->LoadData(infoArray.data(), sizeof(VulkanCommonFunctions::InstanceInfo));

	return m_instanceBuffer;
}

std::shared_ptr<GraphicsBuffer> RenderObject::GetUIInstanceBuffer(const std::vector<std::string>& textureFilePaths)
{
	if (m_instanceBuffer == nullptr)
	{
		return nullptr;
	}

	VulkanCommonFunctions::UIInstanceInfo info = GetUIInstanceInfo(textureFilePaths);
	std::array<VulkanCommonFunctions::UIInstanceInfo, 1> infoArray = { info };

	m_instanceBuffer->LoadData(infoArray.data(), sizeof(VulkanCommonFunctions::UIInstanceInfo));

	return m_instanceBuffer;
}