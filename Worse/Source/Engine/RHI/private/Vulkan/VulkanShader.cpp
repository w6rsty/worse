#include "DXCompiler.hpp"
#include "RHIDevice.hpp"
#include "RHIShader.hpp"

#include "spirv_reflect.h"

namespace worse
{
    namespace
    {
        RHIDescriptorType spvReflectDescriptorTypeToRHI(SpvReflectDescriptorType const type)
        {
            switch (type)
            {
            case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                return RHIDescriptorType::Texture;
            case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                return RHIDescriptorType::TextureStorage;
            case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                return RHIDescriptorType::UniformBuffer;
            case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                return RHIDescriptorType::StructuredBuffer;
            default:
                return RHIDescriptorType::Max;
            }
        }

        // 提取指定类型的描述符
        void spvExtractDescriptor(SpvReflectShaderModule const& reflection,
                                  RHIDescriptorType const descriptorType,
                                  RHIShaderStageFlags const shaderStage,
                                  std::vector<RHIDescriptor>& descriptors)
        {
            if (descriptorType == RHIDescriptorType::PushConstant)
            {
                u32 pushConstantCount = 0;
                spvReflectEnumeratePushConstantBlocks(&reflection, &pushConstantCount, nullptr);
                std::vector<SpvReflectBlockVariable*> pushConstants(pushConstantCount);
                spvReflectEnumeratePushConstantBlocks(&reflection, &pushConstantCount, pushConstants.data());

                for (SpvReflectBlockVariable const* block : pushConstants)
                {
                    RHIDescriptor descriptor = {};
                    descriptor.name          = block->name;
                    descriptor.stageFlags    = shaderStage;
                    descriptor.type          = RHIDescriptorType::PushConstant;
                    descriptor.size          = static_cast<u32>(block->size);

                    descriptors.push_back(descriptor);
                }
            }
            else
            {
                u32 descriptorSetCount = 0;
                spvReflectEnumerateDescriptorSets(&reflection, &descriptorSetCount, nullptr);
                std::vector<SpvReflectDescriptorSet*> descriptorSets(descriptorSetCount);
                spvReflectEnumerateDescriptorSets(&reflection, &descriptorSetCount, descriptorSets.data());

                RHIImageLayout layout = RHIImageLayout::Undefined;
                layout                = descriptorType == RHIDescriptorType::TextureStorage
                                            ? RHIImageLayout::General
                                            : layout;
                layout                = descriptorType == RHIDescriptorType::Texture
                                            ? RHIImageLayout::ShaderRead
                                            : layout;

                for (SpvReflectDescriptorSet const* set : descriptorSets)
                {
                    for (u32 i = 0; i < set->binding_count; ++i)
                    {
                        SpvReflectDescriptorBinding const* binding = set->bindings[i];

                        if (spvReflectDescriptorTypeToRHI(binding->descriptor_type) != descriptorType)
                        {
                            continue;
                        }

                        RHIDescriptor descriptor = {};
                        descriptor.name          = binding->name;
                        descriptor.space         = set->set;
                        descriptor.slot          = binding->binding;
                        descriptor.stageFlags    = shaderStage;
                        descriptor.type          = spvReflectDescriptorTypeToRHI(binding->descriptor_type);
                        descriptor.layout        = layout;

                        if (descriptorType == RHIDescriptorType::UniformBuffer)
                        {
                            descriptor.size = static_cast<u32>(binding->block.size);
                        }
                        descriptor.isArray     = binding->array.dims_count > 0;
                        descriptor.arrayLength = binding->array.dims_count > 0 ? binding->array.dims[0] : 0;
                        descriptors.push_back(descriptor);
                    }
                }
            }
        }
    } // namespace

