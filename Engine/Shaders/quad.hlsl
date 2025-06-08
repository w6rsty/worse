struct VertexInput
{
    uint vertexID : SV_VertexID;
};

struct VertexOutput
{
    float4 position : SV_Position;
    float3 color : COLOR;
};

VertexOutput main_vs(VertexInput input)
{
    VertexOutput output;
    
    // Hard-coded triangle vertices
    float2 positions[3] = {
        float2(0.0, 0.5),   // Top
        float2(-0.5, -0.5), // Bottom left
        float2(0.5, -0.5)   // Bottom right
    };
    
    // Hard-coded colors for interpolation
    float3 colors[3] = {
        float3(1.0, 0.0, 0.0), // Red
        float3(0.0, 1.0, 0.0), // Green
        float3(0.0, 0.0, 1.0)  // Blue
    };
    
    output.position = float4(positions[input.vertexID], 0.0, 1.0);
    output.color = colors[input.vertexID];
    
    return output;
}

float4 main_ps(VertexOutput input) : SV_Target
{
    return float4(input.color, 1.0);
}