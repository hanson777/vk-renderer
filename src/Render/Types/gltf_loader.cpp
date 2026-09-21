#include "gltf_loader.h"
#include "Render/Types/Vertex.h"
#include "Render/Types/vk_image.h"
#include "Render/Types/Texture.h"
#include "Render/Types/Material.h"
#include "Render/Managers/vk_device.h"
#include "Render/Managers/vk_memory.h"
#include <tiny_gltf_v3.h>
#include <stb_image.h>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <cstdint>
#include <iostream>
#include <unordered_map>
#include <tuple>

void GltfLoader::loadGltf(const std::string& filename, SceneResources& scene_resources) {
    tg3_parse_options opts;
    tg3_error_stack errors;
    tg3_model model;

    tg3_parse_options_init(&opts);
    tg3_error_stack_init(&errors);

    tg3_error_code err = tg3_parse_file(&model, &errors, filename.c_str(), filename.length(), &opts);
    if (err != TG3_OK) {
        for (uint32_t i = 0; i < errors.count; i++) {
            std::cerr << "[ERROR::loadModels] " << "Severity: " << errors.entries[i].severity << ' ' <<
                    "Message: " << (errors.entries[i].message ? errors.entries[i].message : "(null)") << '\n';
        }
    }

    loadFallbacks(scene_resources);

    std::vector<ImageData> image_data = loadImageData(model, scene_resources);
    std::vector<uint32_t> image_ids = uploadImageData(model, image_data, scene_resources);

    std::vector<uint32_t> sampler_ids = loadSamplers(model, scene_resources);
    std::vector<uint32_t> texture_ids = loadTextures(model, image_ids, sampler_ids, scene_resources);
    std::vector<uint32_t> material_ids = loadMaterials(model, texture_ids, scene_resources);
    std::vector<uint32_t> mesh_ids = loadMeshes(model, material_ids, scene_resources);

    const tg3_scene* scene = &model.scenes[model.default_scene != -1 ? model.default_scene : 0];

    Tree& tree = scene_resources.getTree();
    tree.init(model.nodes_count);

    uint32_t synthetic_root_node_id = tree.createNode();
    tree.m_root_node_id = synthetic_root_node_id;
    tree.m_last_root_node_id = synthetic_root_node_id;

    uint32_t last_child_id = UINT32_MAX;
    for (uint32_t i = 0; i < scene->nodes_count; i++) {
        uint32_t child_id = importNode(tree, model, scene->nodes[i], synthetic_root_node_id, last_child_id, mesh_ids);

        if (tree.getNode(synthetic_root_node_id)->getFirstChildId() == UINT32_MAX) {
            tree.getNode(synthetic_root_node_id)->setFirstChildId(child_id);
        }
        last_child_id = child_id;
    }

    tg3_model_free(&model);
    tg3_error_stack_free(&errors);
}

void GltfLoader::loadFallbacks(SceneResources& scene_resources) {
    std::vector<Image>& images = scene_resources.getImages();
    std::vector<VkSampler>& samplers = scene_resources.getSamplers();
    std::vector<Texture>& textures = scene_resources.getTextures();
    std::vector<Material>& materials = scene_resources.getMaterials();

    uint32_t fallback_image_id = scene_resources.getFallbackImageId();
    uint32_t fallback_sampler_id = scene_resources.getFallbackSamplerId();
    uint32_t fallback_texture_id = scene_resources.getFallbackTextureId();
    uint32_t fallback_material_id = scene_resources.getFallbackMaterialId();

    if (fallback_image_id == UINT32_MAX) {
        fallback_image_id = static_cast<uint32_t>(images.size());
        uint8_t white_pixel_data[4] = {255, 255, 255, 255};
        images.push_back(createImage(&white_pixel_data[0], 1, 1, 4));
        scene_resources.setFallbackImageId(fallback_image_id);
    }

    if (fallback_sampler_id == UINT32_MAX) {
        VkSamplerCreateInfo sampler_ci{
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = VK_FILTER_LINEAR,
            .minFilter = VK_FILTER_LINEAR,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .compareEnable = VK_FALSE,
        };
        VkSampler sampler;
        VK_CHECK(vkCreateSampler(vk_device::GetDevice(), &sampler_ci, nullptr, &sampler));
        fallback_sampler_id = static_cast<uint32_t>(samplers.size());
        samplers.push_back(sampler);
        scene_resources.setFallbackSamplerId(fallback_sampler_id);
    }
    
    if (fallback_texture_id == UINT32_MAX) {
        fallback_texture_id = static_cast<uint32_t>(textures.size());
        textures.push_back({.image_id = fallback_image_id, .sampler_id = fallback_sampler_id});
        scene_resources.setFallbackTextureId(fallback_texture_id);
    }

    if (fallback_material_id == UINT32_MAX) {
        fallback_material_id = static_cast<uint32_t>(materials.size());
        materials.push_back({.base_color = glm::vec4(1.0f), .texture_index = fallback_texture_id});
        scene_resources.setFallbackMaterialId(fallback_material_id);
    }
}

