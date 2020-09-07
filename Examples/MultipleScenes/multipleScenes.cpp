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

#include "imstkAPIUtilities.h"
#include "imstkCamera.h"
#include "imstkConsoleThread.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkLogger.h"
#include "imstkMeshIO.h"
#include "imstkNew.h"
#include "imstkOneToOneMap.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPlane.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkViewer.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"
#include "imstkLight.h"

using namespace imstk;

///
/// \brief Creates cloth geometry
///
static std::shared_ptr<SurfaceMesh>
makeClothGeometry(const double width,
                  const double height,
                  const int    nRows,
                  const int    nCols)
{
    imstkNew<SurfaceMesh> clothMesh;

    StdVectorOfVec3d vertList;
    vertList.resize(nRows * nCols);
    const double dy = width / (double)(nCols - 1);
    const double dx = height / (double)(nRows - 1);
    for (int i = 0; i < nRows; ++i)
    {
        for (int j = 0; j < nCols; j++)
        {
            vertList[i * nCols + j] = Vec3d((double)dx * i, 1.0, (double)dy * j);
        }
    }
    clothMesh->setInitialVertexPositions(vertList);
    clothMesh->setVertexPositions(vertList);

    // Add connectivity data
    std::vector<SurfaceMesh::TriangleArray> triangles;
    for (std::size_t i = 0; i < nRows - 1; ++i)
    {
        for (std::size_t j = 0; j < nCols - 1; j++)
        {
            SurfaceMesh::TriangleArray tri[2];
            const size_t               index1 = i * nCols + j;
            const size_t               index2 = index1 + nCols;
            const size_t               index3 = index1 + 1;
            const size_t               index4 = index2 + 1;

            // Interleave [/][\]
            if (i % 2 ^ j % 2)
            {
                tri[0] = { { index1, index2, index3 } };
                tri[1] = { { index4, index3, index2 } };
            }
            else
            {
                tri[0] = { { index2, index4, index1 } };
                tri[1] = { { index4, index3, index1 } };
            }
            triangles.push_back(tri[0]);
            triangles.push_back(tri[1]);
        }
    }

    clothMesh->setTrianglesVertices(triangles);

    return clothMesh;
}

///
/// \brief Creates cloth object
///
static std::shared_ptr<PbdObject>
makeClothObj(const std::string& name,
             const double       width,
             const double       height,
             const int          nRows,
             const int          nCols)
{
    imstkNew<PbdObject> clothObj(name);

    // Setup the Geometry
    std::shared_ptr<SurfaceMesh> clothMesh(makeClothGeometry(width, height, nRows, nCols));

    // Setup the Parameters
    imstkNew<PBDModelConfig> pbdParams;
    pbdParams->enableConstraint(PbdConstraint::Type::Distance, 1.0e2);
    pbdParams->enableConstraint(PbdConstraint::Type::Dihedral, 1.0e1);
    pbdParams->m_fixedNodeIds     = { 0, static_cast<size_t>(nCols) - 1 };
    pbdParams->m_uniformMassValue = width * height / ((double)nRows * (double)nCols);
    pbdParams->m_gravity    = Vec3d(0.0, -9.8, 0.0);
    pbdParams->m_defaultDt  = 0.005;
    pbdParams->m_iterations = 5;

    // Setup the Model
    imstkNew<PbdModel> pbdModel;
    pbdModel->setModelGeometry(clothMesh);
    pbdModel->configure(pbdParams);

    // Setup the VisualModel
    imstkNew<RenderMaterial> material;
    material->setBackFaceCulling(false);
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);

    imstkNew<VisualModel> visualModel(clothMesh);
    visualModel->setRenderMaterial(material);

    // Setup the Object
    clothObj->addVisualModel(visualModel);
    clothObj->setPhysicsGeometry(clothMesh);
    clothObj->setDynamicalModel(pbdModel);

    return clothObj;
}

