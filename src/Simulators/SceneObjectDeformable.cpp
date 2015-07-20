// This file is part of the SimMedTK project.
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------

// SimMedTK includes
#include "SceneObjectDeformable.h"
#include "Core/Factory.h"

smSceneObjectDeformable::smSceneObjectDeformable() :
    renderSecondaryMesh(false),
    topologyAltered(false),
    pulledVertex(-1),
    timestepCounter(0),
    subTimestepCounter(0)
{
  this->setRenderDelegate(
    Factory<RenderDelegate>::createSubclass(
      "RenderDelegate", "SceneObjectDeformableRenderDelegate"));
}

smSceneObjectDeformable::~smSceneObjectDeformable()
{
}

void smSceneObjectDeformable::applyContactForces()
{
    if (f_contact.size() != 0)
    {
        for (size_t i = 0; i < f_contact.size(); i++)
        {
            f_ext[i] += f_contact[i];
        }
    }
}

void smSceneObjectDeformable::setContactForcesToZero()
{
    f_contact.assign(f_contact.size(), 0.0);
}

void smSceneObjectDeformable::setContactForceOfNodeWithDofID(const int dofID,
                                                             const core::Vec3d force)
{
    f_contact[dofID] = force(0);
    f_contact[dofID + 1] = force(1);
    f_contact[dofID + 2] = force(2);
}

core::Vec3d smSceneObjectDeformable::getVelocityOfNodeWithDofID(const int dofID) const
{
    core::Vec3d vel(uvel[dofID], uvel[dofID + 1], uvel[dofID + 2]);

    return vel;
}

core::Vec3d smSceneObjectDeformable::getDisplacementOfNodeWithDofID(const int dofID) const
{
    core::Vec3d disp(u[dofID], u[dofID + 1], u[dofID + 2]);

    return disp;
}

core::Vec3d smSceneObjectDeformable::getAccelerationOfNodeWithDofID(const int dofID) const
{
    core::Vec3d accn(uaccel[dofID], uaccel[dofID + 1], uaccel[dofID + 2]);

    return accn;
}

int smSceneObjectDeformable::getNumNodes() const
{
    return numNodes;
}

int smSceneObjectDeformable::getNumTotalDof() const
{
    return numTotalDOF;
}

int smSceneObjectDeformable::getNumDof() const
{
    return numDOF;
}

int smSceneObjectDeformable::getNumFixedNodes() const
{
    return numFixedNodes;
}

int smSceneObjectDeformable::getNumFixedDof() const
{
    return numFixedDof;
}

void smSceneObjectDeformable::setRenderDetail(const std::shared_ptr<RenderDetail> &r)
{
    primarySurfaceMesh->setRenderDetail(r);

    if (secondarySurfaceMesh != nullptr)
    {
        secondarySurfaceMesh->setRenderDetail(r);
    }
}

void smSceneObjectDeformable::setRenderSecondaryMesh()
{
    if (secondarySurfaceMesh != nullptr)
    {
        renderSecondaryMesh = true;
    }
    else
    {
        std::cout << "Secondary rendering mesh is not initilized! Cannot render secondary mesh\n";
    }
}

void smSceneObjectDeformable::setRenderPrimaryMesh()
{
    renderSecondaryMesh = false;
}

std::shared_ptr<SurfaceMesh> smSceneObjectDeformable::getPrimarySurfaceMesh() const
{
    return primarySurfaceMesh;
}

std::shared_ptr<SurfaceMesh> smSceneObjectDeformable::getSecondarySurfaceMesh() const
{
    return secondarySurfaceMesh;
}
