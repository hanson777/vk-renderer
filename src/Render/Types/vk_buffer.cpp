#include "Render/Types/vk_buffer.h"
#include "Render/vk_common.h"
#include "Render/Managers/vk_memory.h"
#include "Render/Managers/vk_device.h"

Buffer::Buffer(Buffer&& other) noexcept : buffer(other.buffer), allocation(other.allocation), address(other.address) {
    other.buffer = VK_NULL_HANDLE;
    other.allocation = VK_NULL_HANDLE;
    other.address = 0;
}

Buffer& Buffer::operator=(Buffer&& other) noexcept {
    if (this != &other) {
        Buffer::destroy();
        buffer = other.buffer;
        allocation = other.allocation;
        address = other.address;
        other.buffer = VK_NULL_HANDLE;
        other.allocation = VK_NULL_HANDLE;
        other.address = 0;
    }
    return *this;
}

Buffer::~Buffer() { destroy(); }

void Buffer::map() {
    VK_CHECK(vmaMapMemory(vk_memory::GetAllocator(), allocation, &mapped));
}

void Buffer::unmap() {
    if (mapped != nullptr) {
        vmaUnmapMemory(vk_memory::GetAllocator(), allocation);
        mapped = nullptr;
    }
}

void Buffer::destroy() {
    if (buffer != VK_NULL_HANDLE) {
        vmaDestroyBuffer(vk_memory::GetAllocator(), buffer, allocation);
        buffer = VK_NULL_HANDLE;
        allocation = VK_NULL_HANDLE;
    }
}

void createBuffer(VkBufferUsageFlags usage, VkDeviceSize size, bool mappable, VmaMemoryUsage memory_usage, Buffer* pBuffer) {
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

    VK_CHECK(vmaCreateBuffer(vk_memory::GetAllocator(), &buffer_create_info, &alloc_create_info, &pBuffer->buffer, &pBuffer->allocation, nullptr));

    if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        VkBufferDeviceAddressInfo address_info{
            .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
            .buffer = pBuffer->buffer,
        };
        pBuffer->address = vkGetBufferDeviceAddress(vk_device::GetDevice(), &address_info);
    }
}

void copyBuffer(Buffer& src, Buffer& dst, VkDeviceSize size) {
    VkCommandPoolCreateInfo cmd_pool_ci{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
        .queueFamilyIndex = vk_device::GetQueueIndex(),
    };
    
    const VkDevice& device = vk_device::GetDevice();
    VkCommandPool cmd_pool;
    VK_CHECK(vkCreateCommandPool(device, &cmd_pool_ci, nullptr, &cmd_pool));

    VkCommandBufferAllocateInfo cmd_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = cmd_pool, 
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    VkCommandBuffer cmd_buffer;
    VK_CHECK(vkAllocateCommandBuffers(device, &cmd_info, &cmd_buffer));

    VkCommandBufferBeginInfo buffer_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    VK_CHECK(vkBeginCommandBuffer(cmd_buffer, &buffer_info));

    VkBufferCopy copy_region{ .size = size };
    vkCmdCopyBuffer(cmd_buffer, src.buffer, dst.buffer, 1, &copy_region);

    VK_CHECK(vkEndCommandBuffer(cmd_buffer));

    VkSubmitInfo submit_info{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd_buffer,
    };

    VkFenceCreateInfo fence_ci{ .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    VkFence fence;
    VK_CHECK(vkCreateFence(device, &fence_ci, nullptr, &fence));

    VK_CHECK(vkQueueSubmit(vk_device::GetQueue(), 1, &submit_info, fence));
    vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
    vkDestroyFence(device, fence, nullptr);

    vkFreeCommandBuffers(device, cmd_pool, 1, &cmd_buffer);
    vkDestroyCommandPool(device, cmd_pool, nullptr);
}
