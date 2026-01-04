#pragma once

#include "source/Objects/ObjectComponent.h"
#include "source/Vulkan Interface/VulkanCommonFunctions.h"
#include "source/Vulkan Interface/GraphicsBuffer.h"

class UIMeshRenderer : public ObjectComponent {
public:
	UIMeshRenderer() { };
	UIMeshRenderer(std::vector<VulkanCommonFunctions::UIVertex> vertices) { m_vertices = vertices; }
	UIMeshRenderer(std::vector<VulkanCommonFunctions::UIVertex> vertices, std::vector<uint16_t> indices) { m_vertices = vertices; m_indices = indices; m_useIndices = true; }

	virtual const std::vector<VulkanCommonFunctions::UIVertex>& GetVertices() { return m_vertices; }
	void SetVertices(std::vector<VulkanCommonFunctions::UIVertex> vertices);
	size_t GetVertexBufferSize() { return m_vertexBufferSize; }

	virtual const std::vector<uint16_t>& GetIndices() { return m_indices; }
	void SetIndices(std::vector<uint16_t> indices);
	size_t GetIndexBufferSize() { return m_indexBufferSize; }

	void SetDirtyData(bool dirty) { m_meshDataDirty = dirty; }
	bool IsMeshDataDirty() { return m_meshDataDirty; }

	void SetVertexBuffer(std::shared_ptr<GraphicsBuffer> vertexBuffer) { m_vertexBuffer = vertexBuffer; }
	std::shared_ptr<GraphicsBuffer> GetVertexBuffer() { return m_vertexBuffer; }

	void SetIndexBuffer(std::shared_ptr<GraphicsBuffer> indexBuffer) { m_indexBuffer = indexBuffer; }
	std::shared_ptr<GraphicsBuffer> GetIndexBuffer() { return m_indexBuffer; }

protected:
	std::vector<VulkanCommonFunctions::UIVertex> m_vertices;
	std::vector<uint16_t> m_indices;

	std::shared_ptr<GraphicsBuffer> m_vertexBuffer = nullptr;
	std::shared_ptr<GraphicsBuffer> m_indexBuffer = nullptr;

	size_t m_indexBufferSize = 0;
	size_t m_vertexBufferSize = 0;

	bool m_useIndices = false;
	bool m_meshDataDirty = false;
};