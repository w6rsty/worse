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
    float  deltaTime;
    float  time;
    float  cameraNear;
    float  cameraFar;

    float3 cameraPosition;
    float padding0;

    float3 cameraForward;
    float padding1;

    matrix view;
    matrix projection;
    matrix viewProjection;
    matrix viewProjectionInverse;
};

struct PushConstantData
{
    matrix transform;
    matrix values;
};

struct LightParameters
{
    float4 color;

    float3 position;
    float  intensity;

    float3 direction;
    float  range;

    float  angle;
    uint   flags;
    float2 padding;
};

// ================================================================================
// global set
// ================================================================================

cbuffer FrameConstantBuffer               : register(b0, space0) { FrameConstantData frameData; };
SamplerComparisonState samplerComparison  : register(s0, space0);
SamplerState samplers[8]                  : register(s1, space0);
Texture2D<float4> materialTextures[]      : register(t0, space0);

[[vk::push_constant]] PushConstantData pushData;

matrix getMatrix()      { return pushData.values; }
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

struct MaterialParameters
{
    uint   baseColorTextureIndex;
    uint   normalTextureIndex;
    uint   metallicRoughnessTextureIndex;
    uint   ambientOcclusionTextureIndex;

    uint   emissiveTextureIndex;
    float  metallic;
    float  roughness;
    float  ambientOcclusion;
    
    float4 baseColor;

    float4 emissive;

    uint   flags;
    uint   padding[3];
};

bool HasBaseColorTexture(MaterialParameters params)         { return (params.flags & (1 << 0)) != 0; }
bool HasNormalTexture(MaterialParameters params)            { return (params.flags & (1 << 1)) != 0; }
bool HasMetallicRoughnessTexture(MaterialParameters params) { return (params.flags & (1 << 2)) != 0; }
bool HasAmbientOcclusionTexture(MaterialParameters params)  { return (params.flags & (1 << 3)) != 0; }
bool HasEmissiveTexture(MaterialParameters params)          { return (params.flags & (1 << 4)) != 0; }

// Constants
static float const PI          = 3.14159265359f;
static float const FLT_MIN     = 0.00000001f;
static float const FLT_MAX_16  = 32767.0f;
static float const FLT_MAX_16U = 65535.0f;

// Saturation
float  saturate_16(float x)  { return clamp(x, 0.0f, FLT_MAX_16U); }
float2 saturate_16(float2 x) { return clamp(x, 0.0f, FLT_MAX_16U); }
float3 saturate_16(float3 x) { return clamp(x, 0.0f, FLT_MAX_16U); }
float4 saturate_16(float4 x) { return clamp(x, 0.0f, FLT_MAX_16U); }

float3 ScreenToWorldPosition(float2 uv, float depth)
{
    float x              = uv.x * 2.0f - 1.0f;
    float y              = (1.0f - uv.y) * 2.0f - 1.0f;
    float4 positionClip  = float4(x, y, depth, 1.0f);
    float4 positionWorld = mul(frameData.viewProjectionInverse, positionClip);
    return positionWorld.xyz / positionWorld.w;
}

#include "CommonStructs.hlsl"

#endif // WS_COMMON_HLSL