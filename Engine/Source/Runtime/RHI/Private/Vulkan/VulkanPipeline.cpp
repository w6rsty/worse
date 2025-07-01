#include "RHIDevice.hpp"
#include "RHIShader.hpp"
#include "RHIVertex.hpp"
#include "RHITexture.hpp"
#include "Pipeline/RHIPipeline.hpp"
#include "Pipeline/RHIBlendState.hpp"
#include "Pipeline/RHIRasterizerState.hpp"
#include "Pipeline/RHIDepthStencilState.hpp"

#include <vector>

namespace worse
{

    namespace
    {
        VkPipelineShaderStageCreateInfo createShaderStage(RHIShader* shader)
        {
            // clang-format off
            VkPipelineShaderStageCreateInfo infoShaderStage = {};
            infoShaderStage.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            infoShaderStage.module = shader->getHandle().asValue<VkShaderModule>();
            infoShaderStage.pName  = shader->getEntryPoint().data();

            switch (shader->getShaderType())
            {
            case RHIShaderType::Vertex:  infoShaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;   break;
            case RHIShaderType::Pixel:   infoShaderStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT; break;
            case RHIShaderType::Compute: infoShaderStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;  break;
            default:                                                                           break;
            }
            // clang-format on

            WS_ASSERT(infoShaderStage.module != VK_NULL_HANDLE);
            WS_ASSERT(infoShaderStage.pName != nullptr);
            WS_ASSERT(infoShaderStage.stage != 0);

            return infoShaderStage;
        }
    } // namespace

