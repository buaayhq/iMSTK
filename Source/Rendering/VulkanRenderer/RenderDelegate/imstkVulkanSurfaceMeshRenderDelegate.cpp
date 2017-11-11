/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

#include "imstkVulkanSurfaceMeshRenderDelegate.h"

namespace imstk
{
VulkanSurfaceMeshRenderDelegate::VulkanSurfaceMeshRenderDelegate(std::shared_ptr<SurfaceMesh> surfaceMesh, VulkanMemoryManager& memoryManager)
    : m_geometry(surfaceMesh)
{
    m_numVertices = (uint32_t)m_geometry->getNumVertices();
    m_numTriangles = (uint32_t)m_geometry->getNumTriangles();
    m_loadFactor = m_geometry->getLoadFactor();
    m_vertexSize = sizeof(VulkanBasicVertex);

    if (!m_geometry->getRenderMaterial())
    {
        m_geometry->setRenderMaterial(std::make_shared<RenderMaterial>());
    }

    this->initializeData(memoryManager, m_geometry->getRenderMaterial());

    this->updateVertexBuffer();

    this->update();
}

void
VulkanSurfaceMeshRenderDelegate::updateVertexBuffer()
{
    auto vertices = (VulkanBasicVertex *)m_vertexBuffer->mapVertices();

    auto normals = m_geometry->getVertexNormals();
    auto tangents = m_geometry->getVertexTangents();
    const StdVectorOfVectorf* UVs;

    if (m_geometry->getDefaultTCoords() != "")
    {
        UVs = m_geometry->getPointDataArray(m_geometry->getDefaultTCoords());
    }
    else
    {
        UVs = nullptr;
    }

    auto vertexPositions = m_geometry->getVertexPositions(Geometry::DataType::PreTransform);
    for (unsigned i = 0; i < m_geometry->getNumVertices(); i++)
    {
        vertices[i].position = glm::vec3(
            vertexPositions[i][0],
            vertexPositions[i][1],
            vertexPositions[i][2]);

        if (normals.size() == m_geometry->getNumVertices())
        {
            vertices[i].normal = glm::vec3(
                normals[i][0],
                normals[i][1],
                normals[i][2]);
        }

        if (tangents.size() == m_geometry->getNumVertices())
        {
            vertices[i].tangent = glm::vec3(
                tangents[i][0],
                tangents[i][1],
                tangents[i][2]);
        }

        if (UVs && UVs->size() == m_geometry->getNumVertices())
        {
            vertices[i].uv = glm::vec2(
                (*UVs)[i][0],
                (*UVs)[i][1]);
        }
    }

    m_vertexBuffer->unmapVertices();

    auto triangles = (std::array<uint32_t, 3> *)m_vertexBuffer->mapTriangles();

    m_vertexBuffer->setNumIndices((uint32_t)m_geometry->getNumTriangles() * 3);
    for (unsigned i = 0; i < m_geometry->getNumTriangles(); i++)
    {
        triangles[i][0] = (uint32_t)m_geometry->getTrianglesVertices()[i][0];
        triangles[i][1] = (uint32_t)m_geometry->getTrianglesVertices()[i][1];
        triangles[i][2] = (uint32_t)m_geometry->getTrianglesVertices()[i][2];
    }
    m_vertexBuffer->unmapTriangles();
}

void
VulkanSurfaceMeshRenderDelegate::update()
{
    this->updateUniforms();

    if (m_geometry->m_dataModified)
    {
        m_geometry->computeVertexNormals();
        this->updateVertexBuffer();
        m_geometry->m_dataModified = false;
    }
}

std::shared_ptr<Geometry>
VulkanSurfaceMeshRenderDelegate::getGeometry() const
{
    return m_geometry;
}
}