    RHINativeHandle RHIShader::nativeCompile()
    {
        std::vector<std::wstring> wArguments;
        switch (m_shaderType)
        {
        case RHIShaderType::Vertex:
            wArguments = {L"-T", L"vs_6_8", L"-E", L"main_vs"};
            break;
        case RHIShaderType::Pixel:
            wArguments = {L"-T", L"ps_6_8", L"-E", L"main_ps"};
            break;
        case RHIShaderType::Compute:
            wArguments = {L"-T", L"cs_6_8", L"-E", L"main_cs"};
            break;
        default:
            WS_ASSERT(false);
            break;
        }

        // Static compilation options to avoid repeated construction
        // clang-format off
        std::vector<std::wstring> commonOptions = {
            L"-spirv",
            L"-fspv-target-env=vulkan1.2",
            L"-fspv-preserve-bindings",  // disable re-numbering
            L"-fspv-preserve-interface", // disable optimization that removes unused locations
            L"-fvk-b-shift", std::to_wstring(RHIConfig::HLSL_REGISTER_SHIFT_B), L"all",
            L"-fvk-t-shift", std::to_wstring(RHIConfig::HLSL_REGISTER_SHIFT_T), L"all",
            L"-fvk-s-shift", std::to_wstring(RHIConfig::HLSL_REGISTER_SHIFT_S), L"all",
            L"-fvk-u-shift", std::to_wstring(RHIConfig::HLSL_REGISTER_SHIFT_U), L"all",
            L"-Zi",
        };

        if (m_shaderType == RHIShaderType::Vertex)
        {
            commonOptions.push_back(L"-fvk-invert-y");
        }
        // clang-format on

        wArguments.insert(wArguments.end(), commonOptions.begin(), commonOptions.end());

        std::string filepath   = m_path.string();
        std::wstring wFilepath = std::wstring(filepath.begin(), filepath.end());
        wArguments.push_back(wFilepath);

        RHINativeHandle shader = {};

        CComPtr<IDxcBlob> codeBlob = DXCompiler::instance()->compile(m_source, wArguments);

        if (!codeBlob)
        {
            WS_LOG_ERROR("Shader", "Compilation failed: {}", m_name);
            return shader;
        }

        VkShaderModuleCreateInfo infoShader = {};
        infoShader.sType                    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        infoShader.pCode                    = static_cast<u32*>(codeBlob->GetBufferPointer());
        infoShader.codeSize                 = codeBlob->GetBufferSize();

        VkShaderModule vkShader = VK_NULL_HANDLE;
        WS_ASSERT_VK(vkCreateShaderModule(RHIContext::device, &infoShader, nullptr, &vkShader));
        shader = RHINativeHandle{vkShader, RHINativeHandleType::Shader};
        RHIDevice::setResourceName(shader, m_name);

        reflect(m_shaderType, static_cast<u32*>(codeBlob->GetBufferPointer()), codeBlob->GetBufferSize() / sizeof(u32));

        codeBlob.Release();

        return shader;
    }

    void RHIShader::reflect(RHIShaderType const shaderType, u32* spirvData, usize const spirvSize)
    {
        WS_ASSERT(spirvData != nullptr);
        WS_ASSERT(spirvSize > 0);

        SpvReflectShaderModule reflection{};
        spvReflectCreateShaderModule(spirvSize * sizeof(u32), spirvData, &reflection);

        RHIShaderStageFlags shaderStage = rhiShaderStageFlags(shaderType);

        // Texture
        spvExtractDescriptor(reflection, RHIDescriptorType::Texture, shaderStage, m_descriptors);
        // RWTexture
        spvExtractDescriptor(reflection, RHIDescriptorType::TextureStorage, shaderStage, m_descriptors);
        // [[vk::push_constant]]
        spvExtractDescriptor(reflection, RHIDescriptorType::PushConstant, shaderStage, m_descriptors);
        // uniform buffer(cbuffer)
        spvExtractDescriptor(reflection, RHIDescriptorType::UniformBuffer, shaderStage, m_descriptors);
        // storage buffer(RWStructuredBuffer)
        spvExtractDescriptor(reflection, RHIDescriptorType::StructuredBuffer, shaderStage, m_descriptors);
    }
} // namespace worse
