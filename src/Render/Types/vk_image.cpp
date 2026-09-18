#include "vk_image.h"
#include "Render/Managers/vk_memory.h"
#include "Render/Managers/vk_device.h"
#include "Render/vk_common.h"
#include "Render/Types/vk_buffer.h"
#include <cstring>

Image createImage(uint8_t* image_data, uint32_t width, uint32_t height, int num_channels) {
    VkFormat image_format = VK_FORMAT_R8G8B8A8_SRGB;
    VkImageCreateInfo image_ci{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = image_format,
        .extent{.width = width, .height = height, .depth = 1},
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
    VkCommandPool cmd_pool = createCommandPool(vk_device::GetQueueIndex(), VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
    VkCommandBuffer cmd_buffer = createCommandBuffer(cmd_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
	vkCmdPipelineBarrier2(cmd_buffer, &transfer_dep_info);

	size_t image_data_size = width * height * num_channels;
	Buffer image_staging = createBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, image_data_size, true, VMA_MEMORY_USAGE_AUTO_PREFER_HOST);
    image_staging.map();
    memcpy(image_staging.mapped, reinterpret_cast<const void*>(image_data), image_data_size);
    image_staging.unmap();

	VkBufferImageCopy image_copy{
		.imageSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1},
		.imageExtent = {.width = width, .height = height, .depth = 1},
	};
	vkCmdCopyBufferToImage(cmd_buffer, image_staging.buffer, image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &image_copy);

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
	VkDependencyInfo shader_read_dep_info
	{
		.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &shader_read_barrier,
	};
	vkCmdPipelineBarrier2(cmd_buffer, &shader_read_dep_info);

    flushCommandBuffer(cmd_buffer, vk_device::GetQueue(), cmd_pool, true);

    return image;
}
