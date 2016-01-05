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

#include "TimeIntegrator.h"

TimeIntegrator::TimeIntegrator(std::shared_ptr< OdeSystem > odeSystem): system(odeSystem)
{}

//---------------------------------------------------------------------------
std::shared_ptr< OdeSystem > TimeIntegrator::getSystem() const
{
    return this->system;
}

//---------------------------------------------------------------------------
void TimeIntegrator::setSystem(std::shared_ptr< OdeSystem > newSystem)
{
    this->system = newSystem;
}

//---------------------------------------------------------------------------
void TimeIntegrator::setSystemMatrix(const core::SparseMatrixd &newSystemMatrix)
{
    this->systemMatrix = newSystemMatrix;
}

//---------------------------------------------------------------------------
const core::SparseMatrixd &TimeIntegrator::getSystemMatrix() const
{
    return this->systemMatrix;
}
