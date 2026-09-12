struct Vertex
{
    float3 position;
    float3 normal;
    float2 uv;
    float3 color;
};

struct VertexOutput
{
    float3 color;
    float4 sv_position : SV_Position;
};

struct MatrixRef 
{
    float4x4 mvp;
};

struct PushConstants 
{
    MatrixRef* matrix_ref;
    Vertex* vertices;
};

[[vk::push_constant]]
ConstantBuffer<PushConstants> pc;

[shader("vertex")]
VertexOutput vertMain(uint vid : SV_VulkanVertexID)
{
    Vertex vert = pc.vertices[vid];

    VertexOutput output;
    output.sv_position = mul(pc.matrix_ref.mvp, float4(vert.position, 1.0));
    output.color = (vert.position + 1.0f) / 2.0f;

    return output;
}

[shader("fragment")]
float4 fragMain(VertexOutput inVert) : SV_Target 
{
    return float4(inVert.color, 1.0f);
}
