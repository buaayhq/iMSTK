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

#include "imstkDynamicObject.h"
#include "imstkMacros.h"
#include "imstkMath.h"

namespace imstk
{
class FemDeformableBodyModel;

///
/// \class FeDeformableObject
///
/// \brief Scene objects that can deform
///
class FeDeformableObject : public DynamicObject
{
public:
    FeDeformableObject(const std::string& name = "FeDeformableObject") : DynamicObject(name) { }
    ~FeDeformableObject() override = default;

    IMSTK_TYPE_NAME(FeDeformableObject)

    ///
    /// \brief Initialize the deformable object
    ///
    bool initialize() override;

    std::shared_ptr<FemDeformableBodyModel> getFEMModel();

protected:
    std::shared_ptr<FemDeformableBodyModel> m_femModel = nullptr;
};
} // namespace imstk