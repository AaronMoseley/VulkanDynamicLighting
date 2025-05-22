#pragma once

#include "RenderObject.h"

std::vector<Vertex> tetrahedronVertices = {
    //front
    {{-sqrt(2.0f / 9.0f), -sqrt(2.0f / 3.0f), -1.0f / 3.0f}, {0.0f, 0.0f, -1.0f}},
    {{sqrt(8.0f / 9.0f), 0.0f, -1.0f / 3.0f}, {0.0f, 0.0f, -1.0f}},
    {{-sqrt(2.0f / 9.0f), sqrt(2.0f / 3.0f), -1.0f / 3.0f}, {0.0f, 0.0f, -1.0f}},

    //left
    {{-sqrt(2.0f / 9.0f), -sqrt(2.0f / 3.0f), -1.0f / 3.0f}, {-0.31426968, 0.0f, 0.11111f}},
    {{-sqrt(2.0f / 9.0f), sqrt(2.0f / 3.0f), -1.0f / 3.0f}, {-0.31426968, 0.0f, 0.11111f}},
    {{0.0f, 0.0f, 1.0f}, {-0.31426968, 0.0f, 0.11111f}},

    //top right
    {{-sqrt(2.0f / 9.0f), -sqrt(2.0f / 3.0f), -1.0f / 3.0f}, {0.15713484f, -0.2721655f, 0.11111f}},
    {{0.0f, 0.0f, 1.0f}, {0.15713484f, -0.2721655f, 0.11111f}},
    {{sqrt(8.0f / 9.0f), 0.0f, -1.0f / 3.0f}, {0.15713484f, -0.2721655f, 0.11111f}},

    //bottom right
    {{sqrt(8.0f / 9.0f), 0.0f, -1.0f / 3.0f}, {0.15713484f, 0.2721655f, 0.11111f}},
    {{0.0f, 0.0f, 1.0f}, {0.15713484f, 0.2721655f, 0.11111f}},
    {{-sqrt(2.0f / 9.0f), sqrt(2.0f / 3.0f), -1.0f / 3.0f}, {0.15713484f, 0.2721655f, 0.11111f}}
};

std::vector<uint16_t> tetrahedronIndices = {
    //front
    0, 1, 2,

    //left
    3, 4, 5,

    //top right
    6, 7, 8,

    //bottom right
    9, 10, 11
};

class Tetrahedron : public RenderObject {
public:
    Tetrahedron() : RenderObject()
    {
        m_vertices = tetrahedronVertices;
        m_indices = tetrahedronIndices;
        m_useIndices = true;
        m_name = "Tetrahedron";
    }

    Tetrahedron(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, glm::vec3 color) : RenderObject(position, rotation, scale, color)
    {
        m_vertices = tetrahedronVertices;
        m_indices = tetrahedronIndices;
        m_useIndices = true;
        m_name = "Tetrahedron";
    }
};