    void
    RHIPipeline::nativeCreate(RHIPipelineState const& pipelineState,
                              RHIDescriptorSetLayout const& descriptorSetLayout)
    {
        // clang-format off
        m_state = pipelineState;

        // shader stages
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
        for (std::size_t i = 0; i < static_cast<std::size_t>(RHIShaderType::Max); ++i)
        {
            if (m_state.shaders[i])
            {
                shaderStages.push_back(createShaderStage(m_state.shaders[i]));
            }
        }

        // descriptor set layout
        std::vector<VkDescriptorSetLayout> layouts;
        layouts.reserve(2);
        {
            // global
            layouts.push_back(RHIDevice::getGlobalDescriptorSetLayout().asValue<VkDescriptorSetLayout>());
            // specific
            layouts.push_back(descriptorSetLayout.getLayout().asValue<VkDescriptorSetLayout>());
        }

        // push constant
        std::vector<VkPushConstantRange> pushConstantRanges;
        {
            for (RHIDescriptor const& descriptor : descriptorSetLayout.getPushConstants())
            {
                VkPushConstantRange pushConstantRange = {};
                pushConstantRange.stageFlags = vulkanShaderStageFlags(descriptor.stageFlags);
                pushConstantRange.offset     = 0;
                pushConstantRange.size       = RHIConfig::MAX_PUSH_CONSTANT_SIZE;

                pushConstantRanges.push_back(pushConstantRange);
            }
        }

        // pipeline layout
        {
            VkPipelineLayoutCreateInfo infoPipelineLayout = {};
            infoPipelineLayout.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            infoPipelineLayout.setLayoutCount         = static_cast<std::uint32_t>(layouts.size());
            infoPipelineLayout.pSetLayouts            = layouts.data();
            infoPipelineLayout.pushConstantRangeCount = static_cast<std::uint32_t>(pushConstantRanges.size());
            infoPipelineLayout.pPushConstantRanges    = pushConstantRanges.data();

            VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
            WS_ASSERT_VK(vkCreatePipelineLayout(RHIContext::device,
                                                &infoPipelineLayout,
                                                nullptr,
                                                &pipelineLayout));
            m_pipelineLayout = RHINativeHandle{pipelineLayout, RHINativeHandleType::PipelineLayout};
            RHIDevice::setResourceName(m_pipelineLayout, m_state.name);
        }
    
        if (m_state.type == RHIPipelineType::Compute)
        {
            VkComputePipelineCreateInfo infoComputePipeline = {};
            infoComputePipeline.sType  = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
            infoComputePipeline.layout = m_pipelineLayout.asValue<VkPipelineLayout>();
            infoComputePipeline.stage  = shaderStages[0];

            VkPipeline pipeline = VK_NULL_HANDLE;
            WS_ASSERT_VK(vkCreateComputePipelines(RHIContext::device,
                                                  VK_NULL_HANDLE,
                                                  1,
                                                  &infoComputePipeline,
                                                  nullptr,
                                                  &pipeline));
            m_pipeline = RHINativeHandle{pipeline, RHINativeHandleType::Pipeline};
            RHIDevice::setResourceName(m_pipeline, pipelineState.name);
        }
        if (m_state.type == RHIPipelineType::Graphics)
        {
            std::vector<VkFormat> attachmentColorFormats;
            VkFormat depthFormat = VK_FORMAT_UNDEFINED;
            VkFormat stencilFormat = VK_FORMAT_UNDEFINED;

            for (RHITexture* texture : m_state.renderTargetColorTextures)
            {
                // render target are set in order, so stop once hit a null texture
                if (!texture)
                {
                    break;
                }

                attachmentColorFormats.push_back(vulkanFormat(texture->getFormat()));
            }

            if (m_state.renderTargetDepthTexture)
            {
                RHITexture* depthTexture = m_state.renderTargetDepthTexture;
                depthFormat = vulkanFormat(depthTexture->getFormat());
                stencilFormat = depthTexture->isFormatStencil() ? depthFormat : VK_FORMAT_UNDEFINED;
            }

            VkPipelineRenderingCreateInfo infoRendering = {};
            infoRendering.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
            infoRendering.colorAttachmentCount    = static_cast<std::uint32_t>(attachmentColorFormats.size());
            infoRendering.pColorAttachmentFormats = attachmentColorFormats.data();
            infoRendering.depthAttachmentFormat   = depthFormat;
            infoRendering.stencilAttachmentFormat = stencilFormat;

            std::vector<VkVertexInputBindingDescription> vertexInputBindings;
            std::vector<VkVertexInputAttributeDescription> vertexInputAttributes;
            // get vertex info from verte shader
            if (RHIShader* vertexShader = m_state.shaders[static_cast<std::size_t>(RHIShaderType::Vertex)])
            {
                RHIInputLayout const& inputLayout = vertexShader->getInputLayout();
                auto attributes = inputLayout.getAttributes();

                if (vertexShader->getVertexType() != RHIVertexType::None)
                {
                    vertexInputBindings.push_back(VkVertexInputBindingDescription{
                        .binding   = 0,
                        .stride    = inputLayout.getStride(),
                        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
                    });

                    for (RHIVertexAttribute const& attribute : attributes)
                    {
                        vertexInputAttributes.push_back(VkVertexInputAttributeDescription{
                            .location = attribute.location,
                            .binding  = 0,
                            .format   = vulkanFormat(attribute.format),
                            .offset   = attribute.offset,
                        });
                    }
                }
            }

            VkPipelineVertexInputStateCreateInfo vertexInputState = {};
            vertexInputState.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertexInputState.vertexBindingDescriptionCount   = static_cast<std::uint32_t>(vertexInputBindings.size());
            vertexInputState.pVertexBindingDescriptions      = vertexInputBindings.data();
            vertexInputState.vertexAttributeDescriptionCount = static_cast<std::uint32_t>(vertexInputAttributes.size());
            vertexInputState.pVertexAttributeDescriptions    = vertexInputAttributes.data();

            VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
            inputAssemblyState.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            inputAssemblyState.topology               = vulkanPrimitiveTopology(m_state.primitiveTopology);
            inputAssemblyState.primitiveRestartEnable = VK_FALSE;

            VkPipelineTessellationStateCreateInfo tesselationState = {};
            tesselationState.sType              = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
            tesselationState.patchControlPoints = 1;

            VkPipelineViewportStateCreateInfo viewportState = {};
            viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewportState.viewportCount = 1;
            viewportState.pViewports    = nullptr;
            viewportState.scissorCount  = 1;
            viewportState.pScissors     = nullptr;

            VkPipelineRasterizationStateCreateInfo rasterizerState = {};
            rasterizerState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizerState.rasterizerDiscardEnable = VK_FALSE;
            rasterizerState.polygonMode             = vulkanPolygonMode(m_state.rasterizerState->getPolygonMode());
            rasterizerState.cullMode                = vulkanCullModeFlags(m_state.rasterizerState->getCullMode());
            rasterizerState.frontFace               = vulkanFrontFace(m_state.rasterizerState->getFrontFace());
            rasterizerState.lineWidth               = m_state.rasterizerState->getLineWidth();
            rasterizerState.depthClampEnable        = m_state.rasterizerState->getDepthClampEnable() ? VK_TRUE : VK_FALSE;
            rasterizerState.depthBiasEnable         = m_state.rasterizerState->getDepthBias() != 0.0f ? VK_TRUE : VK_FALSE;
            rasterizerState.depthBiasConstantFactor = m_state.rasterizerState->getDepthBias();
            rasterizerState.depthBiasClamp          = m_state.rasterizerState->getDepthBiasClamp();
            rasterizerState.depthBiasSlopeFactor    = m_state.rasterizerState->getDepthBiasSlopeFactor();

            VkPipelineMultisampleStateCreateInfo multisampleState = {};
            multisampleState.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampleState.sampleShadingEnable  = VK_FALSE;
            multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
            if (m_state.depthStencilState)
            {
                depthStencilState.sType             = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
                depthStencilState.depthTestEnable   = m_state.depthStencilState->getDepthTestEnabled() ? VK_TRUE : VK_FALSE;
                depthStencilState.depthWriteEnable  = m_state.depthStencilState->getDepthWriteEnabled() ? VK_TRUE : VK_FALSE;
                depthStencilState.depthCompareOp    = vulkanCompareOp(m_state.depthStencilState->getDepthCompareOp());
                depthStencilState.stencilTestEnable = m_state.depthStencilState->getStencilTestEnabled() ? VK_TRUE : VK_FALSE;
                depthStencilState.front.compareOp   = vulkanCompareOp(m_state.depthStencilState->getStencilCompareOp());
                depthStencilState.front.depthFailOp = vulkanStencilOp(m_state.depthStencilState->getStencilDepthFailOp());
                depthStencilState.front.failOp      = vulkanStencilOp(m_state.depthStencilState->getStencilFailOp());
                depthStencilState.front.passOp      = vulkanStencilOp(m_state.depthStencilState->getStencilPassOp());
                depthStencilState.front.writeMask   = m_state.depthStencilState->getStencilWriteMask();
                depthStencilState.front.reference   = 1;
                depthStencilState.back              = depthStencilState.front;
                depthStencilState.minDepthBounds    = 1.0f; // reversed-Z
                depthStencilState.maxDepthBounds    = 0.0f; // reversed-Z
            }

            std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
            if (m_state.blendState)
            {
                VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
                colorBlendAttachment.colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT; 
                colorBlendAttachment.blendEnable         = m_state.blendState->getBlendEnable() ? VK_TRUE : VK_FALSE;
                colorBlendAttachment.srcColorBlendFactor = vulkanBlendFactor(m_state.blendState->getSrcBlend());
                colorBlendAttachment.dstColorBlendFactor = vulkanBlendFactor(m_state.blendState->getDstBlend());
                colorBlendAttachment.colorBlendOp        = vulkanBlendOp(m_state.blendState->getBlendOp());
                colorBlendAttachment.srcAlphaBlendFactor = vulkanBlendFactor(m_state.blendState->getSrcAlphaBlend());
                colorBlendAttachment.dstAlphaBlendFactor = vulkanBlendFactor(m_state.blendState->getDstAlphaBlend());
                colorBlendAttachment.alphaBlendOp        = vulkanBlendOp(m_state.blendState->getAlphaBlendOp());

                for (RHITexture* texture : m_state.renderTargetColorTextures)
                {
                    // render target are set in order, so stop once hit a null texture
                    if (!texture)
                    {
                        break;
                    }

                    colorBlendAttachments.push_back(colorBlendAttachment);
                }
            }
            VkPipelineColorBlendStateCreateInfo colorBlendState = {};
            colorBlendState.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            colorBlendState.logicOpEnable     = VK_FALSE;
            colorBlendState.logicOp           = VK_LOGIC_OP_COPY;
            colorBlendState.attachmentCount   = static_cast<std::uint32_t>(colorBlendAttachments.size());
            colorBlendState.pAttachments      = colorBlendAttachments.data();
            float blendFactor                 = m_state.blendState->getBlendFactor();
            colorBlendState.blendConstants[0] = blendFactor;
            colorBlendState.blendConstants[1] = blendFactor;
            colorBlendState.blendConstants[2] = blendFactor;
            colorBlendState.blendConstants[3] = blendFactor;

            std::vector<VkDynamicState> dynamicStates = {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR,
            };
            VkPipelineDynamicStateCreateInfo dynamicState = {};
            dynamicState.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamicState.dynamicStateCount = static_cast<std::uint32_t>(dynamicStates.size());
            dynamicState.pDynamicStates    = dynamicStates.data();


            VkGraphicsPipelineCreateInfo infoGraphicsPipeline = {};
            infoGraphicsPipeline.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            infoGraphicsPipeline.pNext               = &infoRendering;
            infoGraphicsPipeline.stageCount          = static_cast<std::uint32_t>(shaderStages.size());
            infoGraphicsPipeline.pStages             = shaderStages.data();
            infoGraphicsPipeline.pVertexInputState   = &vertexInputState;
            infoGraphicsPipeline.pInputAssemblyState = &inputAssemblyState;
            infoGraphicsPipeline.pTessellationState  = &tesselationState;
            infoGraphicsPipeline.pViewportState      = &viewportState;
            infoGraphicsPipeline.pRasterizationState = &rasterizerState;
            infoGraphicsPipeline.pMultisampleState   = &multisampleState;
            infoGraphicsPipeline.pDepthStencilState  = &depthStencilState;
            infoGraphicsPipeline.pColorBlendState    = &colorBlendState;
            infoGraphicsPipeline.pDynamicState       = &dynamicState;
            infoGraphicsPipeline.layout              = m_pipelineLayout.asValue<VkPipelineLayout>();
            // clang-format on

            VkPipeline pipeline = VK_NULL_HANDLE;
            WS_ASSERT_VK(vkCreateGraphicsPipelines(RHIContext::device,
                                                   VK_NULL_HANDLE,
                                                   1,
                                                   &infoGraphicsPipeline,
                                                   nullptr,
                                                   &pipeline));
            m_pipeline =
                RHINativeHandle{pipeline, RHINativeHandleType::Pipeline};
            RHIDevice::setResourceName(m_pipeline, pipelineState.name);
        }

        WS_ASSERT_MSG(m_pipeline, "Failed to create pipeline");

        // log creation detail
        std::string shaderNames;
        for (std::size_t i = 0;
             i < static_cast<std::size_t>(RHIShaderType::Max);
             ++i)
        {
            if (m_state.shaders[i])
            {
                if (!shaderNames.empty())
                {
                    shaderNames += ", ";
                }
                shaderNames += m_state.shaders[i]->getName();
            }
        }
        WS_LOG_INFO("Pipeline",
                    "Created `{}` (Type: {}, Topology {}, Shaders: [{}])",
                    m_state.name,
                    m_state.type == RHIPipelineType::Graphics ? "graphics"
                                                              : "compute",
                    rhiPrimitiveTopologyToString(m_state.primitiveTopology),
                    shaderNames);
    }

} // namespace worse