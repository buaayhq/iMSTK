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

#include "imstkVisualModel.h"
#include "imstkRenderMaterial.h"
#include "imstkGeometry.h"
#include "imstkPointSet.h"
#include "imstkSurfaceMesh.h"
#include "imstkImageData.h"

namespace imstk
{
VisualModel::VisualModel() :
    m_name(""),
    m_delegateHint(""),
    m_geometry(nullptr),
    m_renderMaterial(std::make_shared<RenderMaterial>()),
    m_isVisible(true)
{
}

const std::string
VisualModel::getDelegateHint() const
{
    // Prioritize user set delegate hint
    if (m_delegateHint != "")
    {
        return m_delegateHint;
    }

    // Special Handling
    if (getRenderMaterial()->getDisplayMode() == RenderMaterial::DisplayMode::Fluid
        && std::dynamic_pointer_cast<PointSet>(m_geometry) != nullptr)
    {
        return "Fluid";
    }

    if (getRenderMaterial()->getDisplayMode() == RenderMaterial::DisplayMode::SurfaceNormals
        && std::dynamic_pointer_cast<SurfaceMesh>(m_geometry) != nullptr)
    {
        return "SurfaceNormals";
    }

    if (getRenderMaterial()->getDisplayMode() == RenderMaterial::DisplayMode::Points
        && std::dynamic_pointer_cast<ImageData>(m_geometry) != nullptr)
    {
        return "PointSet"; // Match Point set Type name
    }

    // Default delegate through geometry type
    if (m_geometry != nullptr)
    {
        return m_geometry->getTypeName();
    }

    return "";
}

void
VisualModel::setRenderMaterial(std::shared_ptr<RenderMaterial> renderMaterial)
{
    m_renderMaterial = renderMaterial;
    this->postModified();
}

void
VisualModel::setIsVisible(const bool visible)
{
    m_isVisible = visible;
    this->postModified();
}

bool
VisualModel::getRenderDelegateCreated(Renderer* ren)
{
    if (m_renderDelegateCreated.count(ren) == 0)
    {
        m_renderDelegateCreated[ren] = false;
        return false;
    }
    else
    {
        return m_renderDelegateCreated[ren];
    }
}
} // namespace imstk
