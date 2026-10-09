struct Vertex
{
    float3 position;
    float3 normal;
    float2 uv;
    float3 color;
};

struct VertexOutput
{
    float4 sv_position : SV_Position;
    float2 uv : TEXCOORD0;
    nointerpolation uint texture_index : TEXCOORD1;
    nointerpolation float4 base_color : COLOR0;
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

[[vk::binding(0,0)]]
Sampler2D<float4> textures[];

[shader("vertex")]
VertexOutput vertMain(uint vertex_id : SV_VulkanVertexID, uint instance_id : SV_VulkanInstanceID)
{
    Vertex vert = pc.vertices[vertex_id];
    Instance instance = pc.instance_ref.instances[instance_id];
    Material material = pc.material_ref.materials[instance.material_index];

    VertexOutput output;
    output.sv_position = mul(pc.scene_ref.mvp, mul(instance.world_matrix, float4(vert.position, 1.0)));

    output.uv = vert.uv;
    output.texture_index = material.texture_index;
    output.base_color = material.base_color;

    return output;
}

[shader("fragment")]
float4 fragMain(VertexOutput input) : SV_Target 
{
    float4 texel = textures[NonUniformResourceIndex(input.texture_index)].Sample(input.uv);

    return texel * input.base_color;
}
