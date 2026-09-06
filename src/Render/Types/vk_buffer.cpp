#include "Render/Types/vk_buffer.h"
#include "Render/vk_common.h"
#include "Render/Managers/vk_memory.h"
#include "Render/Managers/vk_device.h"
#include "Render/Managers/vk_sync.h"
#include <iostream>

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

VkResult CreateBuffer(VkBufferUsageFlags usage, VkDeviceSize size, bool mappable, VmaMemoryUsage memory_usage, Buffer* pBuffer) {
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

VkResult CopyBuffer(Buffer& src, Buffer& dst, VkDeviceSize size) {
    VkCommandPoolCreateInfo cmd_pool_ci{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
        .queueFamilyIndex = vk_device::GetQueueIndex(),
    };
    
    const VkDevice& device = vk_device::GetDevice();
    VkCommandPool cmd_pool;
    if (VkResult result = vkCreateCommandPool(device, &cmd_pool_ci, nullptr, &cmd_pool); result != VK_SUCCESS) {
        return result;
    };

    VkCommandBufferAllocateInfo cmd_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = cmd_pool, 
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    VkCommandBuffer cmd_buffer;
    if (VkResult result = vkAllocateCommandBuffers(device, &cmd_info, &cmd_buffer); result != VK_SUCCESS) {
        return result;
    };

    VkCommandBufferBeginInfo buffer_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    vkBeginCommandBuffer(cmd_buffer, &buffer_info);    

    VkBufferCopy copy_region{ .size = size };
    vkCmdCopyBuffer(cmd_buffer, src.buffer, dst.buffer, 1, &copy_region);

    VkCommandBufferSubmitInfo cmd_submit_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBuffer = cmd_buffer,
    };

    VkSubmitInfo submit_info{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd_buffer,
    };
    vkEndCommandBuffer(cmd_buffer);

    const VkQueue& queue = vk_device::GetQueue();
    vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);

    vkFreeCommandBuffers(device, cmd_pool, 1, &cmd_buffer);
    vkDestroyCommandPool(device, cmd_pool, nullptr);

    return VK_SUCCESS;
}