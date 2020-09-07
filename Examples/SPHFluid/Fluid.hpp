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

#include "imstkPointSet.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSPHObject.h"
#include "imstkSPHModel.h"
#include "imstkVisualModel.h"

using namespace imstk;

///
/// \brief Generate a sphere-shape fluid object
///
StdVectorOfVec3d
generateSphereShapeFluid(const double particleRadius)
{
    const double sphereRadius = 2.0;
    const Vec3d  sphereCenter(0, 1, 0);

    const auto  sphereRadiusSqr = sphereRadius * sphereRadius;
    const auto  spacing         = 2.0 * particleRadius;
    const auto  N               = static_cast<size_t>(2.0 * sphereRadius / spacing);              // Maximum number of particles in each dimension
    const Vec3d lcorner         = sphereCenter - Vec3d(sphereRadius, sphereRadius, sphereRadius); // Cannot use auto here, due to Eigen bug

    StdVectorOfVec3d particles;
    particles.reserve(N * N * N);

    for (size_t i = 0; i < N; ++i)
    {
        for (size_t j = 0; j < N; ++j)
        {
            for (size_t k = 0; k < N; ++k)
            {
                Vec3d ppos = lcorner + Vec3d(spacing * double(i), spacing * double(j), spacing * double(k));
                Vec3d cx   = ppos - sphereCenter;
                if (cx.squaredNorm() < sphereRadiusSqr)
                {
                    particles.push_back(ppos);
                }
            }
        }
    }

    return particles;
}

///
/// \brief Generate a box-shape fluid object
///
StdVectorOfVec3d
generateBoxShapeFluid(const double particleRadius)
{
    const double boxWidth = 4.0;
    const Vec3d  boxLowerCorner(-2, -3, -2);

    const auto spacing = 2.0 * particleRadius;
    const auto N       = static_cast<size_t>(boxWidth / spacing);

    StdVectorOfVec3d particles;
    particles.reserve(N * N * N);

    for (size_t i = 0; i < N; ++i)
    {
        for (size_t j = 0; j < N; ++j)
        {
            for (size_t k = 0; k < N; ++k)
            {
                Vec3d ppos = boxLowerCorner + Vec3d(spacing * double(i), spacing * double(j), spacing * double(k));
                particles.push_back(ppos);
            }
        }
    }

    return particles;
}

#if SCENE_ID == 3
StdVectorOfVec3d getBunny(); // Defined in Bunny.cpp
#endif
///
/// \brief Generate a bunny-shape fluid object
///
StdVectorOfVec3d
generateBunnyShapeFluid(const double particleRadius)
{
    LOG_IF(FATAL, (std::abs(particleRadius - 0.08) > 1e-6)) << "Particle radius for this scene must be 0.08";
    StdVectorOfVec3d particles;
#if SCENE_ID == 3
    particles = getBunny();
#endif
    return particles;
}

std::shared_ptr<SPHObject>
generateFluid(const double particleRadius)
{
    StdVectorOfVec3d particles;
    switch (SCENE_ID)
    {
    case 1:
        particles = generateSphereShapeFluid(particleRadius);
        break;
    case 2:
        particles = generateBoxShapeFluid(particleRadius);
        break;
    case 3:
        particles = generateBunnyShapeFluid(particleRadius);
        break;
    default:
        LOG(FATAL) << "Invalid scene index";
    }

    LOG(INFO) << "Number of particles: " << particles.size();

    // Create a geometry object
    imstkNew<PointSet> geometry;
    geometry->initialize(particles);

    // Create a fluids object
    imstkNew<SPHObject> fluidObj("Sphere");

    // Create a visual model
    imstkNew<VisualModel> visualModel(geometry.get());
    imstkNew<RenderMaterial> material;
    material->setDisplayMode(RenderMaterial::DisplayMode::Fluid);
    material->setVertexColor(Color::Orange);
    if (material->getDisplayMode() == RenderMaterial::DisplayMode::Fluid)
    {
        material->setPointSize(0.1);
    }
    else
    {
        material->setPointSize(15.0);
    }
    visualModel->setRenderMaterial(material);

    // Create a physics model
    imstkNew<SPHModel> sphModel;
    sphModel->setModelGeometry(geometry);

    // Configure model
    imstkNew<SPHModelConfig> sphParams(particleRadius);
    sphParams->m_bNormalizeDensity = true;
    if (SCENE_ID == 2)   // highly viscous fluid
    {
        sphParams->m_kernelOverParticleRadiusRatio = 6.0;
        sphParams->m_viscosityCoeff                = 0.5;
        sphParams->m_surfaceTensionStiffness       = 5.0;
    }

    if (SCENE_ID == 3)   // bunny-shaped fluid
    {
        sphParams->m_frictionBoundary= 0.3;
    }

    sphModel->configure(sphParams);
    sphModel->setTimeStepSizeType(TimeSteppingType::RealTime);

    // Add the component models
    fluidObj->addVisualModel(visualModel);
    fluidObj->setCollidingGeometry(geometry);
    fluidObj->setDynamicalModel(sphModel);
    fluidObj->setPhysicsGeometry(geometry);

    return fluidObj;
}
