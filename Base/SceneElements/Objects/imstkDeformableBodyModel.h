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

#ifndef imstkDeformableBodyModel_h
#define imstkDeformableBodyModel_h

#include <memory>

#include "Eigen/Sparse"

#include "imstkGeometry.h"
#include "imstkDynamicalModel.h"
#include "imstkTimeIntegrator.h"
#include "imstkInternalForceModel.h"
#include "imstkForceModelConfig.h"
#include "imstkProblemState.h"
#include "imstkNonlinearSystem.h"
#include "imstkVegaMeshReader.h"

//force models
#include "imstkStVKForceModel.h"
#include "imstkLinearFEMForceModel.h"
#include "imstkCorotationalFEMForceModel.h"
#include "imstkIsotropicHyperelasticFEMForceModel.h"
#include "imstkVegaMeshReader.h"

//vega
#include "sparseMatrix.h"

namespace imstk
{

///
/// \class DeformableBodyModel
///
/// \brief Mathematical model of the physics governing the dynamic deformable object
/// Note: Vega specifics will removed in future when the inertial and damping calculations
/// are done with in-house code
///
class DeformableBodyModel : public DynamicalModel
{
public:
    ///
    /// \brief Constructor
    ///
    DeformableBodyModel();

    ///
    /// \brief Destructor
    ///
    virtual ~DeformableBodyModel() = default;

    ///
    /// \brief Set/Get force model configuration
    ///
    void setForceModelConfiguration(std::shared_ptr<ForceModelConfig> fmConfig);
    std::shared_ptr<ForceModelConfig> getForceModelConfiguration() const;

    ///
    /// \brief Set/Get internal force model
    ///
    void setInternalForceModel(std::shared_ptr<InternalForceModel> fm);
    std::shared_ptr<InternalForceModel> getInternalForceModel() const;

    ///
    /// \brief Set/Get time integrator
    ///
    void setTimeIntegrator(std::shared_ptr<TimeIntegrator> timeIntegrator);
    std::shared_ptr<TimeIntegrator> getTimeIntegrator() const;

    ///
    /// \brief Set/Get the geometry used by force model
    ///
    void setModelGeometry(std::shared_ptr<Geometry> geometry);
    std::shared_ptr<Geometry> getModelGeometry();

    ///
    /// \brief Returns the tangent linear system given curent state
    ///
    void getTangent(Vectord& q);

    ///
    /// \brief Configure the force model from external file
    ///
    void configure(const std::string& configFileName);

    ///
    /// \brief Initialize the deformable body model
    ///
    void initialize();

    ///
    /// \brief Load the initial conditions of the deformable object
    ///
    void loadInitialStates();

    ///
    /// \brief Load the boundary conditions from external file
    ///
    void loadBoundaryConditions();

    ///
    /// \brief Initialize the force model
    ///
    void initializeForceModel();

    ///
    /// \brief Initialize the mass matrix from the mesh
    ///
    void initializeMassMatrix(const bool saveToDisk = false);

    ///
    /// \brief Initialize the damping (combines structural and viscous damping) matrix
    ///
    void initializeDampingMatrix();

    ///
    /// \brief Initialize the tangent stiffness matrix
    ///
    void initializeTangentStiffness();

    ///
    /// \brief Initialize the gravity force
    ///
    void initializeGravityForce();

    ///
    /// \brief Compute the RHS of the resulting linear system
    ///
    void computeImplicitSystemRHS(const kinematicState& prevState,
                                  kinematicState& newState);

    ///
    /// \brief Compute the LHS of the resulting linear system
    ///
    void computeImplicitSystemLHS(const kinematicState& prevState,
                                  kinematicState& newState);
    ///
    /// \brief Initialize explicit external forces
    ///
    void initializeExplicitExternalForces();

    ///
    /// \brief Update damping Matrix
    ///
    void updateDampingMatrix();

    ///
    /// \brief Update mass matrix
    /// Note: Not supported yet!
    ///
    void updateMassMatrix();

    ///
    /// \brief Updates the Physics geometry
    ///
    void updatePhysicsGeometry(const kinematicState& state);

    ///
    /// \brief Update states
    ///
    void updateBodyStates(const Vectord& delataV);

    ///
    /// \brief Returns the "function" that evaluates the nonlinear function given
    /// the state vector
    ///
    NonLinearSystem::VectorFunctionType getFunction(const Vectord& q);

    ///
    /// \brief Returns the "function" that evaluates the gradient of the nonlinear
    /// function given the state vector
    ///
    NonLinearSystem::MatrixFunctionType getFunctionGradient(const Vectord& q);


    ///
    /// \brief
    ///
    static void initializeEigenMatrixFromVegaMatrix(const vega::SparseMatrix& vegaMatrix, SparseMatrixd& eigenMatrix);

protected:
    std::shared_ptr<InternalForceModel> m_internalForceModel;       ///> Mathematical model for intenal forces
    std::shared_ptr<TimeIntegrator>     m_timeIntegrator;           ///> Time integrator

    std::shared_ptr<ForceModelConfig>   m_forceModelConfiguration;  ///> Store the configuration here
    std::shared_ptr<Geometry>           m_forceModelGeometry;       ///> Geometry used by force model

    bool m_damped;

    /// Matrices typical to a elastodynamics and 2nd order analogous systems
    SparseMatrixd m_M;    ///> Mass matrix
    SparseMatrixd m_C;    ///> Damping coefficient matrix
    SparseMatrixd m_K;    ///> Tangent (derivative of internal force w.r.t displacements) stiffness matrix
    SparseMatrixd m_Keff; ///> Effective stiffness matrix (dependent on internal force model and time integrator)

    std::shared_ptr<NonLinearSystem> m_nonLinearSystem; ///> Nonlinear system resulting from TI and force model

    Vectord m_Finternal;       ///> Vector of gravity forces
    Vectord m_Feff;       ///> Vector of gravity forces

    // External field forces
    Vectord m_gravityForce;   ///> Vector of gravity forces

    // Explicit external forces
    Vectord m_explicitExternalForce;   ///> Vector of explicitly defined external forces

    // Dirichlet boundary conditions
    std::vector<std::size_t> m_fixedNodeIds;

    std::shared_ptr<vega::VolumetricMesh> m_vegaPhysicsMesh;
    std::shared_ptr<vega::SparseMatrix> m_vegaMassMatrix;///> Vega mass matrix
    std::shared_ptr<vega::SparseMatrix> m_vegaTangentStiffnessMatrix;///> Vega Tangent stiffness matrix
    std::shared_ptr<vega::SparseMatrix> m_vegaDampingMatrix;///> Vega Laplacian damping matrix
};

} // imstk

#endif // ifndef imstkDeformableBodyModel_h
