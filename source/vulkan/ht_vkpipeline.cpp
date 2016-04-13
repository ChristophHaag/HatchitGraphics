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

#include <ht_vkpipeline.h>
#include <ht_vkrenderer.h>

#include <cassert>

namespace Hatchit {

    namespace Graphics {

        namespace Vulkan {

            using namespace Resource;

            VKPipeline::VKPipeline(std::string ID) :
                m_device(VKRenderer::RendererInstance->GetVKDevice()),

                Core::RefCounted<VKPipeline>(std::move(ID))
            {
            }

            VKPipeline::~VKPipeline() 
            {
                vkDestroyPipeline(m_device, m_pipeline, nullptr);
                vkDestroyPipelineCache(m_device, m_pipelineCache, nullptr);
            }

            bool VKPipeline::Initialize(const std::string& fileName)
            {
                Resource::PipelineHandle handle = Resource::Pipeline::GetHandleFromFileName(fileName);
                if (!handle.IsValid())
                {
                    return false;
                    HT_DEBUG_PRINTF("VKPipeline::VInitialize() ERROR: Handle was invalid");
                }

                setRasterState(handle->GetRasterizationState());
                setMultisampleState(handle->GetMultisampleState());

                VAddShaderVariables(handle->GetShaderVariables());

                //Load all shaders
                std::map<Pipeline::ShaderSlot, std::string> shaderPaths = handle->GetSPVShaderPaths();

                std::map<Pipeline::ShaderSlot, std::string>::iterator it;
                for (it = shaderPaths.begin(); it != shaderPaths.end(); it++)
                {
                    //Get the actual shader handle
                    VKShaderHandle shaderHandle = VKShader::GetHandle(it->second, it->second);

                    loadShader(it->first, shaderHandle.StaticCastHandle<IShader>());
                }

                //Get a handle to a compatible render pass
                std::string renderPassPath = handle->GetRenderPassPath();
                VKRenderPassHandle renderPass = VKRenderPass::GetHandle(renderPassPath, renderPassPath);

                if (!preparePipeline(renderPass))
                    return false;

                return true;

                return true;
            }

            bool VKPipeline::VAddShaderVariables(std::map<std::string, ShaderVariable*> shaderVariables)
            {
                std::map<std::string, ShaderVariable*>::iterator it;
                for (it = shaderVariables.begin(); it != shaderVariables.end(); it++)
                {
                    std::string name = it->first;
                    ShaderVariable* var = it->second;
                    
                    switch (var->GetType())
                    {
                    case ShaderVariable::INT:
                        VSetInt(name, *static_cast<int*>(var->GetData()));
                        break;
                    case ShaderVariable::DOUBLE:
                        VSetDouble(name, *static_cast<double*>(var->GetData()));
                        break;
                    case ShaderVariable::FLOAT:
                        VSetFloat(name, *static_cast<float*>(var->GetData()));
                        break;
                    case ShaderVariable::FLOAT2:
                        VSetFloat2(name, *static_cast<Math::Vector2 *>(var->GetData()));
                        break;
                    case ShaderVariable::FLOAT3:
                        VSetFloat3(name, *static_cast<Math::Vector3 *>(var->GetData()));
                        break;
                    case ShaderVariable::FLOAT4:
                        VSetFloat4(name, *static_cast<Math::Vector4 *>(var->GetData()));
                        break;
                    case ShaderVariable::MAT4:
                        VSetMatrix4(name, *static_cast<Math::Matrix4 *>(var->GetData()));
                        break;
                    }
                }

                return true;
            }

