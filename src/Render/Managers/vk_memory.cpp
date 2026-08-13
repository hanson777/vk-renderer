#include "vk_memory.h"
#include "Render/vk_common.h"
#include "vk_device.h"
#include "vk_instance.h"
#include <cstdint>
#include <iostream>

namespace vk_memory {

    constexpr uint32_t VK_VERSION = VK_API_VERSION_1_4;

    VmaAllocator g_allocator = VK_NULL_HANDLE;

    bool Init() {

        VmaVulkanFunctions vulkan_functions{};

        VmaAllocatorCreateInfo allocator_create_info{
            .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
            .physicalDevice = vk_device::GetPhysicalDevice(),
            .device = vk_device::GetDevice(),
            .pVulkanFunctions = &vulkan_functions,
            .instance = vk_instance::GetInstance(),
            .vulkanApiVersion = VK_VERSION,
        };

        vmaImportVulkanFunctionsFromVolk(&allocator_create_info, &vulkan_functions);

        VkResult result = vmaCreateAllocator(&allocator_create_info, &g_allocator);
        if (result != VK_SUCCESS) {
            std::cerr << "[ERROR::MEMORY_MANAGER] failed to create VMA allocator: " << result << '\n';
            return false;
        }
        return true;
    }

    void Shutdown() {
        if (g_allocator != VK_NULL_HANDLE) {
            vmaDestroyAllocator(g_allocator);
        }
    }

    const VmaAllocator& GetAllocator() { return g_allocator; }
}
