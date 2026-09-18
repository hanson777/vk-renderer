#pragma once
#include "Render/vk_common.h"

struct Image {
    VkImage image = VK_NULL_HANDLE;
    VkImageView image_view = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
};

Image createImage(uint8_t* image_data, uint32_t width, uint32_t height, int num_channels);
