/**
**    Hatchit Engine
**    Copyright(c) 2015 ThirdDegree
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

#include <ht_d3d12pipeline.h>

namespace Hatchit {

    namespace Graphics {

        namespace DX
        {
            D3D12Pipeline::D3D12Pipeline(ID3D12Device* device, ID3D12RootSignature* rootSignature)
            {
                m_device = device;
                m_rootSignature = rootSignature;
            }
            
            void D3D12Pipeline::VSetMultisampleState(const Resource::Pipeline::MultisampleState& msState)
            {

            }

            bool D3D12Pipeline::VInitialize(const Resource::PipelineHandle handle)
            {
                HRESULT hr = S_OK;

                m_description = {};
                
                /*Build Input Layout*/
                std::vector<D3D12_INPUT_ELEMENT_DESC> _elements;
                const Resource::Pipeline::InputLayout& _layout = handle->GetInputLayout();
                for (int i = 0; i < _layout.elements.size(); i++)
                {
                    Resource::Pipeline::InputElement element = _layout.elements[i];

                    D3D12_INPUT_ELEMENT_DESC elementDesc;
                    elementDesc.SemanticName = element.semanticName.c_str();
                    elementDesc.SemanticIndex = element.semanticIndex;
                    elementDesc.InputSlot = element.slot;
                    elementDesc.Format = InputFormatFromElement(element);
                    elementDesc.AlignedByteOffset = (i == 0) ? 0 : D3D12_APPEND_ALIGNED_ELEMENT;
                    elementDesc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
                    elementDesc.InstanceDataStepRate = 0;

                    _elements.push_back(elementDesc);
                }

                D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
                inputLayoutDesc.pInputElementDescs = &_elements[0];
                inputLayoutDesc.NumElements = _elements.size();

                /*Build Pipeline State Object*/
                m_description.InputLayout = inputLayoutDesc;
                m_description.pRootSignature = m_rootSignature;
                m_description.VS = ShaderBytecodeFromHandle(Resource::Pipeline::VERTEX, handle);
                m_description.GS = ShaderBytecodeFromHandle(Resource::Pipeline::GEOMETRY, handle);
                m_description.HS = ShaderBytecodeFromHandle(Resource::Pipeline::TESS_CONTROL, handle);
                m_description.DS = ShaderBytecodeFromHandle(Resource::Pipeline::TESS_EVAL, handle);
                m_description.PS = ShaderBytecodeFromHandle(Resource::Pipeline::FRAGMENT, handle);
                m_description.RasterizerState = RasterDescFromHandle(handle);
                m_description.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
                m_description.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
                m_description.SampleMask = UINT_MAX;
                m_description.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
                m_description.NumRenderTargets = 1;
                m_description.RTVFormats[0] = DXGI_FORMAT_B8G8R8A8_UNORM;
                m_description.DSVFormat = DXGI_FORMAT_D32_FLOAT;
                m_description.SampleDesc.Count = 1;

                hr = m_device->CreateGraphicsPipelineState(&m_description, IID_PPV_ARGS(&m_pipelineState));
                if (FAILED(hr))
                {
                    HT_DEBUG_PRINTF("Failed to create pipeline state object.\n");
                    return false;
                }

                return true;
            }

            void D3D12Pipeline::VSetRasterState(const Resource::Pipeline::RasterizerState & rasterState)
            {
                
            }

            void D3D12Pipeline::VLoadShader(Resource::Pipeline::ShaderSlot shaderSlot, IShaderHandle shader)
            {
				
            }

            bool D3D12Pipeline::VPrepare()
            {
                
                return false;
            }

            D3D12_RASTERIZER_DESC D3D12Pipeline::RasterDescFromHandle(const Resource::PipelineHandle& handle)
            {
                D3D12_RASTERIZER_DESC desc;
                Resource::Pipeline::RasterizerState rasterState = handle->GetRasterizationState();

                //Handle cullmode
                switch (rasterState.cullMode)
                {
                case Resource::Pipeline::CullMode::BACK:
                    desc.CullMode = D3D12_CULL_MODE_BACK;
                    break;
                case Resource::Pipeline::CullMode::FRONT:
                    desc.CullMode = D3D12_CULL_MODE_FRONT;
                    break;
                case Resource::Pipeline::CullMode::NONE:
                    desc.CullMode = D3D12_CULL_MODE_NONE;
                    break;
                }

                //Handle polygon mode
                switch (rasterState.polygonMode)
                {
                case Resource::Pipeline::PolygonMode::SOLID:
                    desc.FillMode = D3D12_FILL_MODE_SOLID;
                    break;
                case Resource::Pipeline::PolygonMode::LINE:
                    desc.FillMode = D3D12_FILL_MODE_WIREFRAME;
                    break;
                }

                desc.DepthClipEnable = rasterState.depthClampEnable;
                desc.FrontCounterClockwise = rasterState.frontCounterClockwise;
                desc.DepthBias = 0;
                desc.DepthBiasClamp = 0.0f;
                desc.SlopeScaledDepthBias = 0.0f;
                desc.MultisampleEnable = false;
                desc.AntialiasedLineEnable = false;
                desc.ForcedSampleCount = 0;
                desc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

                return desc;
            }

            D3D12_SHADER_BYTECODE D3D12Pipeline::ShaderBytecodeFromHandle(Resource::Pipeline::ShaderSlot slot, const Resource::PipelineHandle & handle)
            {
                std::map<Resource::Pipeline::ShaderSlot, std::string> paths = handle->GetCSOShaderPaths();
                std::map<Resource::Pipeline::ShaderSlot, Resource::ShaderHandle> handles = handle->GetCSOShaderHandles();

                D3D12ShaderHandle _handle = D3D12Shader::GetHandle("D3D12"+paths[slot]);
                if (_handle.IsValid())
                {
                    if (!_handle->IsInitialized())
                       _handle->VInitFromResource(handles[slot]);

                    m_shaders[slot] = _handle;

                    return _handle->GetBytecode();
                }
                else
                {
                    return CD3DX12_SHADER_BYTECODE();
                }
            }

            D3D12_INPUT_LAYOUT_DESC D3D12Pipeline::InputLayoutDescFromHandle(const Resource::PipelineHandle& handle)
            {
                std::vector<D3D12_INPUT_ELEMENT_DESC> _elements;
                const Resource::Pipeline::InputLayout& _layout = handle->GetInputLayout();
                for (int i = 0; i < _layout.elements.size(); i++)
                {
                    Resource::Pipeline::InputElement element = _layout.elements[i];

                    D3D12_INPUT_ELEMENT_DESC elementDesc;
                    elementDesc.SemanticName = element.semanticName.c_str();
                    elementDesc.SemanticIndex = element.semanticIndex;
                    elementDesc.InputSlot = element.slot;
                    elementDesc.Format = InputFormatFromElement(element);
                    elementDesc.AlignedByteOffset = (i == 0) ? 0 : D3D12_APPEND_ALIGNED_ELEMENT;
                    elementDesc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
                    elementDesc.InstanceDataStepRate = 0;

                    _elements.push_back(elementDesc);
                }

                D3D12_INPUT_LAYOUT_DESC desc;
                desc.pInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[3];
                return D3D12_INPUT_LAYOUT_DESC();
            }

            DXGI_FORMAT D3D12Pipeline::InputFormatFromElement(const Resource::Pipeline::InputElement& element)
            {
                switch (element.format)
                {
                case Resource::Pipeline::InputElement::Format::R32G32B32_FLOAT:
                    return DXGI_FORMAT_R32G32B32_FLOAT;
                case Resource::Pipeline::InputElement::Format::R32G32B32A32_FLOAT:
                    return DXGI_FORMAT_R32G32B32A32_FLOAT;
                default:
                    return DXGI_FORMAT_UNKNOWN;
                }
            }
        }
    }
}