            bool VKPipeline::VSetInt(std::string name, int data)
            {
                //If the variable doesn't exist in the map lets allocate it
                //Otherwise lets just change its data
                std::map<std::string, ShaderVariable*>::iterator it = m_shaderVariables.find(name);
                if (it != m_shaderVariables.end())
                    static_cast<IntVariable*>(m_shaderVariables[name])->SetData(data);
                else
                    m_shaderVariables[name] = new IntVariable(data);

                return true;
            }
            bool VKPipeline::VSetDouble(std::string name, double data)
            {
                //If the variable doesn't exist in the map lets allocate it
                //Otherwise lets just change its data
                std::map<std::string, ShaderVariable*>::iterator it = m_shaderVariables.find(name);
                if (it != m_shaderVariables.end())
                    static_cast<DoubleVariable*>(m_shaderVariables[name])->SetData(data);
                else
                    m_shaderVariables[name] = new DoubleVariable(data);

                return true;
            }
            bool VKPipeline::VSetFloat(std::string name, float data)
            {
                //If the variable doesn't exist in the map lets allocate it
                //Otherwise lets just change its data
                std::map<std::string, ShaderVariable*>::iterator it = m_shaderVariables.find(name);
                if (it != m_shaderVariables.end())
                    static_cast<FloatVariable*>(m_shaderVariables[name])->SetData(data);
                else
                    m_shaderVariables[name] = new FloatVariable(data);

                return true;
            }
            bool VKPipeline::VSetFloat2(std::string name, Math::Vector2 data)
            {
                //If the variable doesn't exist in the map lets allocate it
                //Otherwise lets just change its data
                std::map<std::string, ShaderVariable*>::iterator it = m_shaderVariables.find(name);
                if (it != m_shaderVariables.end())
                    static_cast<Float2Variable*>(m_shaderVariables[name])->SetData(data);
                else
                    m_shaderVariables[name] = new Float2Variable(data);

                return true;
            }
            bool VKPipeline::VSetFloat3(std::string name, Math::Vector3 data)
            {
                //If the variable doesn't exist in the map lets allocate it
                //Otherwise lets just change its data
                std::map<std::string, ShaderVariable*>::iterator it = m_shaderVariables.find(name);
                if (it != m_shaderVariables.end())
                    static_cast<Float3Variable*>(m_shaderVariables[name])->SetData(data);
                else
                    m_shaderVariables[name] = new Float3Variable(data);

                return true;
            }
            bool VKPipeline::VSetFloat4(std::string name, Math::Vector4 data)
            {
                //If the variable doesn't exist in the map lets allocate it
                //Otherwise lets just change its data
                std::map<std::string, ShaderVariable*>::iterator it = m_shaderVariables.find(name);
                if (it != m_shaderVariables.end())
                    static_cast<Float4Variable*>(m_shaderVariables[name])->SetData(data);
                else
                    m_shaderVariables[name] = new Float4Variable(data);

                return true;
            }
            bool VKPipeline::VSetMatrix4(std::string name, Math::Matrix4 data)
            {
                //If the variable doesn't exist in the map lets allocate it
                //Otherwise lets just change its data
                std::map<std::string, ShaderVariable*>::iterator it = m_shaderVariables.find(name);
                if (it != m_shaderVariables.end())
                    static_cast<Matrix4Variable*>(m_shaderVariables[name])->SetData(data);
                else
                    m_shaderVariables[name] = new Matrix4Variable(data);
                
                return true;
            }

            bool VKPipeline::VUpdate()
            {
                //TODO: Organize push constant data other than just matricies
                if (m_shaderVariables.size() == 0)
                    return true;

                std::vector<uint32_t>       intList;
                std::vector<float>          floatList;
                std::vector<Math::Vector2>  vector2List;
                std::vector<Math::Vector3>  vector3List;
                std::vector<Math::Vector4>  vector4List;
                std::vector<Math::Matrix4>  matrixList;

                //Sort data into appropriate lists
                std::map <std::string, ShaderVariable*>::iterator it;
                for (it = m_shaderVariables.begin(); it != m_shaderVariables.end(); it++)
                {
                    ShaderVariable::Type varType = it->second->GetType();

                    switch (varType)
                    {
                    case ShaderVariable::INT:
                        intList.push_back(*(int*)(it->second->GetData()));
                        break;
                    case ShaderVariable::FLOAT:
                        floatList.push_back(*(float*)(it->second->GetData()));
                        break;
                    case ShaderVariable::FLOAT2:
                        vector2List.push_back(*(Math::Vector2*)(it->second->GetData()));
                        break;
                    case ShaderVariable::FLOAT3:
                        vector3List.push_back(*(Math::Vector3*)(it->second->GetData()));
                        break;
                    case ShaderVariable::FLOAT4:
                        vector4List.push_back(*(Math::Vector4*)(it->second->GetData()));
                        break;
                    case ShaderVariable::MAT4:
                        matrixList.push_back(*(Math::Matrix4*)(it->second->GetData()));
                        break;
                    }
                }

                //Resize vectors to fit new push data
                m_intPushData.clear();
                m_intPushData.resize(intList.size());

                m_floatPushData.clear();
                m_floatPushData.resize(floatList.size());

                m_vector2PushData.clear();
                m_vector2PushData.resize(vector2List.size() * 2);

                m_vector3PushData.clear();
                m_vector3PushData.resize(vector3List.size() * 3);

                m_vector4PushData.clear();
                m_vector4PushData.resize(vector4List.size() * 4);

                m_matrixPushData.clear();
                m_matrixPushData.resize(matrixList.size() * 16);

                //Copy data into variable lists
                size_t i = 0; //reuse i
                for (i = 0; i < intList.size(); i++)
                {
                    memcpy(m_intPushData.data() + i, static_cast<void*>(&intList[i]), sizeof(uint32_t));
                }
                
                for (i = 0; i < floatList.size(); i++)
                {
                    memcpy(m_floatPushData.data() + i, static_cast<void*>(&floatList[i]), sizeof(float));
                }
                
                for (i = 0; i < vector2List.size(); i++)
                {
                    Math::Vector2 vec = vector2List[i];
                    memcpy(m_vector2PushData.data() + (2 * i), static_cast<void*>(&vec[0]), sizeof(float) * 2);
                }
                
                for (i = 0; i < vector3List.size(); i++)
                {
                    Math::Vector3 vec = vector3List[i];
                    memcpy(m_vector3PushData.data() + (3 * i), static_cast<void*>(&vec[0]), sizeof(float) * 3);
                }

                for (i = 0; i < vector4List.size(); i++)
                {
                    Math::Vector4 vec = vector4List[i];
                    memcpy(m_vector4PushData.data() + (4 * i), static_cast<void*>(&vec[0]), sizeof(float) * 4);
                }
                
                for (i = 0; i < matrixList.size(); i++)
                {
                    Math::Matrix4 mat = matrixList[i];
                    memcpy(m_matrixPushData.data() + (16 * i), static_cast<void*>(matrixList[i].m_data), sizeof(float) * 16);
                }

                return true;
            }

