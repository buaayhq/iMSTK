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

#include "imstkSceneObjectController.h"
#include "imstkDeviceTracker.h"
#include "imstkSceneObject.h"
#include "imstkDeviceClient.h"
#include "imstkCollidingObject.h"
#include "imstkGeometry.h"

#include <utility>

#include <g3log/g3log.hpp>

namespace imstk
{
SceneObjectController::SceneObjectController(std::shared_ptr<SceneObject> sceneObject, std::shared_ptr<DeviceTracker> trackingController) :
    m_trackingController(trackingController), m_sceneObject(sceneObject)
{
}

void
SceneObjectController::updateControlledObjects()
{
    if (!m_trackingController->isTrackerUpToDate())
    {
        if (!m_trackingController->updateTrackingData())
        {
            LOG(WARNING) << "SceneObjectController::updateControlledObjects warning: could not update tracking info.";
            return;
        }
    }

    if (m_updateCallback)
    {
        m_updateCallback(this);
    }

    // Update colliding geometry
    m_sceneObject->getMasterGeometry()->setTranslation(m_trackingController->getPosition());
    m_sceneObject->getMasterGeometry()->setRotation(m_trackingController->getRotation());
}

void
SceneObjectController::applyForces()
{
    if (auto collidingObject = dynamic_cast<CollidingObject*>(m_sceneObject.get()))
    {
        m_trackingController->getDeviceClient()->setForce(collidingObject->getForce());
    }
}

void
SceneObjectController::setTrackerToOutOfDate()
{
    m_trackingController->setTrackerToOutOfDate();
}
} // imstk
