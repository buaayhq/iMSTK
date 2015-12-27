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

#include "BackwarEuler.h"

void BackwardEuler::solve(const OdeSystemState &state, OdeSystemState &newState, double timeStep)
{
    if(!this->system)
    {
        return;
    }

    this->computeSystemRHS(state,newState,timeStep);
    auto G = [&,this](const core::Vectord &) -> core::Vectord&
    {
        return this->rhs;
    };

    auto DG = [&,this](const core::Vectord &) -> const core::SparseMatrixd&
    {
        this->computeSystemMatrix(newState,timeStep);
        return this->systemMatrix;
    };

    auto updateIterate = [](const core::Vectord &dv, core::Vectord &v)
    {
        v += dv;
        newState.getPositions() = state.getPositions() + timeStep*v;
    };

    auto NewtonSolver = std::make_shared<InexactNewton>();

    newState = state;

    NewtonSolver->setSystem(G);
    NewtonSolver->setJacobian(DG);
    NewtonSolver->solve(newState.getVelocities());
}

//---------------------------------------------------------------------------
BackwardEuler::SystemMatrixType BackwardEuler::getSystemMatrix()
{
    if(!this->system)
    {
        return;
    }

    auto DG = [ &, this](const core::Vectord &x) -> void
    {
        auto &C = this->system->evalDFv(x);
        auto &K = this->system->evalDFx(x);

    };
}
