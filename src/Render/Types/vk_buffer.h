#pragma once
#include "Render/vk_common.h"
#include <cstdint>

struct Buffer {
    Buffer() = default;

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    Buffer(Buffer&& other) noexcept; 
    Buffer& operator=(Buffer&& other) noexcept;

    ~Buffer();

    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    uint64_t address = 0;
    void* mapped = nullptr;

    void map();
    void unmap();

private:
    void destroy();
};

Buffer createBuffer(VkBufferUsageFlags usage, VkDeviceSize size, bool mappable, VmaMemoryUsage memory_usage);
void copyBuffer(Buffer& src, Buffer& dst, VkDeviceSize size);
