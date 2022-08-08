/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkPbdConstraint.h"

namespace imstk
{
///
/// \class PbdContactConstraint
///
/// \brief A constraint on a rigid body that defines rotationl correction through
/// dx applied at a local position r on the body.
///
/// It is primarily useful for rigid vs rigid and rigid vs deformable collision
/// but can be used for other purposes such as joints.
///
class PbdContactConstraint : public PbdConstraint
{
protected:
    PbdContactConstraint(const int numParticles) :
        PbdConstraint(numParticles), m_r(numParticles)
    {
    }

public:
    ~PbdContactConstraint() override = default;

    ///
    /// \brief Update positions by projecting constraints.
    ///
    void projectConstraint(PbdState& bodies,
                           const double dt, const SolverType& type) override;

    ///
    /// \brief Solve the velocities given to the constraint
    ///
    void correctVelocity(PbdState& bodies, const double dt) override;

protected:
    std::vector<Vec3d> m_r;
};

///
/// \class PbdTriangleToBodyConstraint
///
/// \brief Resolves a point on body to a triangle with linear and angular movement
///
class PbdTriangleToBodyConstraint : public PbdContactConstraint
{
public:
    PbdTriangleToBodyConstraint() : PbdContactConstraint(4) { }

    ///
    /// \brief Initialize the constraint
    /// \param PbdState of bodies
    /// \param Body particle id
    /// \param Global position on the particle body (local computed from it)
    /// \param Point0 of triangle
    /// \param Point1 of triangle
    /// \param Point2 of triangle
    ///
    void initConstraint(
        const PbdState& state,
        const PbdParticleId& bodyId,
        const Vec3d contactPtOnBody,
        const PbdParticleId& x0, const PbdParticleId& x1, const PbdParticleId& x2,
        const double compliance = 0.0)
    {
        m_particles[0] = bodyId;
        // Compute local position on body
        m_r[0] = contactPtOnBody - state.getPosition(bodyId);
        m_particles[1] = x0;
        m_particles[2] = x1;
        m_particles[3] = x2;

        setCompliance(compliance);
    }

    bool computeValueAndGradient(PbdState&           bodies,
                                 double&             c,
                                 std::vector<Vec3d>& n) override;
};

///
/// \class PbdPointToBodyConstraint
///
/// \brief Resolves a point on body to a vertex with linear and angular movement
///
class PbdVertexToBodyConstraint : public PbdContactConstraint
{
public:
    PbdVertexToBodyConstraint() : PbdContactConstraint(2) { }

    ///
    /// \brief Initialize the constraint
    /// \param PbdState of bodies
    /// \param Body particle id
    /// \param Global position on the particle body (local computed from it)
    /// \param Point0 of vertex
    ///
    void initConstraint(
        const PbdState&      state,
        const PbdParticleId& bodyId,
        const Vec3d          contactPtOnBody,
        const PbdParticleId& x0,
        const double         compliance = 0.0)
    {
        m_particles[0] = bodyId;
        // Compute local position on body
        m_r[0] = contactPtOnBody - state.getPosition(bodyId);
        m_particles[1] = x0;

        // Infinite stiffness/completely rigid
        setCompliance(compliance);
    }

    bool computeValueAndGradient(PbdState&           bodies,
                                 double&             c,
                                 std::vector<Vec3d>& n) override;
};

///
/// \class PbdEdgeToBodyConstraint
///
/// \brief Resolves a point on body to an edge with linear and angular movement
///
class PbdEdgeToBodyConstraint : public PbdContactConstraint
{
public:
    PbdEdgeToBodyConstraint() : PbdContactConstraint(3) { }

