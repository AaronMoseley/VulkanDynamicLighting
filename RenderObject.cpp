#include "RenderObject.h"

RenderObject::RenderObject()
{
	m_position = glm::vec3(0.0f, 0.0f, 0.0f);
	m_rotation = glm::vec3(0.0f, 0.0f, 0.0f);
	m_scale = glm::vec3(1.0f, 1.0f, 1.0f);
	m_color = glm::vec3(0.0f, 0.0f, 0.0f);
}

RenderObject::RenderObject(glm::vec3 position)
{
	m_position = position;
	m_rotation = glm::vec3(0.0f, 0.0f, 0.0f);
	m_scale = glm::vec3(1.0f, 1.0f, 1.0f);
	m_color = glm::vec3(0.0f, 0.0f, 0.0f);
}

RenderObject::RenderObject(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
{
	m_position = position;
	m_rotation = rotation;
	m_scale = scale;
	m_color = glm::vec3(0.0f, 0.0f, 0.0f);
}

RenderObject::RenderObject(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, glm::vec3 color)
{
	m_position = position;
	m_rotation = rotation;
	m_scale = scale;
	m_color = color;
}

void RenderObject::SetRotation(glm::vec3 rotation)
{
	m_rotation = rotation;
}

void RenderObject::SetPosition(glm::vec3 position)
{
	m_position = position;
}

void RenderObject::Rotate(glm::vec3 amountToRotate) 
{
	m_rotation += amountToRotate;
}

void RenderObject::Move(glm::vec3 amountToMove)
{
	m_position += amountToMove;
}

glm::vec3 RenderObject::getPosition()
{
	return m_position;
}

glm::vec3 RenderObject::getRotation()
{
	return m_rotation;
}

std::vector<Vertex> RenderObject::getVertices()
{
	return m_vertices;
}

std::vector<uint16_t> RenderObject::getIndices()
{
	return m_indices;
}

std::string RenderObject::getName()
{
	return m_name;
}

bool RenderObject::isIndexed()
{
	return m_useIndices;
}

bool RenderObject::getLit()
{
	return m_lit;
}

void RenderObject::setLit(bool lit)
{
	m_lit = lit;
}

uint32_t RenderObject::getVerticesSize()
{
	return static_cast<uint32_t>(m_vertices.size());
}

uint32_t RenderObject::getIndicesSize()
{
	return static_cast<uint32_t>(m_indices.size());
}

InstanceInfo RenderObject::getInstanceInfo()
{
	InstanceInfo result;
	
	result.modelMatrix = glm::mat4(1.0f);
	result.modelMatrix = glm::translate(result.modelMatrix, m_position);

	result.modelMatrix = glm::scale(result.modelMatrix, m_scale);

	result.modelMatrix = glm::rotate(result.modelMatrix, glm::radians(m_rotation.r), glm::vec3(1.0f, 0.0f, 0.0f));
	result.modelMatrix = glm::rotate(result.modelMatrix, glm::radians(m_rotation.g), glm::vec3(0.0f, 1.0f, 0.0f));
	result.modelMatrix = glm::rotate(result.modelMatrix, glm::radians(m_rotation.b), glm::vec3(0.0f, 0.0f, 1.0f));

	result.ambient = m_color;
	result.diffuse = m_color;
	result.specular = glm::vec3(0.5f, 0.5f, 0.5f);
	result.shininess = 32.0f;

	result.lit = (m_lit) ? 1 : 0;

	return result;
}