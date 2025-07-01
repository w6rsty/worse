#include "Common.hlsl"

StructuredBuffer<Material> materials : register(t0, space1);

struct VertexOutput
{
    float4 position  : SV_Position;
    float2 uv        : TEXCOORD0;
    float3 worldPos  : TEXCOORD1;
    float3 normal    : NORMAL;
    float3 tangent   : TANGENT;
    float3 bitangent : TANGENT1;
    [[vk::builtin("PointSize")]] float pointSize : PSIZE;
};

VertexOutput main_vs(VertexPosUvNrmTan input)
{
    VertexOutput output;

    float4x4 mvp = mul(frameData.viewProjection, pushData.transform);
    output.position = mul(mvp, float4(input.position, 1.0));
    
    // Transform to world space
    output.worldPos = mul(pushData.transform, float4(input.position, 1.0)).xyz;
    output.uv = input.uv;
    
    // Transform normals to world space
    float3x3 normalMatrix = (float3x3)pushData.transform;
    output.normal = normalize(mul(input.normal, normalMatrix));
    output.tangent = normalize(mul(input.tangent, normalMatrix));
    output.bitangent = normalize(cross(output.normal, output.tangent));
    output.pointSize = getPadding().x;

    return output;
}

struct PixelOutput
{
    float4 color : SV_Target0;
};

PixelOutput main_ps(VertexOutput input)
{
    PixelOutput output;

    Material material = materials[getMaterialId()];
    
    // Sample albedo only
    float3 albedo = material.albedo.rgb;
    float3 textureAlbedo = materialTextures[material.albedoTextureIndex].Sample(samplers[samplerBilinearWrap], input.uv).rgb;
    albedo *= textureAlbedo;

    output.color = float4(albedo, material.albedo.a);

    return output;
}
