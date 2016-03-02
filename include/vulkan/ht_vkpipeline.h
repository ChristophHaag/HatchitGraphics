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
* \class VKPipeline
* \ingroup HatchitGraphics
*
* \brief An extension of IPipeline implemented with Vulkan
*
* Creates a pipeline with Vulkan that controls rasterization state, 
* topology, shaders etc.
*/

#pragma once

#include <ht_pipeline.h>

#include <ht_vkrenderpass.h>
#include <ht_vkshader.h>

#include <ht_vulkan.h>

#include <cassert>

namespace Hatchit {

    namespace Graphics {

        namespace Vulkan {

            class HT_API VKPipeline : public IPipeline
            {
            public:
                VKPipeline(const VkRenderPass* renderPass);
                ~VKPipeline();

                //If we wanted to allow users to control blending states
                //void VSetColorBlendAttachments(ColorBlendState* colorBlendStates) override;

                /* Set the rasterization state for this pipeline
                * \param rasterState A struct containing rasterization options
                */
                void VSetRasterState(const RasterizerState& rasterState)        override;

                /* Set the multisampling state for this pipeline
                * \param multiState A struct containing multisampling options
                */
                void VSetMultisampleState(const MultisampleState& multiState)   override;

                /* Load a shader into a shader slot for the pipeline
                * \param shaderSlot The slot that you want the shader in; vertex, fragment etc.
                * \param shader A pointer to the shader that you want to load to the given shader slot
                */
                void VLoadShader(ShaderSlot shaderSlot, IShader* shader)        override;

                ///Have Vulkan create a pipeline with these settings
                bool VPrepare()                                                 override;

                VkPipeline GetVKPipeline();
                VkPipelineLayout GetPipelineLayout();
                VkDescriptorSetLayout GetDescriptorSetLayout();

            protected:
                //Input
                const VkRenderPass* m_renderPass;

                VkPipelineRasterizationStateCreateInfo m_rasterizationState;
                VkPipelineMultisampleStateCreateInfo m_multisampleState;
                std::vector<VkPipelineShaderStageCreateInfo> m_shaderStages;

                VkDescriptorSetLayout   m_descriptorLayout;
                VkPipelineLayout        m_pipelineLayout;

                VkDescriptorSet m_descriptorSet; //Collection of shader variables
                VkPipelineCache m_pipelineCache;
                VkPipeline      m_pipeline;

            private:
                bool PrepareLayouts(VkDevice device);
                bool PrepareDescriptorSet(VkDevice device);
                bool PreparePipeline(VkDevice device);
            };
        }
    }
}