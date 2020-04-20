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

#include "imstkIterativeLinearSolver.h"

// Eigen includes
#include <Eigen/IterativeLinearSolvers>

namespace imstk
{
class LinearProjectionConstraint;
///
/// \brief Conjugate gradient sparse linear solver for SPD matrices
///
class ConjugateGradient : public IterativeLinearSolver
{
public:
    ///
    /// \brief Constructors/Destructor
    ///
    ConjugateGradient();
    ConjugateGradient(const SparseMatrixd& A, const Vectord& rhs);
    ~ConjugateGradient() = default;

    ///
    /// \brief Remove specific constructor signatures
    ///
    ConjugateGradient(const ConjugateGradient&) = delete;
    ConjugateGradient& operator=(const ConjugateGradient&) = delete;

    ///
    /// \brief Do one iteration of the method.
    ///
    //void iterate(Vectord& x, bool updateResidual = true) override {};

    ///
    /// \brief Solve the system of equations.
    ///
    void solve(Vectord& x) override;

    ///
    /// \brief Solve the linear system using Conjugate gradient iterations to a
    ///     specified tolerance.
    ///
    void solve(Vectord& x, const double tolerance);

    ///
    /// \brief Return the error calculated by the solver.
    ///
    double getResidual(const Vectord& x) override;

    ///
    /// \brief Sets the system. System of linear equations.
    ///
    void setSystem(std::shared_ptr<LinearSystemType> newSystem) override;

    ///
    /// \brief set/get the maximum number of iterations for the iterative solver.
    ///
    virtual void setMaxNumIterations(const size_t maxIter) override;

    ///
    /// \brief Set solver tolerance
    ///
    void setTolerance(const double tolerance);

    ///
    /// \brief Print solver information
    ///
    void print() const override;

    ///
    /// \brief Apply a filter to the vector supplied
    ///
    void applyLinearProjectionFilter(Vectord& x, const std::vector<LinearProjectionConstraint>& linProj, const bool setVal);

    ///
    /// \brief Get the vector denoting the filter
    ///
    void setLinearProjectors(std::vector<LinearProjectionConstraint>* f)
    {
        m_FixedLinearProjConstraints = f;
    }

    /// \brief Get the vector denoting the filter
    ///
    std::vector<LinearProjectionConstraint>& getLinearProjectors()
    {
        return *m_FixedLinearProjConstraints;
    }

    /// \brief Get the vector denoting the filter
    ///
    void setDynamicLinearProjectors(std::vector<LinearProjectionConstraint>* f)
    {
        m_DynamicLinearProjConstraints = f;
    }

    /// \brief Get the vector denoting the filter
    ///
    std::vector<LinearProjectionConstraint>& getDynamicLinearProjectors()
    {
        return *m_DynamicLinearProjConstraints;
    }

private:

    ///
    /// \brief Modified Conjugate gradient solver
    ///
    void modifiedCGSolve(Vectord& x);

    ///> Pointer to the Eigen's Conjugate gradient solver
    Eigen::ConjugateGradient<SparseMatrixd> m_cgSolver;

    std::vector<LinearProjectionConstraint>* m_FixedLinearProjConstraints   = nullptr;
    std::vector<LinearProjectionConstraint>* m_DynamicLinearProjConstraints = nullptr;
};
} // imstk