std::vector<ImageData> GltfLoader::loadImageData(const tg3_model& model, SceneResources& scene_resources) {
    std::vector<ImageData> image_datas(model.images_count);
    std::vector<uint8_t>& image_buffer = scene_resources.getImageBuffer();
    
    for (int i = 0; i < model.images_count; i++) {
        const tg3_str& uri = model.images[i].uri;
        std::string image_path(uri.data, uri.len);
        int width, height, channels;
        uint8_t* data = stbi_load(image_path.c_str(), &width, &height, &channels, 4);
        if (data == nullptr) {
            std::cerr << "[ERROR::loadImageData] failed to load image at path: " << image_path << ", using white pixel\n";
            uint8_t white_pixel_data[4] = { 255, 255, 255, 255 };
            size_t offset = image_buffer.size();
            image_buffer.insert(image_buffer.end(), white_pixel_data, white_pixel_data + 4);
            image_datas[i] = {
                .offset = offset,
                .size = 4,
                .width = 1,
                .height = 1,
                .channels = 4,
                .id = static_cast<uint32_t>(i),
            };

            continue;
        }

        size_t size = width * height * 4;
        size_t offset = image_buffer.size();
        image_buffer.insert(image_buffer.end(), data, data + size);

        stbi_image_free(data);

        image_datas[i] = {
            .offset = offset,
            .size = size,
            .width = static_cast<uint32_t>(width),
            .height = static_cast<uint32_t>(height),
            .channels = 4,
            .id = static_cast<uint32_t>(i),
        };
    }

    return image_datas;
}

