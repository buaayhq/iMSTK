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

#include <string>

#include "imstkAPIUtilities.h"
#include "imstkCamera.h"
#include "imstkCollisionDetection.h"
#include "imstkCollisionGraph.h"
#include "imstkLight.h"
#include "imstkPlane.h"
#include "imstkPointSet.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSphere.h"
#include "imstkTaskGraph.h"
#include "imstkVTKTextStatusManager.h"
#include "imstkObjectInteractionFactory.h"
#include "imstkSPHPhysiologyInteraction.h"
#include "imstkPhysiologyModel.h"
#include "imstkPhysiologyObject.h"
#include "imstkSPHModel.h"
#include "imstkSPHObject.h"

//#include "PulsePhysiologyEngine.h"

#include "Fluid.hpp"

using namespace imstk;

int
main(int argc, char* argv[])
{
  // SimulationManager must be created first
  auto simManager = std::make_shared<SimulationManager>();

  int threads = -1;
  double particleRadius = 0.04;
  if (SCENE_ID == 5)
  {
      particleRadius = 0.012;
  }
  // Parse command line arguments
  for (int i = 1; i < argc; ++i)
  {
    auto param = std::string(argv[i]);
    if (param.find("threads") == 0
      && param.find_first_of("=") != std::string::npos)
    {
      threads = std::stoi(param.substr(param.find_first_of("=") + 1));
    }
    else if (param.find("radius") == 0
      && param.find_first_of("=") != std::string::npos)
    {
      particleRadius = std::stod(param.substr(param.find_first_of("=") + 1));
      LOG(INFO) << "Particle radius: " << particleRadius;
    }
    else
    {
      LOG(FATAL) << "Invalid argument";
    }
  }

  // Set thread pool size (nthreads <= 0 means using all logical cores)
  simManager->setThreadPoolSize(threads);

  auto scene = simManager->createNewScene("SPHPhysiologyInteraction");

  scene->getConfig()->writeTaskGraph = true;
  //scene->getConfig()->taskTimingEnabled = true;
  //scene->getTaskComputeTimes().at("PhysiologyModel_Solve");

  // Get the VTKViewer
  std::shared_ptr<VTKViewer> viewer = std::make_shared<VTKViewer>(simManager.get(), false);
  viewer->setWindowTitle("Physiolog Example");
  viewer->getVtkRenderWindow()->SetSize(1920, 1080);
  auto statusManager = viewer->getTextStatusManager();
  statusManager->setStatusFontSize(VTKTextStatusManager::Custom, 30);
  statusManager->setStatusFontColor(VTKTextStatusManager::Custom, Color::Red);
  simManager->setViewer(viewer);

  // Generate fluid and solid objects
  auto fluidObj = generateFluid(scene, particleRadius);
  std::shared_ptr<RenderMaterial> material = fluidObj->getVisualModel(0)->getRenderMaterial();
  std::shared_ptr<SPHModel> sphModel = fluidObj->getDynamicalSPHModel();

  // configure model
  auto physiologyParams = std::make_shared<PhysiologyModelConfig>();

  // Create a physics model
  auto physiologyModel = std::make_shared<PhysiologyModel>();
  physiologyModel->configure(physiologyParams);

  // Setup hemorrhage action
  const std::string hemorrhagingCompartment = "";// pulse::VascularCompartment::RightLeg;
  auto hemorrhageAction = std::make_shared<Hemorrhage>(Hemorrhage::Type::External, hemorrhagingCompartment);
  physiologyModel->addAction(hemorrhageAction);

  auto physiologyObj = std::make_shared<PhysiologyObject>("Pulse");
  physiologyObj->setDynamicalModel(physiologyModel);
  
  scene->addSceneObject(physiologyObj);

  auto interactionPair = makeObjectInteractionPair(fluidObj,
                                                   physiologyObj, 
                                                   InteractionType::SphObjToPhysiologyObjCoupling, 
                                                   CollisionDetection::Type::Custom);

  auto physiologyIp = std::dynamic_pointer_cast<SPHPhysiologyObjectInteractionPair>(interactionPair);  

  // configure the sph-physiology interaction pair
  physiologyIp->setHemorrhageAction(hemorrhageAction);
  physiologyIp->setCompartment(physiologyModel->getCompartment(physiologyCompartmentType::Liquid, hemorrhagingCompartment));

  scene->getCollisionGraph()->addInteraction(interactionPair);

  // configure camera  
  (SCENE_ID == 5) ? scene->getCamera()->setPosition(0, 1.0, 4.0): scene->getCamera()->setPosition(0, 1.0, 5.0);
    
  // configure light (white)
  auto whiteLight = std::make_shared<DirectionalLight>("whiteLight");
  whiteLight->setFocalPoint(Vec3d(5, -8, -5));
  whiteLight->setIntensity(7);
  scene->addLight(whiteLight);

  ///////////////////////////////////////
  // Setup some scalars
  std::shared_ptr<PointSet> fluidGeometry = std::dynamic_pointer_cast<PointSet>(fluidObj->getPhysicsGeometry());
  std::shared_ptr<StdVectorOfReal> scalarsPtr = std::make_shared<StdVectorOfReal>(fluidGeometry->getNumVertices());
  std::fill_n(scalarsPtr->data(), scalarsPtr->size(), 0.0);
  fluidGeometry->setScalars(scalarsPtr);

  // Setup the material for the scalars
  material->setScalarVisibility(true);
  std::shared_ptr<ColorFunction> colorFunc = std::make_shared<ColorFunction>();
  colorFunc->setNumberOfColors(2);
  colorFunc->setColor(0, Color::Red);
  colorFunc->setColor(1, Color::Green);
  colorFunc->setColorSpace(ColorFunction::ColorSpace::RGB);
  colorFunc->setRange(0, 3);
  material->setColorLookupTable(colorFunc);

  scene->setTaskGraphConfigureCallback([&](Scene* scene)
    {
      auto taskGraph = scene->getTaskGraph();
      //taskGraph->removeNode(fluidObj->getDynamicalSPHModel()->getComputeSurfaceTensionNode());

      std::shared_ptr<TaskNode> printTotalTime = std::make_shared<TaskNode>([&]()
        {
          if (fluidObj->getDynamicalSPHModel()->getTimeStepCount() % 100 == 0)
          {
            printf("Total time (s): %f\n", fluidObj->getDynamicalSPHModel()->getTotalTime());
          }
        }, "PrintTotalTime");
      taskGraph->insertAfter(fluidObj->getDynamicalSPHModel()->getMoveParticlesNode(), printTotalTime);

      std::shared_ptr<TaskNode> writeSPHStateToCSV = std::make_shared<TaskNode>([&]() {
        fluidObj->getDynamicalSPHModel()->writeStateToCSV();
        }, "WriteStateToCSV");
      taskGraph->insertAfter(fluidObj->getDynamicalSPHModel()->getMoveParticlesNode(), writeSPHStateToCSV);

      std::shared_ptr<TaskNode> writeSPHStateToVtk = std::make_shared<TaskNode>([&]() {
        fluidObj->getDynamicalSPHModel()->writeStateToVtk();
        }, "WriteStateToVtk");
      taskGraph->insertAfter(fluidObj->getDynamicalSPHModel()->getMoveParticlesNode(), writeSPHStateToVtk);

      // This node colors the fluid points based on their type
	  std::shared_ptr<TaskNode> computeVelocityScalars = std::make_shared<TaskNode>([&]() {
          const std::shared_ptr<SPHBoundaryConditions> sphBoundaryConditions = sphModel->getBoundaryConditions();
          StdVectorOfReal& scalars = *scalarsPtr;
          for (size_t i = 0; i < sphModel->getCurrentState()->getNumParticles(); i++)
          {
            if (sphBoundaryConditions->getParticleTypes()[i] == SPHBoundaryConditions::ParticleType::wall)
            {
              scalars[i] = 0;
            }
            else if (sphBoundaryConditions->getParticleTypes()[i] == SPHBoundaryConditions::ParticleType::inlet)
            {
              scalars[i] = 1;
            }
            else if (sphBoundaryConditions->getParticleTypes()[i] == SPHBoundaryConditions::ParticleType::outlet)
            {
              scalars[i] = 2;
            }
            else
            {
              scalars[i] = 3;
            }
		  }
	  }, "ComputeVelocityScalars");
      taskGraph->insertAfter(fluidObj->getUpdateGeometryNode(), computeVelocityScalars);
    });

  simManager->setActiveScene(scene);

  simManager->start(SimulationStatus::Paused);

  return 0;
}
