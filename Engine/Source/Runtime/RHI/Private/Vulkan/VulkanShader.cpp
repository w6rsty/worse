#include "DXCompiler.hpp"
#include "RHIDevice.hpp"
#include "RHIShader.hpp"

#include "spirv_cross/spirv.hpp"
#include "spirv_cross/spirv_hlsl.hpp"

namespace worse
{
    namespace
    {
        void
        spirvExtractDescriptor(spirv_cross::CompilerHLSL const& hlsl,
                               spirv_cross::ShaderResources shaderResources,
                               RHIDescriptorType const descriptorType,
                               RHIShaderStageFlags const shaderStage,
                               std::vector<RHIDescriptor>& descriptors)
        {
            spirv_cross::SmallVector<spirv_cross::Resource> resources;
            switch (descriptorType)
            {
                // clang-format off
            case RHIDescriptorType::Texture:            resources = shaderResources.separate_images;       break;
            case RHIDescriptorType::TextureStorage:     resources = shaderResources.storage_images;        break;
            case RHIDescriptorType::PushConstantBuffer: resources = shaderResources.push_constant_buffers; break;
            case RHIDescriptorType::ConstantBuffer:     resources = shaderResources.uniform_buffers;       break;
            case RHIDescriptorType::StructuredBuffer:   resources = shaderResources.storage_buffers;       break;
            default:                                                                                       break;
                // clang-format on
            }

            RHIImageLayout layout = RHIImageLayout::Undefined;
            layout = descriptorType == RHIDescriptorType::TextureStorage
                         ? RHIImageLayout::General
                         : layout;
            layout = descriptorType == RHIDescriptorType::Texture
                         ? RHIImageLayout::ShaderRead
                         : layout;

            for (spirv_cross::Resource const& resource : resources)
            {
                // clang-format off
                RHIDescriptor descriptor = {};
                descriptor.name          = resource.name;
                descriptor.space         = hlsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
                descriptor.slot          = hlsl.get_decoration(resource.id, spv::DecorationBinding);
                descriptor.stageFlags    = shaderStage;
                descriptor.type          = descriptorType;
                descriptor.layout        = layout;

                auto spirvType = hlsl.get_type(resource.type_id);
                if ((descriptorType == RHIDescriptorType::PushConstantBuffer) ||
                    (descriptorType == RHIDescriptorType::ConstantBuffer))
                {
                    descriptor.size = static_cast<std::uint32_t>(hlsl.get_declared_struct_size(spirvType));
                }
                descriptor.isArray     = spirvType.array.size() > 0;
                descriptor.arrayLength = descriptor.isArray ? spirvType.array[0] : 0;
                // clang-format on

                descriptors.push_back(descriptor);
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
        static const std::vector<std::wstring> commonOptions = {
            L"-spirv",
            L"-fspv-target-env=vulkan1.2",
            L"-fspv-preserve-bindings",  // disable re-numbering
            L"-fspv-preserve-interface", // disable optimization that removes unused locations
            L"-fvk-b-shift", std::to_wstring(RHIConfig::HLSL_REGISTER_SHIFT_B), L"all",
            L"-fvk-t-shift", std::to_wstring(RHIConfig::HLSL_REGISTER_SHIFT_T), L"all",
            L"-fvk-s-shift", std::to_wstring(RHIConfig::HLSL_REGISTER_SHIFT_S), L"all",
            L"-fvk-u-shift", std::to_wstring(RHIConfig::HLSL_REGISTER_SHIFT_U), L"all",
        };
        // clang-format on

        wArguments.insert(wArguments.end(),
                          commonOptions.begin(),
                          commonOptions.end());

        std::string filepath   = m_path.string();
        std::wstring wFilepath = std::wstring(filepath.begin(), filepath.end());
        wArguments.push_back(wFilepath);

        RHINativeHandle shader = {};

        CComPtr<IDxcBlob> codeBlob =
            DXCompiler::instance()->compile(m_source, wArguments);

        if (!codeBlob)
        {
            WS_LOG_ERROR("Shader", "Compilation failed");
            return shader;
        }

        // clang-format off
        VkShaderModuleCreateInfo infoShader = {};
        infoShader.sType                    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        infoShader.pCode                    = static_cast<std::uint32_t*>(codeBlob->GetBufferPointer());
        infoShader.codeSize                 = codeBlob->GetBufferSize();
        // clang-format on

        VkShaderModule vkShader = VK_NULL_HANDLE;
        WS_ASSERT_VK(vkCreateShaderModule(RHIContext::device,
                                          &infoShader,
                                          nullptr,
                                          &vkShader));
        shader = RHINativeHandle{vkShader, RHINativeHandleType::Shader};
        RHIDevice::setResourceName(shader, m_name);

        reflect(m_shaderType,
                static_cast<std::uint32_t*>(codeBlob->GetBufferPointer()),
                codeBlob->GetBufferSize() / sizeof(std::uint32_t));

        codeBlob.Release();

        return shader;
    }

    void RHIShader::reflect(RHIShaderType const shaderType,
                            std::uint32_t* spirvData,
                            std::size_t const spirvSize)
    {
        WS_ASSERT(spirvData != nullptr);
        WS_ASSERT(spirvSize > 0);

        spirv_cross::CompilerHLSL hlsl{spirvData, spirvSize};
        spirv_cross::ShaderResources resources = hlsl.get_shader_resources();

        // clang-format off
        RHIShaderStageFlags shaderStage = rhiShaderStageFlags(shaderType);
        // Texture
        spirvExtractDescriptor(hlsl, resources, RHIDescriptorType::Texture,            shaderStage, m_descriptors);
        // RWTexture
        spirvExtractDescriptor(hlsl, resources, RHIDescriptorType::TextureStorage,     shaderStage, m_descriptors);
        // [[vk::push_constant]]
        spirvExtractDescriptor(hlsl, resources, RHIDescriptorType::PushConstantBuffer, shaderStage, m_descriptors);
        // cbuffer / uniform buffer
        spirvExtractDescriptor(hlsl, resources, RHIDescriptorType::ConstantBuffer,     shaderStage, m_descriptors);
        // RWStructuredBuffer / storage buffer
        spirvExtractDescriptor(hlsl, resources, RHIDescriptorType::StructuredBuffer,   shaderStage, m_descriptors);
        // clang-format on

        for (RHIDescriptor& descriptor : m_descriptors)
        {
            std::string typeName = rhiDescriptorTypeToString(descriptor.type);
            WS_LOG_DEBUG("SPIRV-Cross",
                         "(type: {:>15}, slot: {:>3}, space: {:>2}, length: {:>2}) {}",
                         typeName,
                         descriptor.slot,
                         descriptor.space,
                         descriptor.isArray
                             ? std::to_string(descriptor.arrayLength)
                             : "1",
                         descriptor.name);
        }
    }

} // namespace worse
