/**
**    Hatchit Engine
**    Copyright(c) 2015-2016 Third-Degree
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
#include <ht_refcounted.h>
#include <ht_renderpass.h>

namespace Hatchit
{
    namespace Graphics
    {
        namespace DX
        {
            class HT_API D3D12RenderPass : public Core::RefCounted<D3D12RenderPass>, public RenderPassBase
            {
            public:
                D3D12RenderPass(Core::Guid ID);

                bool Initialize(const std::string& fileName);

                // Inherited via RenderPassBase
                virtual void VUpdate() override;

                virtual bool VBuildCommandList() override;

            private:

            };
        }
    }
}