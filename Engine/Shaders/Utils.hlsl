#ifndef UTILS_HLSL
#define UTILS_HLSL

float FastSqrt(float x)
{
    return (float)(asfloat(0x1fbd1df5 + (asint(x) >> 1)));
}

float3x3 MakeTangentToWorldMatrix(float3 normal, float3 tangent)
{
    // re-orthogonalize T with respect to N
    tangent = normalize(tangent - dot(tangent, normal) * normal);
    // compute bitangent
    float3 bitangent = cross(normal, tangent);
    // create matrix
    return float3x3(tangent, bitangent, normal);
}

// =============================================================================
// PBR
// =============================================================================

// Fresnel-Schlick approximation
float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// Normal Distribution Function (GGX/Trowbridge-Reitz)
float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return num / denom;
}

// Geometry function
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    
    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    
    return num / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}

// =============================================================================
// Tonemapping
// =============================================================================

float3 ACESFilm(float3 x)
{
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

// =============================================================================
// Color Space
// =============================================================================

float3 SRGBToLinear(float3 color)
{
    float3 linearLow  = color / 12.92;
    float3 linearHigh = pow((color + 0.055) / 1.055, 2.4f);
    float3 isHigh     = step(0.0404482362771082, color);
    return lerp(linearLow, linearHigh, isHigh);
}

// =============================================================================
// Luminance
// =============================================================================

// 在 SRGB 色彩空间中的亮度加权因子
static float3 const SRGB_COLOR_SPACE_COEFFICIENT = float3(0.299f, 0.587f, 0.114f);

// Convert color to luminance
float Luminance(float3 color)
{
    return max(dot(color, SRGB_COLOR_SPACE_COEFFICIENT), FLT_MIN);
}


#endif