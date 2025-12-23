#pragma once

#include "ObjectComponent.h"
#include "TextureImage.h"
#include "VulkanCommonFunctions.h"

class UIImage : public ObjectComponent {
public:
	UIImage();
	UIImage(std::string imageFilePath);

	void SetOpacity(float opacity) { m_opacity = opacity; }
	float GetOpacity() { return m_opacity; }

	bool GetTextured() { return m_textured; }
	std::string GetTexturePath() { return m_texturePath; }

	void SetTexture(std::string texturePath) {
		m_texturePath = texturePath;
		m_textured = true;
		m_textureDataDirty = true;
		CalculateMeshInfo();
	};
	void SetTextured(bool textured) { m_textured = textured; }

	void CalculateMeshInfo();

	glm::vec3 GetColor() { return m_color; }
	void SetColor(glm::vec3 color) { m_color = color; }

	void SetDirtyData(bool dirty) { m_meshDataDirty = dirty; }
	bool IsMeshDataDirty() { return m_meshDataDirty; }

	void SetTextureDataDirty(bool dirty) { m_textureDataDirty = dirty; }
	bool IsTextureDataDirty() { return m_textureDataDirty; }

	void SetVertexBuffer(std::shared_ptr<GraphicsBuffer> vertexBuffer) { m_vertexBuffer = vertexBuffer; }
	std::shared_ptr<GraphicsBuffer> GetVertexBuffer() { return m_vertexBuffer; }

	void SetIndexBuffer(std::shared_ptr<GraphicsBuffer> indexBuffer) { m_indexBuffer = indexBuffer; }
	std::shared_ptr<GraphicsBuffer> GetIndexBuffer() { return m_indexBuffer; }

	const std::vector<VulkanCommonFunctions::UIVertex>& GetVertices() { 
		return m_vertices; 
	}
	void SetVertices(std::vector<VulkanCommonFunctions::UIVertex> vertices);

	const std::vector<uint16_t>& GetIndices() { return m_indices; }
	void SetIndices(std::vector<uint16_t> indices);
	uint32_t GetIndexBufferSize() { return static_cast<uint32_t>(m_indices.size()); }

private:
	alignas(16) std::vector<VulkanCommonFunctions::UIVertex> m_vertices = {
		//positions              //texture coords
		{ {-1.0f,  1.0f, 0.0f},  {0.0f, 0.0f} }, //top left
		{ { 1.0f,  1.0f, 0.0f},  {1.0f, 0.0f} }, //top right
		{ { 1.0f, -1.0f, 0.0f},  {1.0f, 1.0f} }, //bottom right
		{ {-1.0f, -1.0f, 0.0f},  {0.0f, 1.0f} }  //bottom left
	};

	alignas(16) std::vector<uint16_t> m_indices = { 0, 1, 2, 2, 3, 0 };

	float m_opacity = 1.0f;

	bool m_textured = false;
	std::string m_texturePath = "";

	bool m_meshDataDirty = false;
	bool m_textureDataDirty = false;

	int m_imageWidth = 0;
	int m_imageHeight = 0;

	std::shared_ptr<GraphicsBuffer> m_vertexBuffer = nullptr;
	std::shared_ptr<GraphicsBuffer> m_indexBuffer = nullptr;

	alignas(16) glm::vec3 m_color = glm::vec3(1.0f);
};