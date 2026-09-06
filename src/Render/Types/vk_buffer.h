#pragma once
#include "Render/vk_common.h"
#include <cstdint>

struct Buffer {
    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    uint64_t address = 0;
    void* mapped = nullptr;

    VkResult map();
    void unmap();
    void destroy();
};

VkResult CreateBuffer(VkBufferUsageFlags usage, VkDeviceSize size, bool mappable, VmaMemoryUsage memory_usage, Buffer* pBuffer);
VkResult CopyBuffer(Buffer& src, Buffer& dst, VkDeviceSize size);