std::vector<uint32_t> GltfLoader::uploadImageData(const tg3_model& model, const std::vector<ImageData>& image_data, SceneResources& scene_resources) {
    std::vector<uint8_t>& image_buffer = scene_resources.getImageBuffer();
    std::vector<Image>& images = scene_resources.getImages();
    std::vector<uint32_t> image_ids(image_data.size());
    Buffer imgs_staging = createBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, image_buffer.size(), true, VMA_MEMORY_USAGE_AUTO_PREFER_HOST);
    imgs_staging.map();
    memcpy(imgs_staging.mapped, image_buffer.data(), image_buffer.size());
    imgs_staging.unmap();

    VkCommandPool cmd_pool = createCommandPool(vk_device::GetQueueIndex(), VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
    VkCommandBuffer cmd_buffer = createCommandBuffer(cmd_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    for (int i = 0; i < image_data.size(); i++) {
        const ImageData& raw_image = image_data[i];
        image_ids[i] = raw_image.id;

        VkFormat image_format = VK_FORMAT_R8G8B8A8_SRGB;
        VkImageCreateInfo image_ci{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = image_format,
            .extent{.width = raw_image.width, .height = raw_image.height, .depth = 1},
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };

        VmaAllocationCreateInfo alloc_ci{ .usage = VMA_MEMORY_USAGE_AUTO };
        Image image;
        VK_CHECK(vmaCreateImage(vk_memory::GetAllocator(), &image_ci, &alloc_ci, &image.image, &image.allocation, nullptr));

        VkImageViewCreateInfo image_view_ci{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = image.image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = image_format,
            .subresourceRange{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .levelCount = 1,
                .layerCount = 1,
            },
        };
        VK_CHECK(vkCreateImageView(vk_device::GetDevice(), &image_view_ci, nullptr, &image.image_view));

        VkImageMemoryBarrier2 transfer_barrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_NONE,
                .srcAccessMask = VK_ACCESS_2_NONE,
                .dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
                .dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .image = image.image,
                .subresourceRange{
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                }
            };
            VkDependencyInfo transfer_dep_info{
                .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                .imageMemoryBarrierCount = 1,
                .pImageMemoryBarriers = &transfer_barrier
            };
            vkCmdPipelineBarrier2(cmd_buffer, &transfer_dep_info);

            VkBufferImageCopy image_copy{
                .bufferOffset = raw_image.offset,
                .imageSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1},
                .imageExtent = {.width = raw_image.width, .height = raw_image.height, .depth = 1},
            };
            vkCmdCopyBufferToImage(cmd_buffer, imgs_staging.buffer, image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &image_copy);

            VkImageMemoryBarrier2 shader_read_barrier{
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
                .srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                .image = image.image,
                .subresourceRange
                {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                }
            };
            VkDependencyInfo shader_read_dep_info{
                .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                .imageMemoryBarrierCount = 1,
                .pImageMemoryBarriers = &shader_read_barrier,
            };
            vkCmdPipelineBarrier2(cmd_buffer, &shader_read_dep_info);
            images.push_back(image);
    }

    flushCommandBuffer(cmd_buffer, vk_device::GetQueue(), cmd_pool, true);
    vkDestroyCommandPool(vk_device::GetDevice(), cmd_pool, nullptr);
    return image_ids;
}

std::vector<uint32_t> GltfLoader::loadSamplers(const tg3_model& model, SceneResources& scene_resources) {
    std::vector<uint32_t> sampler_ids(model.samplers_count);
    std::vector<Texture>& textures = scene_resources.getTextures();
    std::vector<VkSampler>& samplers = scene_resources.getSamplers();
    for (int i = 0; i < model.samplers_count; i++) {
        const tg3_sampler& tg3sampler = model.samplers[i];

		static const std::unordered_map<int32_t, std::tuple<VkFilter, VkSamplerMipmapMode, float>> filter_map{
			{ TG3_TEXTURE_FILTER_NEAREST, { VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST, 0.25f } },
			{ TG3_TEXTURE_FILTER_LINEAR, { VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, 0.25f } },
			{ TG3_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR, { VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_LOD_CLAMP_NONE } },
			{ TG3_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST, { VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_LOD_CLAMP_NONE } },
			{ TG3_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR, { VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_LOD_CLAMP_NONE } },
			{ TG3_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST, { VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_LOD_CLAMP_NONE } }
		};
		static const std::unordered_map<int32_t, VkSamplerAddressMode> wrap_map{
			{ TG3_TEXTURE_WRAP_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT },
			{ TG3_TEXTURE_WRAP_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE },
			{ TG3_TEXTURE_WRAP_MIRRORED_REPEAT, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT }
		};

		VkSamplerCreateInfo sampler_info{
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.magFilter = (tg3sampler.mag_filter == -1) ? VK_FILTER_LINEAR : std::get<0>(filter_map.at(tg3sampler.mag_filter)),
			.minFilter = (tg3sampler.min_filter == -1) ? VK_FILTER_LINEAR : std::get<0>(filter_map.at(tg3sampler.min_filter)),
			.mipmapMode = (tg3sampler.min_filter == -1) ? VK_SAMPLER_MIPMAP_MODE_LINEAR : std::get<1>(filter_map.at(tg3sampler.min_filter)),
			.addressModeU = (tg3sampler.wrap_s == -1) ? VK_SAMPLER_ADDRESS_MODE_REPEAT : wrap_map.at(tg3sampler.wrap_s),
			.addressModeV = (tg3sampler.wrap_t == -1) ? VK_SAMPLER_ADDRESS_MODE_REPEAT : wrap_map.at(tg3sampler.wrap_t),
			.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.compareEnable = VK_FALSE,
			.minLod = 0.0f,
			.maxLod = (tg3sampler.min_filter == -1) ? VK_LOD_CLAMP_NONE : std::get<2>(filter_map.at(tg3sampler.min_filter))
		};

		VkSampler sampler;
		if (vkCreateSampler(vk_device::GetDevice(), &sampler_info, nullptr, &sampler) != VK_SUCCESS) {
            std::cerr << "[ERROR::loadSamplers] Unable to create texture sampler, using fallback\n";
			sampler_ids[i] = scene_resources.getFallbackSamplerId();
		} else {
            uint32_t sampler_id = static_cast<uint32_t>(samplers.size());
            samplers.push_back(sampler);
            sampler_ids[i] = sampler_id;
        }
	}
    return sampler_ids;
}

