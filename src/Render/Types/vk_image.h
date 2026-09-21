#pragma once
#include "Render/vk_common.h"

struct ImageData {
    size_t offset = 0;
    size_t size = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    int channels = 0;
    uint32_t id = UINT32_MAX;
};

struct Image {
    VkImage image = VK_NULL_HANDLE;
    VkImageView image_view = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
};

Image createImage(uint8_t* image_data, uint32_t width, uint32_t height, int channels);
