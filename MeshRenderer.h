#pragma once

#include "ObjectComponent.h"
#include "VulkanCommonFunctions.h"
#include "GraphicsBuffer.h"

#include <glm/glm.hpp>

#include <vector>

class MeshRenderer : public ObjectComponent {
public:
	inline const static std::string kCustomMeshName = "CustomMesh";

	MeshRenderer() {};
	MeshRenderer(std::vector<VulkanCommonFunctions::Vertex> vertices, std::string name) { m_vertices = vertices; m_meshName = name; }
	MeshRenderer(std::vector<VulkanCommonFunctions::Vertex> vertices, std::vector<uint16_t> indices, std::string name) { m_vertices = vertices; m_indices = indices; m_useIndices = true; m_meshName = name; }

	virtual std::vector<VulkanCommonFunctions::Vertex>& GetVertices() { return m_vertices; }
	void SetVertices(std::vector<VulkanCommonFunctions::Vertex> vertices);
	size_t GetVertexBufferSize() { return m_vertexBufferSize; }

	virtual std::vector<uint16_t>& GetIndices() { return m_indices; }
	void SetIndices(std::vector<uint16_t> indices);
	size_t GetIndexBufferSize() { return m_indexBufferSize; }

	glm::vec3 GetColor() { return m_color; }
	void SetColor(glm::vec3 color) { m_color = color; }

	bool IsIndexed() { return m_useIndices; }
	void SetIndexed(bool useIndices) { m_useIndices = useIndices; }

	bool GetLit() { return m_lit; }
	void SetLit(bool lit) { m_lit = lit; }

	bool GetTextured() { return m_textured; }
	uint32_t GetTextureIndex() { return m_textureIndex; }

	void SetTexture(uint32_t textureIndex) { m_textureIndex = textureIndex; m_textured = true; };
	void SetTextured(bool textured) { m_textured = textured; }

	void SetVertexBuffer(std::shared_ptr<GraphicsBuffer> vertexBuffer) { m_vertexBuffer = vertexBuffer; }
	std::shared_ptr<GraphicsBuffer> GetVertexBuffer() { return m_vertexBuffer; }

	void SetIndexBuffer(std::shared_ptr<GraphicsBuffer> indexBuffer) { m_indexBuffer = indexBuffer; }
	std::shared_ptr<GraphicsBuffer> GetIndexBuffer() { return m_indexBuffer; }

	std::string GetMeshName() { return m_meshName; }

	void SetDirtyData(bool dirty) { m_dataDirty = dirty; }
	bool IsDataDirty() { return m_dataDirty; }

protected:
	std::string m_meshName = kCustomMeshName;

	glm::vec3 m_color = glm::vec3(1.0f);
	std::vector<VulkanCommonFunctions::Vertex> m_vertices;
	std::vector<uint16_t> m_indices;

	std::shared_ptr<GraphicsBuffer> m_vertexBuffer = nullptr;
	std::shared_ptr<GraphicsBuffer> m_indexBuffer = nullptr;

	size_t m_indexBufferSize = 0;
	size_t m_vertexBufferSize = 0;

	bool m_useIndices = false;
	bool m_lit = true;

	bool m_dataDirty = false;

	bool m_textured = false;
	uint32_t m_textureIndex = 0;
};