            VkPipeline                          VKPipeline::GetVKPipeline() { return m_pipeline; }

            void VKPipeline::SendPushConstants(const VkCommandBuffer& commandBuffer, const VkPipelineLayout& pipelineLayout)
            {
                //Send a push for each type of data to send; vectors, matricies, ints etc.
                uint32_t dataSize = static_cast<uint32_t>(m_matrixPushData.size() * sizeof(float));
                vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, dataSize, m_matrixPushData.data());
            }

            /*
                Private Methods
            */

            void VKPipeline::setRasterState(const Pipeline::RasterizerState& rasterState)
            {
                VkPolygonMode polyMode;
                VkCullModeFlagBits cullMode;
                VkFrontFace frontFace;

                switch (rasterState.polygonMode)
                {
                case Pipeline::PolygonMode::SOLID:
                    polyMode = VK_POLYGON_MODE_FILL;
                    break;
                case Pipeline::PolygonMode::LINE:
                    polyMode = VK_POLYGON_MODE_LINE;
                    break;
                default:
                    polyMode = VK_POLYGON_MODE_FILL;
                    break;
                }

                switch (rasterState.cullMode)
                {
                case Pipeline::NONE:
                    cullMode = VK_CULL_MODE_NONE;
                    break;
                case Pipeline::FRONT:
                    cullMode = VK_CULL_MODE_FRONT_BIT;
                    break;
                case Pipeline::BACK:
                    cullMode = VK_CULL_MODE_BACK_BIT;
                    break;
                }

                if (rasterState.frontCounterClockwise)
                    frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
                else
                    frontFace = VK_FRONT_FACE_CLOCKWISE;

                m_rasterizationState = {}; //default setup
                m_rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
                m_rasterizationState.pNext = nullptr;
                m_rasterizationState.polygonMode = polyMode;
                m_rasterizationState.cullMode = cullMode;
                m_rasterizationState.frontFace = frontFace;
                m_rasterizationState.depthClampEnable = rasterState.depthClampEnable;
                m_rasterizationState.rasterizerDiscardEnable = rasterState.discardEnable;
                m_rasterizationState.depthBiasEnable = VK_FALSE;
            }

