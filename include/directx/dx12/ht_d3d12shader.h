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
#include <ht_directx.h>
#include <ht_shader.h>
#include <unordered_map>

namespace Hatchit {
    
    namespace Graphics {

        namespace DX
        {

            class HT_API D3D12Shader : public Core::RefCounted<D3D12Shader>, public IShader
            {
            public:
                D3D12Shader(Core::Guid ID);
                D3D12Shader(D3D12Shader&&) = default;
                ~D3D12Shader();

                bool Initialize(const std::string& fileName);

                D3D12_SHADER_BYTECODE GetBytecode();

            private:
                ID3DBlob*                   m_blob;
              
            };
            using D3D12ShaderHandle = Core::Handle<D3D12Shader>;
        }
    }
}