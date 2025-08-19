#ifndef SHADOW_MAPPING_HLSL
#define SHADOW_MAPPING_HLSL


float CalculateShadowPCF(Texture2D<float> shadowMap, float3 worldPos, matrix lightViewProjection, float bias)
{
    float4 lightSpacePos = mul(lightViewProjection, float4(worldPos, 1.0));

    float3 projCoords = lightSpacePos.xyz / lightSpacePos.w;

    projCoords.xy = projCoords.xy * 0.5 + 0.5;
    projCoords.y = 1.0 - projCoords.y; // 翻转Y轴 (DirectX风格)

    if (projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 ||
        projCoords.y > 1.0 || projCoords.z > 1.0 || projCoords.z < 0.0)
    {
        return 1.0;
    }

    float currentDepth = projCoords.z;

    float2 shadowMapSize;
    shadowMap.GetDimensions(shadowMapSize.x, shadowMapSize.y);
    float2 texelSize = 1.0 / shadowMapSize;

    float shadow         = 0.0;
    const int pcfRadius  = 1;
    const int pcfSamples = (2 * pcfRadius + 1) * (2 * pcfRadius + 1);

    // PCF sample
    for (int x = -pcfRadius; x <= pcfRadius; ++x)
    {
        for (int y = -pcfRadius; y <= pcfRadius; ++y)
        {
            float2 offset = float2(x, y) * texelSize;
            float2 sampleCoord = projCoords.xy + offset;

            float shadowDepth =
                shadowMap.SampleLevel(samplers[samplerPointClampEdge], sampleCoord, 0)
                    .r;

            if (currentDepth - bias <= shadowDepth) 
            {
                shadow += 1.0;
            }
        }
    }

  return shadow / float(pcfSamples);
}

// 计算阴影偏移以减少阴影失真
float CalculateShadowBias(float3 normal, float3 lightDirection, float baseBias, float maxBias)
{
    float cosTheta = saturate(dot(normal, lightDirection));
    float bias = baseBias + maxBias * (1.0 - cosTheta);
    return bias;
}


float ComputeShadow(Texture2D<float> shadowMap, Surface surface, LightParameters light)
{

    if (!(light.flags & (1 << 0)))
    {
          return 1.0;
    }


    matrix lightViewProjection = getMatrix();

    float3 lightDir = normalize(-light.direction);
    float bias = CalculateShadowBias(surface.normal, lightDir, 0.0005, 0.005);

    return CalculateShadowPCF(shadowMap, surface.positionWorld, lightViewProjection, bias);
}

#endif