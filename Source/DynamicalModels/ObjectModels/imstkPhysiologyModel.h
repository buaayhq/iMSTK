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

#pragma once

#include "imstkDynamicalModel.h"
#include "imstkPhysiologyState.h"
#include "imstkSPHKernels.h"
#include "imstkNeighborSearch.h"

class PhysiologyEngine;
class HowToTracker;
class SELiquidCompartment;

namespace imstk
{
class ComputeNode;
class PointSet;

///
/// \class SPHModelConfig
/// \brief Class that holds the SPH model parameters
///
class PhysiologyModelConfig
{
private:
    void initialize();

public:
    explicit PhysiologyModelConfig();

    // place all variables that are needed to initialize Pulse here
    Real m_pulseTimeStep = Real(1e-6);
    Real m_minTimestep = Real(1e-3);
    Real m_maxTimestep = Real(1e-3);

    
};

///
/// \class SPHModel
/// \brief SPH fluid model
///
class PhysiologyModel : public DynamicalModel<PhysiologyState>
{
public:
  const SELiquidCompartment* m_femoralCompartment;
  double m_femoralFlowRate;

    ///
    /// \brief Constructor
    ///
    PhysiologyModel();

    ///
    /// \brief Destructor
    ///
    virtual ~PhysiologyModel() override = default;

    ///
    /// \brief Set simulation parameters
    ///
    void configure(const std::shared_ptr<PhysiologyModelConfig>& params) { m_modelParameters = params; }

    ///
    /// \brief Initialize the dynamical model
    ///
    virtual bool initialize() override;

//    ///
//    /// \brief Update states
//    ///
    virtual void updateBodyStates(const Vectord&, const StateUpdateType) override {}
//
//    ///
//    /// \brief Update positions of point set geometry
//    ///
//    virtual void updatePhysicsGeometry() override;
//
//    ///
//    /// \brief Reset the current state to the initial state
//    ///
    //virtual void resetToInitialState() override
    //{ this->m_currentState->setState(this->m_initialState); }
//
//    ///
//    /// \brief Get the simulation parameters
//    ///
//    //const std::shared_ptr<SPHModelConfig>& getParameters() const
//    //{ assert(m_modelParameters); return m_modelParameters; }
//
//    ///
//    /// \brief Get the kinematics particle data (positions + velocities)
//    ///
//    SPHKinematicState& getKinematicsState()
//    {
//        assert(this->m_currentState);
//        return *this->m_currentState;
//    }
//
//    const SPHKinematicState& getKinematicsState() const
//    {
//        assert(this->m_currentState);
//        return *this->m_currentState;
//    }
//
//    ///
//    /// \brief Get particle simulation data
//    ///
//    SPHSimulationState& getState() { return m_simulationState; }
//    const SPHSimulationState& getState() const { return m_simulationState; }
//
//    ///
//    /// \brief Set the default time step size,
//    /// valid only if using a fixed time step for integration
//    ///
    virtual void setTimeStep(const double timeStep) override
    { setDefaultTimeStep(static_cast<Real>(timeStep)); }
//
//    ///
//    /// \brief Set the default time step size,
//    /// valid only if using a fixed time step for integration
//    ///
    void setDefaultTimeStep(const Real timeStep)
    { m_defaultDt = static_cast<Real>(timeStep); }
//
//    ///
//    /// \brief Returns the time step size
//    ///
    virtual double getTimeStep() const override
    { return static_cast<double>(m_dt); }
//
//    void setInitialVelocities(StdVectorOfVec3d& initialVelocities);
//    StdVectorOfVec3d getInitialVelocities();
//    void periodicBCs(const size_t p);
//
//    std::shared_ptr<TaskNode> getFindParticleNeighborsNode() const { return m_findParticleNeighborsNode; }
//    std::shared_ptr<TaskNode> getComputeDensityNode() const { return m_computeDensityNode; }
//    std::shared_ptr<TaskNode> getComputePressureNode() const { return m_computePressureAccelNode; }
//    std::shared_ptr<TaskNode> getComputeSurfaceTensionNode() const { return m_computeSurfaceTensionNode; }
//    std::shared_ptr<TaskNode> getComputeTimeStepSizeNode() const { m_computeTimeStepSizeNode; }
//    std::shared_ptr<TaskNode> getSumAccelsNode() const { m_sumAccelsNode; }
//    std::shared_ptr<TaskNode> getIntegrateNode() const { m_integrateNode; }
    std::shared_ptr<TaskNode> getSolveNode() const { return m_solveNode; }


//
protected:
//    ///
//    /// \brief Setup SPH compute graph connectivity
//    ///
    virtual void initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink) override;
//
//private:
//    ///
//    /// \brief Compute time step size, do nothing if using a fixed time step size for integration
//    ///
//    void computeTimeStepSize();
//
//    ///
//    /// \brief Compute time step size based on CFL condition
//    ///
//    Real computeCFLTimeStepSize();
//
//    ///
//    /// \brief Find the neighbors for each particle
//    ///
//    void findParticleNeighbors();
//
//    ///
//    /// \brief Pre-compute relative positions with neighbor particles
//    ///
//    void computeNeighborRelativePositions();
//
//    ///
//    /// \brief Collect the densities of neighbor particles,
//    /// called after all density computation (after density normalization, if applicable)
//    ///
//    void collectNeighborDensity();
//
//    ///
//    /// \brief Compute particle densities
//    ///
//    void computeDensity();
//
//    ///
//    /// \brief Normalize densities, producing smoother density field
//    ///
//    void normalizeDensity();
//
//    ///
//    /// \brief Compute particle acceleration due to pressure
//    ///
//    void computePressureAcceleration();
//
//    ///
//    /// \brief Sum the forces computed in parallel
//    ///
//    void sumAccels();
//
//    ///
//    /// \brief Update particle velocities due to pressure
//    ///
//    void updateVelocity(const Real timestep);
//
//    void updateVelocityNoGravity(Real timestep);
//
//    ///
//    /// \brief Compute viscosity
//    ///
//    void computeViscosity();
//
//    ///
//    /// \brief Compute surface tension and update velocities
//    /// Compute surface tension using Akinci et at. 2013 model
//    /// (Versatile Surface Tension and Adhesion for SPH Fluids)
//    ///
//    void computeSurfaceTension();
//
//    ///
//    /// \brief Move particles
//    ///
//    void moveParticles(const Real timestep);

