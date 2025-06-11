struct FrameConstantData
{
    float deltaTime;
    float time;
};

cbuffer BufferFrame : register(b0)
{
    FrameConstantData bufferFrame;
};

struct VertexInput
{
    float4 position : POSITION;
};

struct VertexOutput
{
    float4 position : SV_Position;
};

VertexOutput main_vs(VertexInput input)
{
    VertexOutput output;
    
    output.position = float4(input.position.xy, 0.0, 1.0);

    return output;
}

static const float EXPOSURE = 1.2;

float3 ACESFilm(float3 x)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

float random(float2 st)
{
    return frac(sin(dot(st, float2(12.9898, 78.233))) * 43758.5453123);
}

float noise(float2 st)
{
    float2  i = floor(st);
    float2  f = frac (st);

    float a = random(i);
    float b = random(i + float2(1.0, 0.0));
    float c = random(i + float2(0.0, 1.0));
    float d = random(i + float2(1.0, 1.0));

    float2 u = f * f * (3.0 - 2.0 * f);
    return lerp(a, b, u.x) +
           (c - a) * u.y * (1.0 - u.x) +
           (d - b) * u.x * u.y;
}

static const int OCTAVES = 8;
float fbm(float2 st)
{
    float value     = 0.0;
    float amplitude = 0.5;

    [unroll]
    for (int i = 0; i < OCTAVES; ++i)
    {
        value     += amplitude * noise(st);
        st        *= 2.0;
        amplitude *= 0.5;
    }
    return value;
}

float sdBox(float2 p, float2 b)
{
    float2 d = abs(p) - b;
    return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0);
}

float opSmoothUnion(float d1, float d2, float k)
{
    float h = saturate(0.5 + 0.5 * (d2 - d1) / k);
    return lerp(d2, d1, h) - k * h * (1.0 - h);
}

float smoothstep(float a, float b, float x)
{
    float t = saturate((x - a) / (b - a));
    return t * t * (3.0 - 2.0 * t);
}

float3 Shade(float2 uv, float t)
{
    // 1) 轮廓 SDF
    float sdf = sdBox(uv, float2(0.2, 0.4));

    // 2) 边缘扰动
    float edgeNoise = fbm(uv * 5.0 + t) * 0.1;
    float finalSDF  = sdf - edgeNoise;

    // 3) 遮罩
    float mask = 1.0 - smoothstep(0.0, 0.05, finalSDF);

    // 4) UV 扭曲 & 内部能量纹理
    float2 distortion_uv  = uv * 3.0;
    float2 distortion_off = float2(fbm(distortion_uv + t),
                                   fbm(distortion_uv - t * 1.2));
    float2 distorted_uv   = uv + (distortion_off * 2.0 - 1.0) * 0.2;

    float innerPattern = fbm(distorted_uv * 6.0 + t * 2.0);
          innerPattern = pow(innerPattern, 3.0) * 2.0;
    // float innerPattern = 1.0;

    // 5) 颜色
    float3 baseColor   = float3(0.1, 0.1, 0.1);
    float3 hotColor    = float3(0.8, 0.7, 0.1);
    float3 shadowColor = baseColor + innerPattern * hotColor;

    float3 bgColor = float3(0.1, 0.1, 0.1) * (1.0 - length(uv) * 0.5);

    return lerp(bgColor, shadowColor, mask);
}

float4 main_ps(VertexOutput pin) : SV_Target
{
    float iTime = bufferFrame.time;
    // static const float iTime = 0.0;
    static const float2 iResolution = float2(800.0, 600.0);

    float2 fragCoord = pin.position.xy;
    float2 uv        = (fragCoord - 0.5 * iResolution) / iResolution.y;
    float  t         = iTime * 0.3;

    // A) 基础色
    float edgeNoise = fbm(uv * 5.0 + t) * 0.1;
    float sdf       = sdBox(uv, float2(0.2, 0.4)) - edgeNoise;

    float3 finalColor = Shade(uv, t);

    // B) Chromatic Aberration（靠近轮廓）
    float2 grad    = float2(ddx(sdf), ddy(sdf));
    float2 dir     = normalize(grad + 1e-6);   // 防止除 0
    float  edgeMix = 1.0 - smoothstep(0.0, 0.08, abs(sdf));
    float  shift   = 0.01;

    float3 caColor;
    caColor.r = Shade(uv + dir * shift, t).r;   // R → 外
    caColor.g = finalColor.g;                   // G 不动
    caColor.b = Shade(uv - dir * shift, t).b;   // B → 内

    finalColor = lerp(finalColor, caColor, edgeMix);

    // C) HDR & Tone Mapping
    float3 hdr = finalColor * EXPOSURE;
    float3 ldr = ACESFilm(hdr);

    float dither = random(pin.position.xy) * (1.0 / 255.0);
    ldr += dither;
    ldr = pow(ldr, 1.0 / 2.0);

    return float4(ldr, 1.0);
}