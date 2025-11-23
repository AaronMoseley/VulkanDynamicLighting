#include "ObjectComponent.h"
#include "RenderObject.h"

Scene* ObjectComponent::GetScene()
{
	if (m_owner == nullptr)
	{
		return nullptr;
	}

	return m_owner->GetSceneManager();
}

std::shared_ptr<WindowManager> ObjectComponent::GetWindowManager()
{
	return GetOwner()->GetWindowManager();
}