std::shared_ptr<Scene>
createSoftBodyScene(std::string sceneName)
{
    imstkNew<SceneConfig> sceneConfig;
    sceneConfig->lazyInitialization = true;
    imstkNew<Scene> scene(sceneName, sceneConfig);

    scene->getActiveCamera()->setPosition(0.0, 2.0, 15.0);

    // Load a sample mesh
    auto tetMesh = MeshIO::read<TetrahedralMesh>(iMSTK_DATA_ROOT "/asianDragon/asianDragon.veg");

    // Extract the surface mesh
    imstkNew<SurfaceMesh> surfMesh;
    tetMesh->extractSurfaceMesh(surfMesh, true);

    imstkNew<RenderMaterial> material;
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    imstkNew<VisualModel> surfMeshModel(surfMesh.get());
    surfMeshModel->setRenderMaterial(material);

    imstkNew<PbdObject> deformableObj("Dragon");
    imstkNew<PbdModel>  pbdModel;
    pbdModel->setModelGeometry(tetMesh);

    // configure model
    imstkNew<PBDModelConfig> pbdParams;

    // FEM constraint
    pbdParams->m_femParams->m_YoungModulus = 100.0;
    pbdParams->m_femParams->m_PoissonRatio = 0.3;
    pbdParams->m_fixedNodeIds = { 51, 127, 178 };
    pbdParams->enableFEMConstraint(PbdConstraint::Type::FEMTet, PbdFEMConstraint::MaterialType::StVK);

    // Other parameters
    pbdParams->m_uniformMassValue = 1.0;
    pbdParams->m_gravity    = Vec3d(0.0, -9.8, 0.0);
    pbdParams->m_iterations = 45;

    // Set the parameters
    pbdModel->configure(pbdParams);
    deformableObj->setDynamicalModel(pbdModel);
    deformableObj->addVisualModel(surfMeshModel);
    deformableObj->setPhysicsGeometry(tetMesh);
    deformableObj->setPhysicsToVisualMap(std::make_shared<OneToOneMap>(tetMesh, surfMesh));
    deformableObj->setDynamicalModel(pbdModel);

    scene->addSceneObject(deformableObj);

    imstkNew<Plane> planeGeom;
    planeGeom->setWidth(40.0);
    planeGeom->setTranslation(0, -6, 0);
    imstkNew<CollidingObject> planeObj("Plane");
    planeObj->setVisualGeometry(planeGeom);
    planeObj->setCollidingGeometry(planeGeom);
    scene->addSceneObject(planeObj);

    return scene;
}

std::shared_ptr<Scene>
createClothScene(std::string sceneName)
{
    imstkNew<Scene> scene("PBDCloth");
    {
        std::shared_ptr<PbdObject> clothObj = makeClothObj("Cloth", 10.0, 10.0, 16, 16);
        scene->addSceneObject(clothObj);

        // Light (white)
        imstkNew<DirectionalLight> whiteLight("whiteLight");
        whiteLight->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
        whiteLight->setIntensity(1.0);
        scene->addLight(whiteLight);

        // Light (red)
        imstkNew<SpotLight> colorLight("colorLight");
        colorLight->setPosition(Vec3d(-5.0, -3.0, 5.0));
        colorLight->setFocalPoint(Vec3d(0.0, -5.0, 5.0));
        colorLight->setIntensity(100.);
        colorLight->setColor(Color::Red);
        colorLight->setSpotAngle(30.0);
        scene->addLight(colorLight);

        // Adjust camera
        scene->getActiveCamera()->setFocalPoint(0.0, -5.0, 5.0);
        scene->getActiveCamera()->setPosition(-15.0, -5.0, 25.0);
        scene->getActiveCamera()->setPosition(-15., -5.0, 15.0);
    }
    return scene;
}

