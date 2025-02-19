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

#include "imstkCamera.h"
#include "imstkCapsule.h"
#include "imstkDirectionalLight.h"
#include "imstkGeometryUtilities.h"
#include "imstkHapticDeviceClient.h"
#include "imstkHapticDeviceManager.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLaparoscopicToolController.h"
#include "imstkMeshIO.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCollision.h"
#include "imstkPbdObjectGrasping.h"
#include "imstkPointwiseMap.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

///
/// \brief Spherically project the texture coordinates
///
static void
setSphereTexCoords(std::shared_ptr<SurfaceMesh> surfMesh, const double uvScale)
{
    Vec3d min, max;
    surfMesh->computeBoundingBox(min, max);
    const Vec3d size   = max - min;
    const Vec3d center = (max + min) * 0.5;

    const double radius = (size * 0.5).norm();

    imstkNew<VecDataArray<float, 2>> uvCoordsPtr(surfMesh->getNumVertices());
    VecDataArray<float, 2>&          uvCoords = *uvCoordsPtr.get();
    for (int i = 0; i < surfMesh->getNumVertices(); i++)
    {
        Vec3d vertex = surfMesh->getVertexPosition(i) - center;

        // Compute phi and theta on the sphere
        const double theta = asin(vertex[0] / radius);
        const double phi   = atan2(vertex[1], vertex[2]);
        uvCoords[i] = Vec2f(phi / (PI * 2.0) + 0.5, theta / (PI * 2.0) + 0.5) * uvScale;
    }
    surfMesh->setVertexTCoords("tcoords", uvCoordsPtr);
}

///
/// \brief Creates tissue object
/// \param name
/// \param physical dimension of tissue
/// \param dimensions of tetrahedral grid used for tissue
/// \param center of tissue block
///
static std::shared_ptr<PbdObject>
makeTissueObj(const std::string& name,
              const Vec3d& size, const Vec3i& dim, const Vec3d& center)
{
    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> tissueMesh = GeometryUtils::toTetGrid(center, size, dim);
    std::shared_ptr<SurfaceMesh>     surfMesh   = tissueMesh->extractSurfaceMesh();
    setSphereTexCoords(surfMesh, 6.0);

    // Setup the Parameters
    imstkNew<PbdModelConfig> pbdParams;
    const bool               useFem = true;
    if (useFem)
    {
        // Actual skin young's modulus, 0.42MPa to 0.85Mpa, as reported in papers
        // Actual skin possion ratio, 0.48, as reported in papers
        pbdParams->m_femParams->m_YoungModulus = 40000.0;
        pbdParams->m_femParams->m_PoissonRatio = 0.48;
        // FYI:
        //  - Poisson ratio gives shear to bulk, with 0.5 being complete shear
        //    where everything is like a fluid and can slide past each other. 0.0
        //    gives complete bulk where its rigid
        //  - Youngs modulus then gives the scaling of the above in pressure
        //    (pascals).
        pbdParams->enableFemConstraint(PbdFemConstraint::MaterialType::StVK);
    }
    else
    {
        pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 100000.0);
        pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Volume, 100000.0);
    }
    pbdParams->m_doPartitioning   = false;
    pbdParams->m_uniformMassValue = 100.0;
    pbdParams->m_dt = 0.001; // realtime used in update calls later in main
    pbdParams->m_iterations = 5;

    // Due to poor boundary conditions turning off gravity is useful. But then makes
    // your tissue look like it's in space (springy and no resistance). So viscous
    // damping is introduced to approximate these conditions.
    //
    // Ultimately this is a result of not modelling everything around the tissue.
    // and poor/hard to model boundary conditions.
    pbdParams->m_gravity = Vec3d::Zero();
    pbdParams->m_viscousDampingCoeff = 0.03; // Removed from velocity

    // Fix the borders
    for (int z = 0; z < dim[2]; z++)
    {
        for (int y = 0; y < dim[1]; y++)
        {
            for (int x = 0; x < dim[0]; x++)
            {
                if (x == 0 || /*z == 0 ||*/ x == dim[0] - 1 /*|| z == dim[2] - 1*/)
                {
                    pbdParams->m_fixedNodeIds.push_back(x + dim[0] * (y + dim[1] * z));
                }
            }
        }
    }

    // Setup the Model
    imstkNew<PbdModel> pbdModel;
    pbdModel->setModelGeometry(tissueMesh);
    pbdModel->configure(pbdParams);

    // Setup the material
    imstkNew<RenderMaterial> material;
    material->setDisplayMode(RenderMaterial::DisplayMode::Surface);
    material->setShadingModel(RenderMaterial::ShadingModel::PBR);
    material->addTexture(std::make_shared<Texture>(iMSTK_DATA_ROOT "/textures/fleshDiffuse.jpg",
            Texture::Type::Diffuse));
    material->addTexture(std::make_shared<Texture>(iMSTK_DATA_ROOT "/textures/fleshNormal.jpg",
            Texture::Type::Normal));
    material->addTexture(std::make_shared<Texture>(iMSTK_DATA_ROOT "/textures/fleshORM.jpg",
            Texture::Type::ORM));
    material->setNormalStrength(0.3);

    // Add a visual model to render the surface of the tet mesh
    imstkNew<VisualModel> visualModel;
    visualModel->setGeometry(surfMesh);
    visualModel->setRenderMaterial(material);

    // Setup the Object
    imstkNew<PbdObject> tissueObj(name);
    tissueObj->addVisualModel(visualModel);
    tissueObj->setPhysicsGeometry(tissueMesh);
    tissueObj->setCollidingGeometry(surfMesh);
    tissueObj->setPhysicsToCollidingMap(std::make_shared<PointwiseMap>(tissueMesh, surfMesh));
    tissueObj->setDynamicalModel(pbdModel);

    return tissueObj;
}

