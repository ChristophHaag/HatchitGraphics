/**
**    Hatchit Engine
**    Copyright(c) 2015 Third-Degree
**
**    GNU Lesser General Public License
**    This file may be used under the terms of the GNU Lesser
**    General Public License version 3 as published by the Free
**    Software Foundation and appearing in the file LICENSE.LGPLv3 included
**    in the packaging of this file. Please review the following information
**    to ensure the GNU Lesser General Public License requirements
**    will be met: https://www.gnu.org/licenses/lgpl.html
**
**/

#pragma once

#include <ht_platform.h>
#include <ht_material.h>
#include <ht_rendertarget.h>
#include <ht_math.h>

namespace Hatchit {

    namespace Graphics {

        class HT_API IRenderPass
        {
        public:
			virtual ~IRenderPass() { };

            //Will this be sent the Objects that it needs to render?
            virtual void VRender() = 0;

        private:
            //We're going need to send this to the shader
            Math::Vector3 eyePoint;

            //Data representing *where* the objects will be viewed from in this pass
			Math::Matrix4 viewMatrix;
			Math::Matrix4 projectionMatrix;

            //Which material we will be rendering with
            IMaterial* material;

            //Output
            IRenderTarget* renderTarget;
        };
    }
}
