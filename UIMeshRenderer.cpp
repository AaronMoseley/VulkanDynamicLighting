#include "UIMeshRenderer.h"

void UIMeshRenderer::SetVertices(std::vector<VulkanCommonFunctions::UIVertex> vertices)
{
	m_vertexBufferSize = vertices.size();
	m_vertices = vertices;

	SetDirtyData(true);
}

void UIMeshRenderer::SetIndices(std::vector<uint16_t> indices)
{
	m_indexBufferSize = indices.size();
	m_indices = indices;
	m_useIndices = true;

	SetDirtyData(true);
}