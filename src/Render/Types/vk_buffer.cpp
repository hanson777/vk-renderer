#include "Render/Types/vk_buffer.h"
#include "Render/vk_common.h"
#include "Render/Managers/vk_memory.h"
#include "Render/Managers/vk_device.h"

VkResult Buffer::map() {
    return vmaMapMemory(vk_memory::GetAllocator(), allocation, &mapped);
}

void Buffer::unmap() {
    if (mapped != nullptr) {
        return vmaUnmapMemory(vk_memory::GetAllocator(), allocation);
        mapped = nullptr;
    }
}

void Buffer::destroy() {
    if (buffer) {
        vmaDestroyBuffer(vk_memory::GetAllocator(), buffer, allocation);
        buffer = VK_NULL_HANDLE;
    }
}

VkResult CreateBuffer(VkBufferUsageFlags usage, size_t size, bool mappable, VmaMemoryUsage memory_usage, Buffer* pBuffer) {
    VkBufferCreateInfo buffer_create_info{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage, 
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    VmaAllocationCreateInfo alloc_create_info{
        .flags = mappable ? VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT : 0u,
        .usage = memory_usage,
    };

    if (VkResult result = vmaCreateBuffer(vk_memory::GetAllocator(), &buffer_create_info, &alloc_create_info, &pBuffer->buffer, &pBuffer->allocation, nullptr); result != VK_SUCCESS) {
        return result;
    }

    if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        VkBufferDeviceAddressInfo address_info{
            .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
            .buffer = pBuffer->buffer,
        };
        pBuffer->address = vkGetBufferDeviceAddress(vk_device::GetDevice(), &address_info);
    }
    return VK_SUCCESS;
}
