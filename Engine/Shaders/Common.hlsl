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
    uint materialIndex;
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