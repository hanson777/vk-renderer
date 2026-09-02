#pragma once
#include "Render/vk_common.h"
#include <cstdint>

struct Buffer {
    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    uint64_t address = 0;
    void* mapped = nullptr;
};

Buffer CreateBuffer(VkBufferUsageFlags usage, size_t byte_size, bool mappable, VmaMemoryUsage memory_usage);
