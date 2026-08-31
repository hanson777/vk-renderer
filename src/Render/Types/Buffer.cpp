#pragma once
#include "Render/Types/Buffer.h"
#include "Render/vk_common.h"
#include "Render/Managers/vk_memory.h"
#include "Render/Managers/vk_device.h"
#include <iostream>

Buffer createBuffer(VkBufferUsageFlags usage, size_t size, bool staging, VmaMemoryUsage memory_usage) {
    VkBufferCreateInfo buffer_create_info{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage, 
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    VmaAllocationCreateInfo alloc_create_info{
        .flags = staging ? VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT : 0u,
        .usage = memory_usage,
    };

    Buffer buffer;
    if (vmaCreateBuffer(vk_memory::GetAllocator(), &buffer_create_info, &alloc_create_info, &buffer.buffer, &buffer.allocation, nullptr) != VK_SUCCESS) {
        std::cerr << "[ERROR::createBuffer] failed to create buffer\n";
        return buffer;
    }

    if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        VkBufferDeviceAddressInfo address_info{
            .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
            .buffer = buffer.buffer,
        };
        buffer.address = vkGetBufferDeviceAddress(vk_device::GetDevice(), &address_info);
    }

    return buffer;
}