std::vector<uint32_t> GltfLoader::loadTextures(const tg3_model& model, const std::vector<uint32_t>& image_ids, const std::vector<uint32_t> sampler_ids,  SceneResources& scene_resources) {
    std::vector<uint32_t> texture_ids(model.textures_count);
    std::vector<Texture>& textures = scene_resources.getTextures();
    for (int i = 0; i < model.textures_count; i++) {
       const tg3_texture& tex = model.textures[i];
       uint32_t texture_id = static_cast<uint32_t>(textures.size());
       textures.push_back({
               .image_id = tex.source != -1 ? image_ids[tex.source] : scene_resources.getFallbackImageId(),
               .sampler_id = tex.sampler != -1 ? sampler_ids[tex.sampler] : scene_resources.getFallbackSamplerId(),
               });
       texture_ids[i] = texture_id;
    }
    return texture_ids;
}

std::vector<uint32_t> GltfLoader::loadMaterials(const tg3_model& model, const std::vector<uint32_t>& texture_ids, SceneResources& scene_resources) {
    std::vector<uint32_t> material_ids(model.materials_count);
    std::vector<Material>& materials = scene_resources.getMaterials();
    for (int i = 0; i < model.materials_count; i++) {
        const tg3_material* tg3mat = &model.materials[i];
        uint32_t material_id = static_cast<uint32_t>(materials.size());
        materials.push_back({
                    .base_color = glm::vec4(tg3mat->pbr_metallic_roughness.base_color_factor[0],
                                            tg3mat->pbr_metallic_roughness.base_color_factor[1],
                                            tg3mat->pbr_metallic_roughness.base_color_factor[2],
                                            tg3mat->pbr_metallic_roughness.base_color_factor[3]),
                    .texture_index = tg3mat->pbr_metallic_roughness.base_color_texture.index != -1
                                         ? texture_ids[tg3mat->pbr_metallic_roughness.base_color_texture.index]
                                         : scene_resources.getFallbackTextureId(),
                });
        material_ids[i] = material_id;
    }

    size_t mats_size = sizeof(materials[0]) * materials.size();
    Buffer mat_staging = createBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, mats_size, true, VMA_MEMORY_USAGE_AUTO);
    mat_staging.map();
    memcpy(mat_staging.mapped, materials.data(), mats_size); 
    mat_staging.unmap();

    Buffer mat_buffer = createBuffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, mats_size, false, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
    copyBuffer(mat_staging, mat_buffer, mats_size);

    uint32_t mat_buffer_id = scene_resources.addBuffer(std::move(mat_buffer));
    scene_resources.setMaterialBufferId(mat_buffer_id);

    return material_ids;
}

