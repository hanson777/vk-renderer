static float3 positions[36] = float3[](
    // -X face
    float3(-0.5, -0.5, -0.5), float3(-0.5, -0.5, 0.5), float3(-0.5, 0.5, 0.5),
    float3(-0.5, -0.5, -0.5), float3(-0.5, 0.5, 0.5), float3(-0.5, 0.5, -0.5),

    // +X face
    float3(0.5, -0.5, 0.5), float3(0.5, -0.5, -0.5), float3(0.5, 0.5, -0.5),
    float3(0.5, -0.5, 0.5), float3(0.5, 0.5, -0.5), float3(0.5, 0.5, 0.5),

    // -Y face
    float3(-0.5, -0.5, -0.5), float3(0.5, -0.5, -0.5), float3(0.5, -0.5, 0.5),
    float3(-0.5, -0.5, -0.5), float3(0.5, -0.5, 0.5), float3(-0.5, -0.5, 0.5),

    // +Y face
    float3(-0.5, 0.5, 0.5), float3(0.5, 0.5, 0.5), float3(0.5, 0.5, -0.5),
    float3(-0.5, 0.5, 0.5), float3(0.5, 0.5, -0.5), float3(-0.5, 0.5, -0.5),

    // -Z face
    float3(0.5, -0.5, -0.5), float3(-0.5, -0.5, -0.5), float3(-0.5, 0.5, -0.5),
    float3(0.5, -0.5, -0.5), float3(-0.5, 0.5, -0.5), float3(0.5, 0.5, -0.5),

    // +Z face
    float3(-0.5, -0.5, 0.5), float3(0.5, -0.5, 0.5), float3(0.5, 0.5, 0.5),
    float3(-0.5, -0.5, 0.5), float3(0.5, 0.5, 0.5), float3(-0.5, 0.5, 0.5)
);

struct VertexOutput {
    float4 sv_position : SV_Position;
};

[shader("vertex")]
VertexOutput vertMain(uint vid : SV_VertexID) {
    VertexOutput output;
    output.sv_position = float4(positions[vid], 1.0);
    return output;
}

[shader("fragment")]
float4 fragMain(VertexOutput inVert) : SV_Target
{
    float3 color = inVert.sv_position.xyz;
    return float4(color, 1.0);
}