    void solvePulse();


//
//
//protected:
//    std::shared_ptr<TaskNode> m_findParticleNeighborsNode = nullptr;
//    std::shared_ptr<TaskNode> m_computeDensityNode        = nullptr;
//    std::shared_ptr<TaskNode> m_computePressureAccelNode  = nullptr;
//    std::shared_ptr<TaskNode> m_computeSurfaceTensionNode = nullptr;
//    std::shared_ptr<TaskNode> m_computeTimeStepSizeNode   = nullptr;
//    std::shared_ptr<TaskNode> m_sumAccelsNode = nullptr;
//    std::shared_ptr<TaskNode> m_integrateNode = nullptr;
//
    std::shared_ptr<TaskNode> m_solveNode = nullptr;
private:
//    std::shared_ptr<PointSet> m_pointSetGeometry;
//    SPHSimulationState m_simulationState;
//
      Real m_dt;                                          ///> time step size
      Real m_defaultDt;                                   ///> default time step size
//
//    SPHSimulationKernels m_kernels;                     ///> SPH kernels (must be initialized during model initialization)
    std::shared_ptr<PhysiologyModelConfig> m_modelParameters;  ///> SPH Model parameters (must be set before simulation)
//    std::shared_ptr<NeighborSearch> m_neighborSearcher; ///> Neighbor Search (must be initialized during model initialization)
//
//    std::shared_ptr<StdVectorOfVec3d> m_pressureAccels       = nullptr;
//    std::shared_ptr<StdVectorOfVec3d> m_surfaceTensionAccels = nullptr;
//
//    StdVectorOfVec3d m_initialVelocities;

    std::unique_ptr<PhysiologyEngine> m_pulseObj = nullptr;
    double m_dT_s;

};
} // end namespace imstk