            void VKPipeline::setMultisampleState(const Pipeline::MultisampleState& multiState)
            {
                VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT;

                switch (multiState.samples)
                {
                case Pipeline::SAMPLE_1_BIT:
                    sampleCount = VK_SAMPLE_COUNT_1_BIT;
                    break;
                case Pipeline::SAMPLE_2_BIT:
                    sampleCount = VK_SAMPLE_COUNT_2_BIT;
                    break;
                case Pipeline::SAMPLE_4_BIT:
                    sampleCount = VK_SAMPLE_COUNT_4_BIT;
                    break;
                case Pipeline::SAMPLE_8_BIT:
                    sampleCount = VK_SAMPLE_COUNT_8_BIT;
                    break;
                case Pipeline::SAMPLE_16_BIT:
                    sampleCount = VK_SAMPLE_COUNT_16_BIT;
                    break;
                case Pipeline::SAMPLE_32_BIT:
                    sampleCount = VK_SAMPLE_COUNT_32_BIT;
                    break;
                case Pipeline::SAMPLE_64_BIT:
                    sampleCount = VK_SAMPLE_COUNT_64_BIT;
                    break;
                }

                m_multisampleState = {};
                m_multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
                m_multisampleState.pNext = nullptr;
                m_multisampleState.pSampleMask = nullptr;
                m_multisampleState.rasterizationSamples = sampleCount;
                m_multisampleState.sampleShadingEnable = multiState.perSampleShading;
                m_multisampleState.minSampleShading = multiState.minSamples;
            }

            void VKPipeline::loadShader(Pipeline::ShaderSlot shaderSlot, IShaderHandle shaderHandle)
            {
                VKShaderHandle shader = shaderHandle.DynamicCastHandle<VKShader>();
                m_shaderHandles[shaderSlot] = shader;

                VkShaderModule shaderModule = shader->GetShaderModule();

                VkShaderStageFlagBits shaderType;

                switch (shaderSlot)
                {
                case Pipeline::ShaderSlot::VERTEX:
                    shaderType = VK_SHADER_STAGE_VERTEX_BIT;
                    break;
                case Pipeline::ShaderSlot::FRAGMENT:
                    shaderType = VK_SHADER_STAGE_FRAGMENT_BIT;
                    break;
                case Pipeline::ShaderSlot::GEOMETRY:
                    shaderType = VK_SHADER_STAGE_GEOMETRY_BIT;
                    break;
                case Pipeline::ShaderSlot::TESS_CONTROL:
                    shaderType = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
                    break;
                case Pipeline::ShaderSlot::TESS_EVAL:
                    shaderType = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
                    break;
                case Pipeline::ShaderSlot::COMPUTE:
                    shaderType = VK_SHADER_STAGE_COMPUTE_BIT;
                    break;
                }

                VkPipelineShaderStageCreateInfo shaderStage = {};
                shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                shaderStage.stage = shaderType;
                shaderStage.module = shaderModule;
                shaderStage.pName = "main";

                m_shaderStages.push_back(shaderStage);
            }

