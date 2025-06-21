struct FrameConstantData
{
    float deltaTime;
    float time;
    float2 padding0; // align to 16 bytes

    float3 cameraPosition;
    float cameraNear;
    float3 cameraForward;
    float cameraFar;
    float4 padding1; // align to 16 bytes

    float4x4 view;
    float4x4 projection;
    float4x4 viewProjection;
};

struct PushConstantData
{
    float4x4 model;
    float4x4 values;
};

static const uint samplerPointClampEdge;
static const uint samplerPointClampBorder;
static const uint samplerWrap;
static const uint samplerBilinearClampEdge;
static const uint samplerBilinearClampBorder;
static const uint samplerBilinearWrap;
static const uint samplerTrilinearClamp;
static const uint samplerAnisotropicClamp;

// ================================================================================
// global set
// ================================================================================

cbuffer FrameConstantBuffer               : register(b0, space0) { FrameConstantData frameData; };
SamplerComparisonState samplerComparison  : register(s0, space0);
SamplerState samplers[8]                  : register(s1, space0);
Texture2D<float4> materialTextures[]      : register(t0, space0);

[[vk::push_constant]] PushConstantData pushData;

float2 getF2() { return float2(pushData.values._m00, pushData.values._m10); }
float3 getF30() { return float3(pushData.values._m20, pushData.values._m30, pushData.values._m01); }
float3 getF31() { return float3(pushData.values._m11, pushData.values._m21, pushData.values._m31); }
float4 getF4() { return float4(pushData.values._m02, pushData.values._m12, pushData.values._m22, pushData.values._m32); }
uint getMaterialIndex() { return asuint(pushData.values._m30); }

Texture2D<float4> albedoTexture : register(t0, space1);
RWTexture2D<float4> output : register(u0, space1);

float3 ACESFilm(float3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

[numthreads(8, 8, 1)]
void main_cs(uint3 id : SV_DispatchThreadID)
{
    float3 color = float3(0.0f, 0.0f, 0.0f);
    uint2 dims;
    albedoTexture.GetDimensions(dims.x, dims.y);
    if (id.x < dims.x && id.y < dims.y)
    {
        // Sample the albedo texture
        color = albedoTexture[id.xy].rgb;

        // Apply ACES tone mapping
        color = ACESFilm(color);
    }
    color += getF30() * 0.5f;
    output[id.xy] = float4(color, 1.0f);
}