    ///
    /// \brief Initialize the constraint
    /// \param PbdState of bodies
    /// \param Body particle id
    /// \param Global position on the particle body (local computed from it)
    /// \param Point0 of edge
    /// \param Point1 of edge
    ///
    void initConstraint(
        const PbdState& state,
        const PbdParticleId& bodyId,
        const Vec3d contactPtOnBody,
        const PbdParticleId& x0, const PbdParticleId& x1,
        const double compliance = 0.0)
    {
        m_particles[0] = bodyId;
        // Compute local position on body
        m_r[0] = contactPtOnBody - state.getPosition(bodyId);
        m_particles[1] = x0;
        m_particles[2] = x1;

        setCompliance(compliance);
    }

    bool computeValueAndGradient(PbdState&           bodies,
                                 double&             c,
                                 std::vector<Vec3d>& n) override;
};

///
/// \class PbdBodyToBodyConstraint
///
/// \brief Resolves contact with contact plane reprojection between two bodies
/// Resolves distance between two points on two bodies by given direction
///
class PbdBodyToBodyConstraint : public PbdContactConstraint
{
public:
    PbdBodyToBodyConstraint() : PbdContactConstraint(2) { }

    void initConstraint(
        const PbdState&      state,
        const PbdParticleId& bodyId0,
        const Vec3d&         contactPtOnBody0,
        const Vec3d&         contactNormal,
        const PbdParticleId& bodyId1,
        const Vec3d          contactPtOnBody1,
        const double         compliance = 0.0)
    {
        m_particles[0] = bodyId0;
        // Compute local position on body
        m_r[0] = contactPtOnBody0 - state.getPosition(bodyId0);
        m_particles[1] = bodyId1;
        m_r[1] = contactPtOnBody1 - state.getPosition(bodyId1);

        m_contactNormal = contactNormal.normalized();

        setCompliance(compliance);
    }

    bool computeValueAndGradient(PbdState&           bodies,
                                 double&             c,
                                 std::vector<Vec3d>& n) override;

protected:
    Vec3d m_contactNormal = Vec3d::Zero();
};

///
/// \class PbdBodyToBodyDistanceConstraint
///
/// \brief Constrain two locally defined points on each body by a given distance
///
class PbdBodyToBodyDistanceConstraint : public PbdContactConstraint
{
public:
    PbdBodyToBodyDistanceConstraint() : PbdContactConstraint(2) { }

    ///
    /// \brief Initialize the constraint
    /// ptOnBody is global position
    ///
    void initConstraint(
        const PbdState&      state,
        const PbdParticleId& bodyId0,
        const Vec3d          ptOnBody0,
        const PbdParticleId& bodyId1,
        const Vec3d          ptOnBody1,
        const double         restLength,
        const double         compliance)
    {
        m_particles[0] = bodyId0;
        // Compute local position on body in rest pose
        m_r[0]      = ptOnBody0 - state.getPosition(bodyId0);
        m_rest_r[0] = state.getOrientation(bodyId0).inverse()._transformVector(m_r[0]);

        m_particles[1] = bodyId1;
        // Compute local position on body
        m_r[1]      = ptOnBody1 - state.getPosition(bodyId1);
        m_rest_r[1] = state.getOrientation(bodyId1).inverse()._transformVector(m_r[1]);

        m_restLength = restLength;

        setCompliance(compliance);
    }

    ///
    /// \brief Initialize the constraint
    /// ptOnBody is global position
    /// restLength is set to the initial distance between these two points
    ///
    void initConstraint(
        const PbdState&      state,
        const PbdParticleId& bodyId0,
        const Vec3d          ptOnBody0,
        const PbdParticleId& bodyId1,
        const Vec3d          ptOnBody1,
        const double         compliance)
    {
        initConstraint(state, bodyId0, ptOnBody0, bodyId1, ptOnBody1,
            (ptOnBody1 - ptOnBody0).norm(), compliance);
    }

    bool computeValueAndGradient(PbdState&           bodies,
                                 double&             c,
                                 std::vector<Vec3d>& n) override;

protected:
    std::array<Vec3d, 2> m_rest_r = { Vec3d::Zero(), Vec3d::Zero() }; // In rest pose
    double m_restLength = 0.0;
};
} // namespace imstk