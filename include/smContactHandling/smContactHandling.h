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

#ifndef SMCONTACTHANDLING_H
#define SMCONTACTHANDLING_H

// STL includes
#include <memory>

// SimMedTK includes
#include "smCore/smCoreClass.h"
#include "smContactHandling/smContactConfig.h"
#include "smCore/smSceneObject.h"

class smMesh;
class smCollisionPair;

enum smContactHandlingType
{
	SIMMEDTK_CONTACT_PENALTY_UNILATERAL,
	SIMMEDTK_CONTACT_PENALTY_BILATERAL,
	SIMMEDTK_CONTACT_PBD_UNILATERAL,
	SIMMEDTK_CONTACT_PBD_BILATERAL,
	SIMMEDTK_CONTACT_LPC_UNILATERAL,
	SIMMEDTK_CONTACT_LCP_UNILATERAL,
	SIMMEDTK_CONTACT_LCP_BILATERAL
};

/// \brief Base class to for handling contact for collision response
/// Input: collision pairs containig the collision information
/// Output: contact forces or Jacobians or projection/PBD constraints
class smContactHandling: public smCoreClass
{
public:
    
    
    smContactHandling() {};

    virtual ~smContactHandling() {};

    ///
    /// @brief Set the scene objects that are colliding
    ///
    void setSceneObjects(const std::shared_ptr< smSceneObject >& first,
                        const std::shared_ptr< smSceneObject >& second);

    ///
    /// @brief Set the scene objects that are colliding
    ///
    void setCollisionPairs(const std::shared_ptr< smCollisionPair >& colPair);

    ///
    /// @brief Get how the contact between scene objects is processed
    ///
    std::shared_ptr<smCollisionPair> getCollisionPairs() const;

    ///
    /// @brief Get how the contact between scene objects is processed
    ///
    smContactHandlingType getContactHandlingType() const;

    ///
    /// @brief Get the first scene object
    ///
	std::shared_ptr<smSceneObject> getFirstSceneObject() const;
	
    ///
    /// @brief Get the second scene object
    ///
	std::shared_ptr<smSceneObject> getSecondSceneObject() const;

    ///
    /// @brief Implementation of how the contacts between colliding objects is resolved
    ///
    virtual void resolveContacts() = 0;
protected:

    smContactHandlingType contactHandlingType;

    std::pair<std::shared_ptr<smSceneObject>, std::shared_ptr<smSceneObject>> collidingSceneObjects;

    std::shared_ptr<smCollisionPair> collisionPairs;


};

#endif //SMCONTACTHANDLING_H
