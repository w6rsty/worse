#include "Common.hlsl"
#include "Utils.hlsl"

StructuredBuffer<MaterialParameters> materialParams : register(t0, space1);

struct VertexOutput
{
    float4 position  : SV_Position;
    float2 uv        : TEXCOORD0;
    float3 worldPos  : POSITION;
    float3 normal    : NORMAL;
    float3 tangent   : TANGENT;
    float3 bitangent : TANGENT1;
};

VertexOutput main_vs(VertexPosUvNrmTan input)
{
    VertexOutput output;

    matrix mvp            = mul(frameData.viewProjection, pushData.transform);
    output.position       = mul(mvp, float4(input.position, 1.0));
    
    output.worldPos       = mul(pushData.transform, float4(input.position, 1.0)).xyz;
    output.uv             = input.uv;
    
    float3x3 normalMatrix = (float3x3)pushData.transform;
    output.normal         = normalize(mul(input.normal, normalMatrix));
    output.tangent        = normalize(mul(input.tangent.xyz, normalMatrix)) * input.tangent.w;
    output.bitangent      = normalize(cross(output.normal, output.tangent));

    return output;
}

struct GBuffer
{
    float4 albedo   : SV_Target0;
    float4 normal   : SV_Target1;
    float4 material : SV_Target2; // metallic, roughness, emission, occlusion
    float4 position : SV_Target3;
};

GBuffer main_ps(VertexOutput input)
{
    MaterialParameters material = materialParams[getMaterialId()];
    float4 albedo     = material.baseColor;
    float emission    = 0.0f;
    float3 normal     = input.normal;
    float metallic    = material.metallic;
    float roughness   = material.roughness;
    float occlusion   = 1.0f;

    // albedo
    if (HasBaseColorTexture(material))
    {
        float4 albedoSample = 1.0f;

        albedoSample = materialTextures[material.baseColorTextureIndex].Sample(samplers[samplerBilinearWrap], input.uv);
        albedoSample.rgb = SRGBToLinear(albedoSample.rgb);
        albedo *= albedoSample;

        albedo.a = 1.0f; // Not support transparency now
    }

    // Emission
    if (HasEmissiveTexture(material))
    {
        float3 emissiveColor = materialTextures[material.emissiveTextureIndex].Sample(samplers[samplerBilinearWrap], input.uv).rgb;
        albedo.rgb += SRGBToLinear(emissiveColor);
        emission = Luminance(emissiveColor);
    }

    // Normal mapping
    if (HasNormalTexture(material))
    {
        float3 normalSample = materialTextures[material.normalTextureIndex].Sample(samplers[samplerBilinearWrap], input.uv).rgb;
        // normal in tangent space
        float3 tangentNormal = normalize(normalSample * 2.0 - 1.0);
        // reconstruct normal, 兼容 bc5 2 2-channel normal map
        tangentNormal.z = FastSqrt(max(0.0, 1.0 - tangentNormal.x * tangentNormal.x - tangentNormal.y * tangentNormal.y));

        float3x3 TBN = MakeTangentToWorldMatrix(input.normal, input.tangent);
        normal = normalize(mul(tangentNormal, TBN));
    }
    
    // Metallic, Roughness, Occlusion
    {
        float4 packed = materialTextures[material.metallicRoughnessTextureIndex].Sample(samplers[samplerBilinearWrap], input.uv);
        metallic *= lerp(1.0f, packed.r, HasMetallicRoughnessTexture(material) ? 1.0f : 0.0f);
        roughness *= lerp(1.0f, packed.g, HasMetallicRoughnessTexture(material) ? 1.0f : 0.0f);

        float occlusionSample = materialTextures[material.ambientOcclusionTextureIndex].Sample(samplers[samplerBilinearWrap], input.uv).r;
        occlusion = lerp(occlusion, occlusionSample, HasAmbientOcclusionTexture(material) ? 1.0f : 0.0f);
    }

    GBuffer gbuffer;
    gbuffer.albedo   = albedo;
    gbuffer.normal   = float4(normal, getMaterialId());
    gbuffer.material = float4(metallic, roughness, emission, occlusion);
    gbuffer.position = float4(input.worldPos, 1.0f);

    return gbuffer;
}