std::vector<uint32_t> GltfLoader::loadMeshes(const tg3_model& model, const std::vector<uint32_t>& material_ids, SceneResources& scene_resources) {
    std::vector<uint32_t> mesh_ids(model.meshes_count);
    uint32_t num_vertices = 0;
    uint32_t num_indices = 0;
    for (uint32_t m = 0; m < model.meshes_count; m++) {
        const tg3_mesh* tg3_mesh = &model.meshes[m];
        for (uint32_t p = 0; p < tg3_mesh->primitives_count; p++) {
            const tg3_primitive* primitive = &tg3_mesh->primitives[p];

            if (primitive->indices != -1) {
                const tg3_accessor* accessor = &model.accessors[primitive->indices];
                num_indices += accessor->count;
            }

            if (primitive->attributes_count > 0) {
                const tg3_str_int_pair* attrib = &primitive->attributes[0];
                const tg3_accessor* accessor = &model.accessors[attrib->value];
                num_vertices += accessor->count;
            }
        }
    }

    std::vector<Vertex>& vertices = scene_resources.getVertices();
    std::vector<uint32_t>& indices = scene_resources.getIndices();

    vertices.reserve(num_vertices);
    indices.reserve(num_indices);

    for (uint32_t m = 0; m < model.meshes_count; m++) {
        const tg3_mesh* tg3_mesh = &model.meshes[m];
        Mesh mesh;
        mesh.primitives.resize(tg3_mesh->primitives_count);

        for (uint32_t p = 0; p < tg3_mesh->primitives_count; p++) {
            const tg3_primitive* primitive = &tg3_mesh->primitives[p];
            mesh.primitives[p].material_id = primitive->material != -1 ?  material_ids[primitive->material] : scene_resources.getFallbackMaterialId();
            mesh.primitives[p].vertex_start = static_cast<uint32_t>(vertices.size());

            if (primitive->indices != -1) {
                const tg3_accessor* accessor = &model.accessors[primitive->indices];
                const tg3_buffer_view* buffer_view = &model.buffer_views[accessor->buffer_view];
                const tg3_buffer* buffer = &model.buffers[buffer_view->buffer];

                mesh.primitives[p].first_index = static_cast<uint32_t>(indices.size());
                mesh.primitives[p].index_count = accessor->count;

                if (accessor->component_type == TG3_COMPONENT_TYPE_UNSIGNED_INT) {
                    const uint32_t* data = reinterpret_cast<const uint32_t*>(buffer->data.data + buffer_view->byte_offset + accessor->byte_offset);
                    indices.insert(indices.end(), data, data + accessor->count);
                } 
                else if (accessor->component_type == TG3_COMPONENT_TYPE_UNSIGNED_SHORT) {
                    const uint16_t* data = reinterpret_cast<const uint16_t*>(buffer->data.data + buffer_view->byte_offset + accessor->byte_offset);
                    for (uint64_t i = 0; i < accessor->count; i++) {
                        indices.push_back(static_cast<uint32_t>(data[i]));
                    }
                }
            }

            struct Attribute { size_t offset; size_t stride; const std::byte* start; int num_floats; };
            std::vector<Attribute> attribs(primitive->attributes_count);
            for (uint32_t a = 0; a < primitive->attributes_count; a++) {
                const tg3_str_int_pair* attrib = &primitive->attributes[a];
                const tg3_accessor* accessor = &model.accessors[attrib->value];
                mesh.primitives[p].vertex_count = accessor->count;

                size_t field_offset = 0;
                int float_count = 0;
                if (strcmp(attrib->key.data, "POSITION") == 0) {
                    field_offset = offsetof(Vertex, position);
                    float_count = 3;
                } 
                else if (strcmp(attrib->key.data, "NORMAL") == 0) {
                    field_offset = offsetof(Vertex, normal);
                    float_count = 3;
                } 
                else if (strcmp(attrib->key.data, "TEXCOORD_0") == 0) {
                    field_offset = offsetof(Vertex, uv);
                    float_count = 2;
                } 
                else if(strcmp(attrib->key.data, "COLOR_0") == 0) {
                    assert(accessor->type == TG3_TYPE_VEC3);
                    field_offset = offsetof(Vertex, color);
                    float_count = 3;
                } 
                else continue;

                const tg3_buffer_view* buffer_view = &model.buffer_views[accessor->buffer_view];
                const tg3_buffer* buffer = &model.buffers[buffer_view->buffer];

                attribs[a].offset = field_offset; 
                attribs[a].stride = buffer_view->byte_stride != 0 ? buffer_view->byte_stride : sizeof(float) * float_count;
                attribs[a].start = reinterpret_cast<const std::byte*>(buffer->data.data + buffer_view->byte_offset + accessor->byte_offset);
                attribs[a].num_floats = float_count;
            }

            for (int v = 0; v < mesh.primitives[p].vertex_count; v++) {
                Vertex vertex;
                for (const Attribute& attrib : attribs) {
                    const void* src = reinterpret_cast<const void*>(attrib.start + (v * attrib.stride));
                    void* dst = reinterpret_cast<std::byte*>(&vertex) + attrib.offset;
                    memcpy(dst, src, attrib.num_floats * sizeof(float));
                }
                vertices.push_back(vertex);
            }
        }
        std::vector<Mesh>& meshes = scene_resources.getMeshes();
        uint32_t mesh_id = meshes.size();
        meshes.push_back(std::move(mesh));
        mesh_ids[m] = mesh_id;
    } 

    size_t verts_size = sizeof(vertices[0]) * vertices.size();
    size_t indices_size = sizeof(indices[0]) * indices.size();
    Buffer vertex_staging = createBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, verts_size, true, VMA_MEMORY_USAGE_AUTO_PREFER_HOST);
    Buffer index_staging = createBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, indices_size, true, VMA_MEMORY_USAGE_AUTO_PREFER_HOST);
    vertex_staging.map();
    index_staging.map();
    memcpy(vertex_staging.mapped, vertices.data(), verts_size); 
    memcpy(index_staging.mapped, indices.data(), indices_size); 
    vertex_staging.unmap();
    index_staging.unmap();

    Buffer vertex_buffer = createBuffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, verts_size, false, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
    Buffer index_buffer = createBuffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indices_size, false, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
    
    copyBuffer(vertex_staging, vertex_buffer, verts_size);
    copyBuffer(index_staging, index_buffer, indices_size);

    uint32_t vert_id = scene_resources.addBuffer(std::move(vertex_buffer));
    uint32_t index_id = scene_resources.addBuffer(std::move(index_buffer));
    scene_resources.setVertexBufferId(vert_id);
    scene_resources.setIndexBufferId(index_id);

    return mesh_ids;
}

