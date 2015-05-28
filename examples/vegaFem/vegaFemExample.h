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

#include "smCore/smConfig.h"
#include "smCore/smErrorLog.h"
#include "smCore/smCoreClass.h"
#include "smCore/smSDK.h"
#include "smRendering/smViewer.h"
#include "smSimulators/smVegaFemSimulator.h"
#include "smSimulators/smVegaFemSceneObject.h"

class vegaFemExample : public smSimulationMain
{

public:
    vegaFemExample();

    virtual ~vegaFemExample(){}
    void simulateMain(const smSimulationMainParam &){}
private:
    std::shared_ptr<smSDK> sdk;
    std::shared_ptr<smVegaFemSceneObject> femobj;
    std::shared_ptr<smVegaFemSimulator> femSim;
    smMatrix33d mat;
    std::shared_ptr<smSimulator> simulator;
   std::shared_ptr<smViewer> viewer;
    std::shared_ptr<smScene> scene1;
};

void VegaFemExample();
