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
#include "imstkDebugGeometryObject.h"
#include "imstkDirectionalLight.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLogger.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkVTKTextStatusManager.h"
#include "imstkVTKViewer.h"

using namespace imstk;

static Vec3d
getRandomPositions(const double radius)
{
    return radius * Vec3d(
        2.0 * static_cast<double>(rand()) / static_cast<double>(RAND_MAX) - 1.0,
        2.0 * static_cast<double>(rand()) / static_cast<double>(RAND_MAX) - 1.0,
        2.0 * static_cast<double>(rand()) / static_cast<double>(RAND_MAX) - 1.0);
}

static Color
getRandomColor()
{
    return Color(
        static_cast<double>(rand()) / static_cast<double>(RAND_MAX),
        static_cast<double>(rand()) / static_cast<double>(RAND_MAX),
        static_cast<double>(rand()) / static_cast<double>(RAND_MAX),
        1.0);
}

///
/// \brief This example demonstrates debug rendering in iMSTK
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Create a scene
    imstkNew<Scene> scene("Debug rendering example");
    scene->getConfig()->debugCamBoundingBox = false;
    scene->getCamera("debug")->setPosition(0.0, 0.0, 50.0);

    // Setup a viewer to render in its own thread
    imstkNew<VTKViewer> viewer;
    viewer->setActiveScene(scene);
    viewer->setWindowTitle("Debug Rendering");
    viewer->setSize(1920, 1080);

    // Seed with system time
    srand(time(NULL));

    auto statusManager = viewer->getTextStatusManager();
    statusManager->setStatusFontSize(VTKTextStatusManager::StatusType::Custom, 30);
    statusManager->setStatusFontColor(VTKTextStatusManager::StatusType::Custom, Color::Orange);

    imstkNew<DebugGeometryObject> debugGeometryObj;
    scene->addSceneObject(debugGeometryObj);

    int mode  = 0; // 0: add point, 1: add line, 2: add triangle
    int count = 0; // The number of times cycling between modes

    auto updateFunc =
        [&](Event*)
        {
            if (count > 100)
            {
                count = 0;
                debugGeometryObj->clear();
            }

            if (mode % 3 == 0)
            {
                debugGeometryObj->addPoint(
                    getRandomPositions(15.0),
                    getRandomColor());
            }
            else if (mode % 3 == 1)
            {
                Vec3d p     = getRandomPositions(50.0);
                Vec3d shift = getRandomPositions(1.0);
                debugGeometryObj->addLine(p + shift, -p + shift, getRandomColor());
            }
            else
            {
                Vec3d shift = getRandomPositions(10.0);
                debugGeometryObj->addTriangle(
                    getRandomPositions(5.0) + shift,
                    getRandomPositions(5.0) + shift,
                    getRandomPositions(5.0) + shift,
                    getRandomColor());

                mode = -1;
                count++;
            }
            mode++;

            statusManager->setCustomStatus("Primitives: " +
                           std::to_string(debugGeometryObj->getNumPoints()) + " (points) | " +
                           std::to_string(debugGeometryObj->getNumLines()) + " (lines) | " +
                           std::to_string(debugGeometryObj->getNumTriangles()) + " (triangles)"
                );
        };

    // Set Camera configuration
    scene->getActiveCamera()->setPosition(Vec3d(0.0, 0.0, 50.0));

    // Light
    imstkNew<DirectionalLight> light1;
    light1->setFocalPoint(Vec3d(-1.0, -1.0, -1.0));
    light1->setIntensity(1.0);
    scene->addLight("light1", light1);

    // Run the simulation
    {
        // Setup a scene manager to advance the scene in its own thread
        imstkNew<SceneManager> sceneManager;
        sceneManager->setActiveScene(scene);
        connect<Event>(sceneManager, &SceneManager::postUpdate, updateFunc);

        imstkNew<SimulationManager> driver;
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.1);

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

        driver->start();
    }

    return 0;
}
