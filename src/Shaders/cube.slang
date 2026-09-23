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

struct SceneRef 
{
    float4x4 mvp;
};

struct Material
{
    float4 base_color;
    uint texture_index;
};

struct MaterialBuffer
{
    Material materials[];
};

struct Instance
{
    float4x4 world_matrix;
    uint material_index;
};

struct InstanceBuffer
{
    Instance[] instances;
};

struct PushConstants 
{
    SceneRef* scene_ref;
    Vertex* vertices;
    MaterialBuffer* material_ref;
    InstanceBuffer* instance_ref;
};

[[vk::push_constant]]
ConstantBuffer<PushConstants> pc;

[shader("vertex")]
VertexOutput vertMain(uint vid : SV_VulkanVertexID)
{
    Vertex vert = pc.vertices[vid];

    VertexOutput output;
    output.sv_position = mul(pc.scene_ref.mvp, mul(pc.model, float4(vert.position, 1.0)));
    output.color = (vert.position + 1.0f) / 2.0f;

    return output;
}

[shader("fragment")]
float4 fragMain(VertexOutput inVert) : SV_Target 
{
    return float4(inVert.color, 1.0f);
}
