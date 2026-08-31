#pragma once
#include "Render/vk_common.h"

struct Buffer {
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceAddress address = 0;
    VmaAllocation allocation = VK_NULL_HANDLE;
};

Buffer CreateBuffer(VkBufferUsageFlags usage, size_t byte_size, bool writable, VmaMemoryUsage memory_usage);
