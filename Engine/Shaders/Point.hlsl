#include "Common.hlsl"

StructuredBuffer<Material> materials : register(t0, space1);

struct VertexOutput
{
    float4 position : SV_Position;
    float3 worldPos : TEXCOORD0;
    [[vk::builtin("PointSize")]] float pointSize : PSIZE;
};

VertexOutput main_vs(VertexPos input)
{
    VertexOutput output;

    float4x4 mvp = mul(frameData.viewProjection, pushData.transform);
    output.position = mul(mvp, float4(input.position, 1.0));
    
    // Transform to world space
    output.worldPos = mul(pushData.transform, float4(input.position, 1.0)).xyz;
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
    output.color = material.albedo;

    return output;
}