uint32_t GltfLoader::importNode(Tree& tree, const tg3_model& model, int32_t model_node_id, uint32_t parent_id, uint32_t prev_sibling_id, std::vector<uint32_t>& mesh_ids) {
    const tg3_node& tg3node = model.nodes[model_node_id];
    uint32_t node_id = tree.createNode();
    Node* node_ptr = tree.getNode(node_id);
    node_ptr->setParentId(parent_id); 

    if (tg3node.has_matrix) {
        node_ptr->setMatrix(glm::make_mat4(tg3node.matrix));
    }
    else {
        node_ptr->setTranslation(glm::vec3(tg3node.translation[0], tg3node.translation[1], tg3node.translation[2]));
        node_ptr->setRotation(glm::quat(tg3node.rotation[3], tg3node.rotation[0], tg3node.rotation[1], tg3node.rotation[2]));
        node_ptr->setScale(glm::vec3(tg3node.scale[0], tg3node.scale[1], tg3node.scale[2]));
    }

    if (tg3node.mesh != -1) {
        node_ptr->setMeshId(mesh_ids[tg3node.mesh]);
    }

    if (prev_sibling_id != UINT32_MAX) {
        tree.getNode(prev_sibling_id)->setNextSiblingId(node_id);
    }

    uint32_t last_child_id = UINT32_MAX;
    for (int i = 0; i < tg3node.children_count; i++) {
        int32_t child_index = tg3node.children[i];

        // don't store a pointer after this recursive call, vector allocations can invalidate pointers
        last_child_id = importNode(tree, model, child_index, node_id, last_child_id, mesh_ids);

        if (tree.getNode(node_id)->getFirstChildId() == UINT32_MAX) {
            tree.getNode(node_id)->setFirstChildId(last_child_id);
        }
    }

    return node_id;
}
