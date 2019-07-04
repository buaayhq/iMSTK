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

#include "imstkSimulationManager.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdSolver.h"
#include "imstkTetrahedralMesh.h"
#include "imstkMeshIO.h"
#include "imstkOneToOneMap.h"
#include "imstkMeshToMeshBruteforceCD.h"
#include "imstkPBDCollisionHandling.h"

using namespace imstk;

///
/// \brief This example demonstrates the collision interaction
/// using Position based dynamics
///
int main()
{
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("PbdCollision");

    scene->getCamera()->setPosition(0, 10.0, 10.0);

    // Load a sample mesh
    auto tetMesh = MeshIO::read(iMSTK_DATA_ROOT "/asianDragon/asianDragon.veg");
    if (!tetMesh)
    {
        LOG(WARNING) << "Could not read mesh from file.";
        return 1;
    }

    auto surfMesh = std::make_shared<SurfaceMesh>();
    auto surfMeshVisual = std::make_shared<SurfaceMesh>();
    auto volTetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(tetMesh);
    if (!volTetMesh)
    {
        LOG(WARNING) << "Dynamic pointer cast from PointSet to TetrahedralMesh failed!";
        return 1;
    }
    volTetMesh->extractSurfaceMesh(surfMesh, true);

    auto material = std::make_shared<RenderMaterial>();
    material->setDisplayMode(RenderMaterial::DisplayMode::WIREFRAME_SURFACE);
    auto surfMeshModel = std::make_shared<VisualModel>(surfMesh);
    surfMeshModel->setRenderMaterial(material);

    auto deformMapP2V = std::make_shared<OneToOneMap>(tetMesh, surfMesh);
    auto deformMapC2V = std::make_shared<OneToOneMap>(surfMesh, surfMesh);
    auto deformMapP2C = std::make_shared<OneToOneMap>(tetMesh, surfMesh);

    auto deformableObj = std::make_shared<PbdObject>("Dragon");
    deformableObj->addVisualModel(surfMeshModel);
    deformableObj->setCollidingGeometry(surfMesh);
    deformableObj->setPhysicsGeometry(volTetMesh);
    deformableObj->setPhysicsToCollidingMap(deformMapP2C);
    deformableObj->setPhysicsToVisualMap(deformMapP2V);
    deformableObj->setCollidingToVisualMap(deformMapC2V);

    // Create model and object
    auto pbdModel = std::make_shared<PbdModel>();
    pbdModel->setModelGeometry(volTetMesh);

    // configure model
    auto pbdParams = std::make_shared<PBDModelConfig>();

    // FEM constraint
    pbdParams->m_YoungModulus = 1000.0;
    pbdParams->m_PoissonRatio = 0.3;
    pbdParams->enableFEMConstraint(PbdConstraint::Type::FEMTet, PbdFEMConstraint::MaterialType::Corotation);

    // Other parameters
    pbdParams->m_uniformMassValue = 1.0;
    pbdParams->m_gravity = Vec3d(0, -10.0, 0);
    pbdParams->m_dt = 0.01;
    pbdParams->m_maxIter = 5;
    pbdParams->m_proximity = 0.3;
    pbdParams->m_contactStiffness = 0.1;

    pbdModel->configure(pbdParams);
    deformableObj->setDynamicalModel(pbdModel);

    // Create solver
    auto pbdSolver = std::make_shared<PbdSolver>();
    pbdSolver->setPbdObject(deformableObj);
    scene->addNonlinearSolver(pbdSolver);

    scene->addSceneObject(deformableObj);

    // Build floor geometry
    const double width = 100.0;
	const double height = 100.0;
    const size_t nRows = 2;
	const size_t nCols = 2;
    const double dy = width / static_cast<double>(nCols - 1);
    const double dx = height / static_cast<double>(nRows - 1);

    StdVectorOfVec3d vertList;
    vertList.resize(nRows * nCols);
    for (size_t i = 0; i < nRows; ++i)
    {
        for (size_t j = 0; j < nCols; j++)
        {
            const double y = static_cast<double>(dy * j);
            const double x = static_cast<double>(dx * i);
            vertList[i * nCols + j] = Vec3d(x - 50, -10.0, y - 50);
        }
    }

    // c. Add connectivity data
    std::vector<SurfaceMesh::TriangleArray> triangles;
    for (std::size_t i = 0; i < nRows - 1; ++i)
    {
        for (std::size_t j = 0; j < nCols - 1; j++)
        {
            SurfaceMesh::TriangleArray tri[2];
            tri[0] = { { i*nCols + j, i*nCols + j + 1, (i + 1) * nCols + j } };
            tri[1] = { { (i + 1) * nCols + j + 1, (i + 1) * nCols + j, i * nCols + j + 1 } };
            triangles.push_back(tri[0]);
            triangles.push_back(tri[1]);
        }
    }

    auto floorMesh = std::make_shared<SurfaceMesh>();
    floorMesh->initialize(vertList, triangles);

    auto materialFloor = std::make_shared<RenderMaterial>();
    materialFloor->setDisplayMode(RenderMaterial::DisplayMode::WIREFRAME_SURFACE);
    auto floorMeshModel = std::make_shared<VisualModel>(floorMesh);
    floorMeshModel->setRenderMaterial(materialFloor);

    auto floor = std::make_shared<PbdObject>("Floor");
    floor->setCollidingGeometry(floorMesh);
    floor->setVisualGeometry(floorMesh);
    floor->setPhysicsGeometry(floorMesh);

    auto pbdModel2 = std::make_shared<PbdModel>();
    pbdModel2->setModelGeometry(floorMesh);

    // configure model
    auto pbdParams2 = std::make_shared<PBDModelConfig>();
    pbdParams2->m_uniformMassValue = 0.0;
    pbdParams2->m_proximity = 0.1;
    pbdParams2->m_contactStiffness = 1.0;

    // Set the parameters
    pbdModel2->configure(pbdParams2);
    floor->setDynamicalModel(pbdModel2);

    auto pbdSolverfloor = std::make_shared<PbdSolver>();
    pbdSolverfloor->setPbdObject(floor);
    scene->addNonlinearSolver(pbdSolverfloor);

    scene->addSceneObject(floor);

    // Collision
    auto colData = std::make_shared<CollisionData>();
    auto CD = std::make_shared<MeshToMeshBruteForceCD>(surfMesh, floorMesh, colData);

    auto CH = std::make_shared<PBDCollisionHandling>(CollisionHandling::Side::A,
                    CD->getCollisionData(), deformableObj, floor, pbdSolver);
    scene->getCollisionGraph()->addInteractionPair(deformableObj, floor, CD, CH, nullptr);

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    sdk->setActiveScene(scene);
    sdk->startSimulation(SimulationStatus::RUNNING);

    return 0;
}