void
testMultipleScenesInBackendMode()
{
    imstkNew<SceneManager> sceneManager("SceneManager");
    auto                   scene1 = createClothScene("clothScene");
    sceneManager->addScene(scene1);
    auto scene2 = createSoftBodyScene("deformableBodyScene");
    sceneManager->addScene(scene2);

    scene1->initialize();
    scene2->initialize();

    // set to scene 1
    sceneManager->setActiveScene(scene1);

    sceneManager->update();
    sceneManager->update();
    sceneManager->update();
    sceneManager->update();

    // set to scene 2
    sceneManager->setActiveScene(scene2);

    sceneManager->update();
    sceneManager->update();
    sceneManager->update();
    sceneManager->update();

    // set to scene 1
    sceneManager->setActiveScene(scene1);

    sceneManager->update();
    sceneManager->update();
    sceneManager->update();
    sceneManager->update();

    // set to scene 2
    sceneManager->setActiveScene(scene2);

    sceneManager->update();
    sceneManager->update();
    sceneManager->update();
    sceneManager->update();

    std::cout << "Press any key to exit..." << std::endl;

    getchar();
}

void
testMultipleScenesInRenderMode()
{
    // Simulation manager defaults to rendering mode
    auto scene1 = createClothScene("clothScene");
    auto scene2 = createSoftBodyScene("deformableBodyScene");

    scene1->getConfig()->trackFPS = true;

    scene1->initialize();
    scene2->initialize();

    // set to scene 1
    imstkNew<SceneManager> sceneManager("SceneManager");
    sceneManager->setActiveScene(scene2);

    // Setup a viewer to render in its own thread
    imstkNew<VTKViewer> viewer("Viewer");
    viewer->setActiveScene(scene2);
    viewer->addChildThread(sceneManager); // SceneManager will start/stop with viewer

    // Create a call back on key press of 's' to switch scenes
    LOG(INFO) << "s/S followed by enter to switch scenes";
    LOG(INFO) << "q/Q followed by enter to quit";

    connect<KeyPressEvent>(viewer->getKeyboardDevice(), EventType::KeyPress,
        [&](KeyPressEvent* e)
    {
        if (e->m_keyPressType == KEY_PRESS)
        {
            if (e->m_key == 's' || e->m_key == 'S')
            {
                if (sceneManager->getActiveScene() == scene1)
                {
                    sceneManager->setActiveScene(scene2);
                    viewer->setActiveScene(scene2);
                }
                else
                {
                    sceneManager->setActiveScene(scene1);
                    viewer->setActiveScene(scene1);
                }

                if (!sceneManager->getActiveScene())
                {
                    sceneManager->setActiveScene(scene1);
                    viewer->setActiveScene(scene1);
                }
            }
            else if (e->m_key == 'q' || e->m_key == 'Q')
            {
                viewer->stop(false);
            }
        }
        });

    if (scene1->getConfig()->trackFPS)
    {
        apiutils::printUPS(sceneManager);
    }

    viewer->start();
}

void
testMultipleScenesInBackgroundMode()
{
    imstkNew<ConsoleThread> consoleThread;

    imstkNew<SceneManager> sceneManager("SceneManager");
    consoleThread->addChildThread(sceneManager); // Start/stop scene with console
    auto scene1 = createClothScene("clothScene");
    auto scene2 = createSoftBodyScene("deformableBodyScene");
    sceneManager->addScene(scene1);
    sceneManager->addScene(scene2);

    scene1->initialize();
    scene2->initialize();

    sceneManager->setActiveScene(scene1);

    LOG(INFO) << "s/S followed by enter to switch scenes";
    LOG(INFO) << "q/Q followed by enter to quit";
    auto keyPressFunc = [&](KeyPressEvent* e)
                        {
                            if (e->m_keyPressType == KEY_PRESS)
                            {
                                if (e->m_key == 's' || e->m_key == 'S')
                                {
                                    if (sceneManager->getActiveScene()->getName() == scene1->getName())
                                    {
                                        sceneManager->setActiveScene(scene2);
                                    }
                                    else
                                    {
                                        sceneManager->setActiveScene(scene1);
                                    }
                                }
                                else if (e->m_key == 'q' || e->m_key == 'Q')
                                {
                                    consoleThread->stop(false);
                                }
                            }
                        };
    connect<KeyPressEvent>(consoleThread->getKeyboardDevice(), EventType::KeyPress, keyPressFunc);

    consoleThread->start();
}

///
/// \brief Test multiple scenes
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    testMultipleScenesInBackendMode();
    testMultipleScenesInBackgroundMode();
    testMultipleScenesInRenderMode();

    return 0;
}
