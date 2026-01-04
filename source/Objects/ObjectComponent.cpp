#include "ObjectComponent.h"
#include "source/Objects/RenderObject.h"

Scene* ObjectComponent::GetScene()
{
	if (m_owner == nullptr)
	{
		return nullptr;
	}

	return m_owner->GetSceneManager();
}

WindowManager* ObjectComponent::GetWindowManager()
{
	return GetOwner()->GetWindowManager();
}