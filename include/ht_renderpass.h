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

/**
* \class RenderPass
* \ingroup HatchitGraphics
*
* \brief A wrapper over a RenderPassBase pointer that will be mapped
* to an appropriate render pass object based on the renderer.
*/

#pragma once

#include <ht_platform.h>
#include <ht_rendertarget.h>
#include <ht_pipeline.h>
#include <ht_mesh.h>
#include <ht_material.h>
#include <ht_math.h>
#include <ht_color.h>
#include <ht_renderpass_resource.h>
#include <ht_shadervariable.h>

namespace Hatchit {

    namespace Graphics {

        class HT_API RenderPass : public Core::RefCounted<RenderPass>
        {
        public:
            RenderPass(Core::Guid ID);
            virtual ~RenderPass() { };

            bool BuildCommandList();

            void SetView(Math::Matrix4 view);
            void SetProj(Math::Matrix4 proj);

            void ScheduleRenderRequest(MaterialHandle material, MeshHandle mesh, std::vector<Resource::ShaderVariable*> instanceVariables);

            uint64_t GetLayerFlags();

        private:
            RenderPassBase* m_base;
        };

        using RenderPassHandle = Core::Handle<IRenderPass>;
    }
}
