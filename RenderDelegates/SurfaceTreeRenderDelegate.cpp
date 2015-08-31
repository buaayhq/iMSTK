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

#include "Core/Model.h"
#include "Core/RenderDelegate.h"
#include "Core/Factory.h"
#include "Collision/MeshCollisionModel.h"
#include "Collision/SurfaceTree.h"

class SurfaceTreeRenderDelegate : public RenderDelegate
{
public:
  virtual void draw() const override;

  template<typename T>
  bool drawTree() const;
};

/// \brief Draw the surface tree.
void SurfaceTreeRenderDelegate::draw() const
{
  if (this->drawTree<SurfaceTree<MeshCollisionModel::AABBNodeType>>())
    return;
  if (this->drawTree<SurfaceTree<OctreeCell>>())
    return;
}

template<typename T>
bool SurfaceTreeRenderDelegate::drawTree() const
{
  T* geom = this->getSourceGeometryAs<T>();
  if (!geom)
    return false;

  core::Vec3d center;
  glColor3fv(Color::colorGreen.toGLColor<GLfloat>());

  glEnable(GL_LIGHTING);
  glPushAttrib(GL_LIGHTING_BIT);
  glColor3fv(Color::colorGreen.toGLColor<GLfloat>());
  glColor3fv(Color::colorBlue.toGLColor<GLfloat>());

  glPushMatrix();
  glColor3fv(Color::colorPink.toGLColor<GLfloat>());

  geom->getRoot()->draw();

  glPopMatrix();


  glPopAttrib();
  glEnable(GL_LIGHTING);
  return true;
}

RegisterFactoryClass(RenderDelegate,
                     SurfaceTreeRenderDelegate,
                     RenderDelegate::RendererType::Other)
