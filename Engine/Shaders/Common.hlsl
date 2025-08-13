#ifndef WS_COMMON_HLSL
#define WS_COMMON_HLSL

static uint const samplerPointClampEdge      = 0;
static uint const samplerPointClampBorder    = 1;
static uint const samplerPointWrap           = 2;
static uint const samplerBilinearClampEdge   = 3;
static uint const samplerBilinearClampBorder = 4;
static uint const samplerBilinearWrap        = 5;
static uint const samplerTrilinearClamp      = 6;
static uint const samplerAnisotropicClamp    = 7;

static uint const THREAD_GROUP_COUNT_X = 8;
static uint const THREAD_GROUP_COUNT_Y = 8;

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
    float4x4 transform;
    float4x4 values;
};

// ================================================================================
// global set
// ================================================================================

cbuffer FrameConstantBuffer               : register(b0, space0) { FrameConstantData frameData; };
SamplerComparisonState samplerComparison  : register(s0, space0);
SamplerState samplers[8]                  : register(s1, space0);
Texture2D<float4> materialTextures[]      : register(t0, space0);

[[vk::push_constant]] PushConstantData pushData;

float2 getF2()          { return float2(pushData.values._m00, pushData.values._m10); }
float3 getF30()         { return float3(pushData.values._m20, pushData.values._m30, pushData.values._m01); }
float3 getF31()         { return float3(pushData.values._m11, pushData.values._m21, pushData.values._m31); }
float4 getF4()          { return float4(pushData.values._m02, pushData.values._m12, pushData.values._m22, pushData.values._m32); }
uint   getMaterialId()  { return pushData.values._m03; }
bool   getTransparent() { return pushData.values._m13 != 0; }
float2 getPadding()     { return float2(pushData.values._m23, pushData.values._m33); }

struct VertexPos
{
    float3 position : POSITION;
};

struct VertexPosCol
{
    float3 position : POSITION;
    float4 color    : COLOR;
};

struct VertexPosUv
{
    float3 position : POSITION;
    float2 uv       : TEXCOORD;
};

struct VertexPosUvNrmTan
{
    float3 position : POSITION;
    float2 uv       : TEXCOORD;
    float3 normal   : NORMAL;
    float4 tangent  : TANGENT;
};

struct Material
{
    uint baseColorTextureIndex;
    uint normalTextureIndex;
    uint metallicRoughnessTextureIndex;
    uint ambientOcclusionTextureIndex;
    uint emissiveTextureIndex;
    float metallic;
    float roughness;
    float ambientOcclusion;
    
    float4 baseColor;
    float4 emissive;
};

#endif // WS_COMMON_HLSL