            bool VKPipeline::preparePipeline(const VKRenderPassHandle& passHandle)
            {
                VkResult err;

                //If we don't have a vertex AND fragment shader available we need to log that and fail
                bool hasVert = false;
                bool hasFrag = false;
                for (uint32_t i = 0; i < m_shaderStages.size(); i++)
                {
                    if (m_shaderStages[i].stage & VK_SHADER_STAGE_VERTEX_BIT)
                        hasVert = true;
                    if (m_shaderStages[i].stage & VK_SHADER_STAGE_FRAGMENT_BIT)
                        hasFrag = true;
                }

                if (!hasVert || !hasFrag)
                {
                    HT_DEBUG_PRINTF("Error: Pipeline must have both Vertex and Fragment shaders");
                    assert(!hasVert || !hasFrag);

                    return false;
                }
                

                //Vertex info state
                VkVertexInputBindingDescription vertexBindingDescriptions[1] = {};

                vertexBindingDescriptions[0] = {};
                vertexBindingDescriptions[0].binding = 0;
                vertexBindingDescriptions[0].stride = sizeof(Vertex);
                vertexBindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

                VkVertexInputAttributeDescription vertexAttributeDescriptions[3] = {};

                vertexAttributeDescriptions[0] = {};
                vertexAttributeDescriptions[0].binding = 0;
                vertexAttributeDescriptions[0].location = 0;
                vertexAttributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
                vertexAttributeDescriptions[0].offset = 0;

                vertexAttributeDescriptions[1] = {};
                vertexAttributeDescriptions[1].binding = 0;
                vertexAttributeDescriptions[1].location = 1;
                vertexAttributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
                vertexAttributeDescriptions[1].offset = sizeof(float) * 3;

                vertexAttributeDescriptions[2] = {};
                vertexAttributeDescriptions[2].binding = 0;
                vertexAttributeDescriptions[2].location = 2;
                vertexAttributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
                vertexAttributeDescriptions[2].offset = sizeof(float) * 6;

                VkPipelineVertexInputStateCreateInfo vertexInputState = {};
                vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
                vertexInputState.pNext = nullptr;
                vertexInputState.vertexBindingDescriptionCount = 1;
                vertexInputState.vertexAttributeDescriptionCount = 3;
                vertexInputState.pVertexBindingDescriptions = vertexBindingDescriptions;
                vertexInputState.pVertexAttributeDescriptions = vertexAttributeDescriptions;

                //Topology
                VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
                inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
                inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

                //Must have a blend attachment for each color attachment or esle the write mask will be 0x0
                //In that case nothing will be written to the attachment
                VkPipelineColorBlendAttachmentState blendAttachmentState[3] = {};
                blendAttachmentState[0].colorWriteMask = 0xf;
                blendAttachmentState[0].blendEnable = VK_FALSE;
                blendAttachmentState[1].colorWriteMask = 0xf;
                blendAttachmentState[1].blendEnable = VK_FALSE;
                blendAttachmentState[2].colorWriteMask = 0xf;
                blendAttachmentState[2].blendEnable = VK_FALSE;

                //Color blends and masks
                VkPipelineColorBlendStateCreateInfo colorBlendState = {};
                colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
                colorBlendState.pAttachments = blendAttachmentState;
                colorBlendState.attachmentCount = 3;

                //Viewport
                VkPipelineViewportStateCreateInfo viewportState = {};
                viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
                viewportState.viewportCount = 1;
                viewportState.scissorCount = 1;

                //Enable dynamic states
                VkDynamicState dynamicStateEnables[] = { VK_DYNAMIC_STATE_VIEWPORT , VK_DYNAMIC_STATE_SCISSOR };

                VkPipelineDynamicStateCreateInfo dynamicState = {};
                dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
                dynamicState.pNext = nullptr;
                dynamicState.pDynamicStates = dynamicStateEnables;
                dynamicState.dynamicStateCount = 2;

                //Depth and stencil states
                VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
                depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
                depthStencilState.pNext = nullptr;
                depthStencilState.depthTestEnable = VK_TRUE;
                depthStencilState.depthWriteEnable = VK_TRUE;
                depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
                depthStencilState.depthBoundsTestEnable = VK_FALSE;
                depthStencilState.back.failOp = VK_STENCIL_OP_KEEP;
                depthStencilState.back.passOp = VK_STENCIL_OP_KEEP;
                depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
                depthStencilState.stencilTestEnable = VK_FALSE;
                depthStencilState.front = depthStencilState.back;
                depthStencilState.front.compareOp = VK_COMPARE_OP_NEVER;

                //Get pipeline layout
                VKRootLayoutHandle rootLayoutHandle = VKRenderer::RendererInstance->GetVKRootLayoutHandle();
                VkPipelineLayout pipelineLayout = rootLayoutHandle->VKGetPipelineLayout();

                //Finalize pipeline
                VkGraphicsPipelineCreateInfo pipelineInfo = {};
                pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
                pipelineInfo.layout = pipelineLayout;
                pipelineInfo.renderPass = passHandle->GetVkRenderPass();
                pipelineInfo.stageCount = static_cast<uint32_t>(m_shaderStages.size());
                pipelineInfo.pVertexInputState = &vertexInputState;
                pipelineInfo.pInputAssemblyState = &inputAssemblyState;
                pipelineInfo.pRasterizationState = &m_rasterizationState;
                pipelineInfo.pColorBlendState = &colorBlendState;
                pipelineInfo.pMultisampleState = &m_multisampleState;
                pipelineInfo.pViewportState = &viewportState;
                pipelineInfo.pDepthStencilState = &depthStencilState;
                pipelineInfo.pStages = m_shaderStages.data();
                pipelineInfo.pDynamicState = &dynamicState;

                VkPipelineCacheCreateInfo pipelineCacheInfo = {};
                pipelineCacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

                err = vkCreatePipelineCache(m_device, &pipelineCacheInfo, nullptr, &m_pipelineCache);
                assert(!err);
                if (err != VK_SUCCESS)
                {
                    HT_DEBUG_PRINTF("VKRenderer::preparePipeline(): Failed to create pipeline cache\n");
                    return false;
                }

                err = vkCreateGraphicsPipelines(m_device, m_pipelineCache, 1, &pipelineInfo, nullptr, &m_pipeline);
                assert(!err);
                if (err != VK_SUCCESS)
                {
                    HT_DEBUG_PRINTF("VKRenderer::preparePipeline(): Failed to create pipeline\n");
                    return false;
                }

                return true;
            }

        }
    }
}