#pragma once

#include "MeshRenderer.h"
#include "VulkanCommonFunctions.h"

class Tetrahedron : public MeshRenderer {
public:
    Tetrahedron() : MeshRenderer()
    {
        m_meshName = "Tetrahedron";
    }

	const std::vector<VulkanCommonFunctions::Vertex>& GetVertices() override { return tetrahedronVertices; };
	const std::vector<uint16_t>& GetIndices() override { return tetrahedronIndices; };

private:
    using MeshRenderer::SetIndices;
    using MeshRenderer::SetVertices;

    const std::vector<VulkanCommonFunctions::Vertex> tetrahedronVertices = {
        //front
        {{-sqrt(2.0f / 9.0f), -sqrt(2.0f / 3.0f), -1.0f / 3.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
        {{sqrt(8.0f / 9.0f), 0.0f, -1.0f / 3.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.5f}},
        {{-sqrt(2.0f / 9.0f), sqrt(2.0f / 3.0f), -1.0f / 3.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},

        //left
        {{-sqrt(2.0f / 9.0f), -sqrt(2.0f / 3.0f), -1.0f / 3.0f}, {-0.31426968, 0.0f, 0.11111f}, {0.0f, 0.0f}},
        {{-sqrt(2.0f / 9.0f), sqrt(2.0f / 3.0f), -1.0f / 3.0f}, {-0.31426968, 0.0f, 0.11111f}, {1.0f, 0.5f}},
        {{0.0f, 0.0f, 1.0f}, {-0.31426968, 0.0f, 0.11111f}, {0.0f, 1.0f}},

        //top right
        {{-sqrt(2.0f / 9.0f), -sqrt(2.0f / 3.0f), -1.0f / 3.0f}, {0.15713484f, -0.2721655f, 0.11111f}, {0.0f, 0.0f}},
        {{0.0f, 0.0f, 1.0f}, {0.15713484f, -0.2721655f, 0.11111f}, {1.0f, 0.5f}},
        {{sqrt(8.0f / 9.0f), 0.0f, -1.0f / 3.0f}, {0.15713484f, -0.2721655f, 0.11111f}, {0.0f, 1.0f}},

        //bottom right
        {{sqrt(8.0f / 9.0f), 0.0f, -1.0f / 3.0f}, {0.15713484f, 0.2721655f, 0.11111f}, {0.0f, 0.0f}},
        {{0.0f, 0.0f, 1.0f}, {0.15713484f, 0.2721655f, 0.11111f}, {1.0f, 0.5f}},
        {{-sqrt(2.0f / 9.0f), sqrt(2.0f / 3.0f), -1.0f / 3.0f}, {0.15713484f, 0.2721655f, 0.11111f}, {0.0f, 1.0f}}
    };

    const std::vector<uint16_t> tetrahedronIndices = {
        //front
        0, 1, 2,

        //left
        3, 4, 5,

        //top right
        6, 7, 8,

        //bottom right
        9, 10, 11
    };
};