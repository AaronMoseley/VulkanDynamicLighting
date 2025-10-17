#pragma once

#include "ObjectComponent.h"
#include "VulkanCommonFunctions.h"
#include "Factory.h"

#include <glm/glm.hpp>

#include <vector>

class MeshRenderer : public ObjectComponent {
public:
	MeshRenderer() {};
	MeshRenderer(std::vector<VulkanCommonFunctions::Vertex> vertices, std::string name) { m_vertices = vertices; m_meshName = name; }
	MeshRenderer(std::vector<VulkanCommonFunctions::Vertex> vertices, std::vector<uint16_t> indices, std::string name) { m_vertices = vertices; m_indices = indices; m_useIndices = true; m_meshName = name; }

	virtual std::vector<VulkanCommonFunctions::Vertex> GetVertices() { return m_vertices; }
	void SetVertices(std::vector<VulkanCommonFunctions::Vertex> vertices) { m_vertices = vertices; }

	virtual std::vector<uint16_t> GetIndices() { return m_indices; }
	void SetIndices(std::vector<uint16_t> indices) { m_indices = indices; }

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

	std::string GetMeshName() { return m_meshName; }

protected:
	inline static const std::string m_name = "MeshRenderer";
	std::string m_meshName = "CustomMesh";

	glm::vec3 m_color = glm::vec3(1.0f);
	std::vector<VulkanCommonFunctions::Vertex> m_vertices;
	std::vector<uint16_t> m_indices;

	bool m_useIndices = false;
	bool m_lit = true;

	bool m_textured = false;
	uint32_t m_textureIndex = 0;
};