///
/// \brief This example demonstrates Pbd grasping. PbdObjectGrasping allows
/// us to hold onto parts of a tissue or other pbd deformable with a tool
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Scene
    imstkNew<Scene> scene("PbdTissueGraspingExample");
    scene->getActiveCamera()->setPosition(0.001, 0.05, 0.15);
    scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    scene->getActiveCamera()->setViewUp(0.0, 0.96, -0.28);

    imstkNew<Capsule> geomShaft;
    geomShaft->setLength(1.0);
    geomShaft->setRadius(0.005);
    geomShaft->setOrientation(Quatd(Rotd(PI_2, Vec3d(1.0, 0.0, 0.0))));
    geomShaft->setTranslation(Vec3d(0.0, 0.0, 0.5));
    imstkNew<CollidingObject> objShaft("ShaftObject");
    objShaft->setVisualGeometry(MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/laptool/pivot.obj"));
    objShaft->setCollidingGeometry(geomShaft);
    scene->addSceneObject(objShaft);

    imstkNew<Capsule> geomUpperJaw;
    geomUpperJaw->setLength(0.05);
    geomUpperJaw->setTranslation(Vec3d(0.0, 0.0013, -0.016));
    geomUpperJaw->setRadius(0.004);
    geomUpperJaw->setOrientation(Quatd(Rotd(PI_2, Vec3d(1.0, 0.0, 0.0))));
    imstkNew<CollidingObject> objUpperJaw("UpperJawObject");
    objUpperJaw->setVisualGeometry(MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/laptool/upper.obj"));
    objUpperJaw->setCollidingGeometry(geomUpperJaw);
    scene->addSceneObject(objUpperJaw);

    imstkNew<Capsule> geomLowerJaw;
    geomLowerJaw->setLength(0.05);
    geomLowerJaw->setTranslation(Vec3d(0.0, -0.0013, -0.016));
    geomLowerJaw->setRadius(0.004);
    geomLowerJaw->setOrientation(Quatd(Rotd(PI_2, Vec3d(1.0, 0.0, 0.0))));
    imstkNew<CollidingObject> objLowerJaw("LowerJawObject");
    objLowerJaw->setVisualGeometry(MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/laptool/lower.obj"));
    objLowerJaw->setCollidingGeometry(geomLowerJaw);
    scene->addSceneObject(objLowerJaw);

    imstkNew<Capsule> pickGeom;
    pickGeom->setLength(0.05);
    pickGeom->setTranslation(Vec3d(0.0, 0.0, -0.016));
    pickGeom->setRadius(0.006);
    pickGeom->setOrientation(Quatd(Rotd(PI_2, Vec3d(1.0, 0.0, 0.0))));

    // ~4in x 4in patch of tissue
    std::shared_ptr<PbdObject> tissueObj = makeTissueObj("PbdTissue",
        Vec3d(0.1, 0.025, 0.1), Vec3i(6, 3, 6), Vec3d(0.0, -0.03, 0.0));
    scene->addSceneObject(tissueObj);

    imstkNew<HapticDeviceManager> deviceManager;
    deviceManager->setSleepDelay(1.0);
    std::shared_ptr<HapticDeviceClient> client = deviceManager->makeDeviceClient();

    // Create and add virtual coupling object controller in the scene
    imstkNew<LaparoscopicToolController> controller;
    controller->setParts(objShaft, objUpperJaw, objLowerJaw, pickGeom);
    controller->setDevice(client);
    controller->setJawAngleChange(1.0);
    controller->setTranslationScaling(0.001);
    scene->addController(controller);

    // Add collision for both jaws of the tool
    auto upperJawCollision = std::make_shared<PbdObjectCollision>(tissueObj, objUpperJaw, "SurfaceMeshToCapsuleCD");
    auto lowerJawCollision = std::make_shared<PbdObjectCollision>(tissueObj, objLowerJaw, "SurfaceMeshToCapsuleCD");
    scene->addInteraction(upperJawCollision);
    scene->addInteraction(lowerJawCollision);

    // Add picking interaction for both jaws of the tool
    auto jawPicking = std::make_shared<PbdObjectGrasping>(tissueObj);
    // Pick the surface instead of the tetrahedral mesh
    jawPicking->setGeometryToPick(tissueObj->getVisualGeometry(),
        std::dynamic_pointer_cast<PointwiseMap>(tissueObj->getPhysicsToCollidingMap()));
    scene->addInteraction(jawPicking);

    // Light
    imstkNew<DirectionalLight> light;
    light->setFocalPoint(Vec3d(0.0, -1.0, -1.0));
    light->setIntensity(1.0);
    scene->addLight("light", light);

    // Run the simulation
    {
        // Setup a viewer to render
        imstkNew<VTKViewer> viewer;
        viewer->setActiveScene(scene);
        viewer->setDebugAxesLength(0.01, 0.01, 0.01);

        // Setup a scene manager to advance the scene
        imstkNew<SceneManager> sceneManager;
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused

        imstkNew<SimulationManager> driver;
        driver->addModule(deviceManager);
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.001);

        // Add mouse and keyboard controls to the viewer
        {
            auto mouseControl = std::make_shared<MouseSceneControl>();
            mouseControl->setDevice(viewer->getMouseDevice());
            mouseControl->setSceneManager(sceneManager);
            viewer->addControl(mouseControl);

            auto keyControl = std::make_shared<KeyboardSceneControl>();
            keyControl->setDevice(viewer->getKeyboardDevice());
            keyControl->setSceneManager(sceneManager);
            keyControl->setModuleDriver(driver);
            viewer->addControl(keyControl);
        }

        connect<Event>(sceneManager, &SceneManager::postUpdate,
            [&](Event*)
            {
                // Simulate the cloth in real time
                tissueObj->getPbdModel()->getConfig()->m_dt = sceneManager->getDt();
            });

        connect<Event>(controller, &LaparoscopicToolController::JawClosed,
            [&](Event*)
            {
                LOG(INFO) << "Jaw Closed!";

                upperJawCollision->setEnabled(false);
                lowerJawCollision->setEnabled(false);
                jawPicking->beginRayPointGrasp(pickGeom, pickGeom->getPosition(),
                    -pickGeom->getOrientation().toRotationMatrix().col(1), 0.03);
                //jawPicking->beginCellGrasp(pickGeom, "SurfaceMeshToCapsuleCD");
            });
        connect<Event>(controller, &LaparoscopicToolController::JawOpened,
            [&](Event*)
            {
                LOG(INFO) << "Jaw Opened!";

                upperJawCollision->setEnabled(true);
                lowerJawCollision->setEnabled(true);
                jawPicking->endGrasp();
            });

        driver->start();
    }

    return 0;
}
