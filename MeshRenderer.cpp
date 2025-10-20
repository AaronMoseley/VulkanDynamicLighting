#include "MeshRenderer.h"

void MeshRenderer::SetVertices(std::vector<VulkanCommonFunctions::Vertex> vertices)
{
	if (GetMeshName() != kCustomMeshName)
	{
		return;
	}

	m_vertexBufferSize = vertices.size();
	m_vertices = vertices;

	SetDirtyData(true);
}

void MeshRenderer::SetIndices(std::vector<uint16_t> indices)
{
	if (GetMeshName() != kCustomMeshName)
	{
		return;
	}

	m_indexBufferSize = indices.size();
	m_indices = indices;

	SetDirtyData(true);
}