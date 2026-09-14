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
    }
}

Buffer createBuffer(VkBufferUsageFlags usage, VkDeviceSize size, bool mappable, VmaMemoryUsage memory_usage) {
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

    Buffer buffer;
    VK_CHECK(vmaCreateBuffer(vk_memory::GetAllocator(), &buffer_create_info, &alloc_create_info, &buffer.buffer, &buffer.allocation, nullptr));

    if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        VkBufferDeviceAddressInfo address_info{
            .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
            .buffer = buffer.buffer,
        };
        buffer.address = vkGetBufferDeviceAddress(vk_device::GetDevice(), &address_info);
    }
    return buffer;
}

VkCommandPool createCommandPool(uint32_t queue_family_index, VkCommandPoolCreateFlags flags) {
    VkCommandPoolCreateInfo cmd_pool_ci{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = flags,
        .queueFamilyIndex = queue_family_index,
    };
    
    VkCommandPool cmd_pool;
    VK_CHECK(vkCreateCommandPool(vk_device::GetDevice(), &cmd_pool_ci, nullptr, &cmd_pool));
    return cmd_pool;
}

VkCommandBuffer createCommandBuffer(VkCommandPool cmd_pool, VkCommandBufferLevel level, bool begin) {
    VkCommandBufferAllocateInfo cmd_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = cmd_pool, 
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    VkCommandBuffer cmd_buffer;
    VK_CHECK(vkAllocateCommandBuffers(vk_device::GetDevice(), &cmd_info, &cmd_buffer));

    if (begin) {
        VkCommandBufferBeginInfo begin_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };
        VK_CHECK(vkBeginCommandBuffer(cmd_buffer, &begin_info));
    }

    return cmd_buffer;
}

void flushCommandBuffer(VkCommandBuffer cmd_buffer, VkQueue queue, VkCommandPool cmd_pool, bool free) {
    VK_CHECK(vkEndCommandBuffer(cmd_buffer));
    VkSubmitInfo submit_info{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd_buffer,
    };

    VkFenceCreateInfo fence_ci{ .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    VkFence fence;
    const VkDevice& device = vk_device::GetDevice();
    VK_CHECK(vkCreateFence(device, &fence_ci, nullptr, &fence));

    VK_CHECK(vkQueueSubmit(queue, 1, &submit_info, fence));
    vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
    vkDestroyFence(device, fence, nullptr);

    if (free) {
        vkFreeCommandBuffers(device, cmd_pool, 1, &cmd_buffer);
    }
}

void copyBuffer(Buffer& src, Buffer& dst, VkDeviceSize size) {
    VkCommandPool cmd_pool = createCommandPool(vk_device::GetQueueIndex(), VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
    VkCommandBuffer cmd_buffer = createCommandBuffer(cmd_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    VkBufferCopy copy_region{ .size = size };
    vkCmdCopyBuffer(cmd_buffer, src.buffer, dst.buffer, 1, &copy_region);

    flushCommandBuffer(cmd_buffer, vk_device::GetQueue(), cmd_pool, true);

    vkDestroyCommandPool(vk_device::GetDevice(), cmd_pool, nullptr);
}
