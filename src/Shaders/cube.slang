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
    float3 color;
    float4 sv_position : SV_Position;
};

struct PushConstants {
    float4x4 mvp;
}

[[vk::push_constant]]
ConstantBuffer<PushConstants> pc;

[shader("vertex")]
VertexOutput vertMain(uint vid : SV_VertexID) {
    VertexOutput output;
    float3 pos = positions[vid];
    output.sv_position = mul(pc.mvp, float4(pos, 1.0));
    output.color = (pos + 1.0f) / 2.0f;
    return output;
}

[shader("fragment")]
float4 fragMain(VertexOutput inVert) : SV_Target {
    return float4(inVert.color, 1.0f);
}