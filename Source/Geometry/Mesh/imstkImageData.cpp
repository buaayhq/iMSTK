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

#include "imstkImageData.h"

namespace imstk
{
void
ImageData::print() const
{
    Geometry::print();
}

double
ImageData::getVolume() const
{
    if (!this->m_data)
    {
        return 0.0;
    }

    double bounds[6];
    this->m_data->GetBounds(bounds);
    return ((bounds[1] - bounds[0]) *
            (bounds[3] - bounds[2]) *
            (bounds[5] - bounds[4]));
}

void
ImageData::initialize(vtkImageData* im)
{
    this->clear();
    if (im)
    {
        this->m_data = vtkSmartPointer<vtkImageData>::New();
        this->m_data->DeepCopy(im);
    }
}

void
ImageData::clear()
{
    if (this->m_data)
    {
        this->m_data = nullptr;
    }
}

vtkImageData*
ImageData::getData()
{
    return this->m_data;
}
} // imstk
