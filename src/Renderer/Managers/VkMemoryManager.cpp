#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include "VkMemoryManager.h"
#include "VkDeviceManager.h"
#include "VkInstanceManager.h"
#include <vma/vk_mem_alloc.h>
#include <volk.h>
#include <cstdint>
#include <iostream>

namespace VkMemoryManager {

    constexpr uint32_t VK_VERSION = VK_API_VERSION_1_4;

    VmaAllocator g_allocator = VK_NULL_HANDLE;

    bool Init() {

        VmaVulkanFunctions vulkanFunctions{};

        VmaAllocatorCreateInfo allocatorCreateInfo{
            .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
            .physicalDevice = VkDeviceManager::GetPhysicalDevice(),
            .device = VkDeviceManager::GetLogicalDevice(),
            .pVulkanFunctions = &vulkanFunctions,
            .instance = VkInstanceManager::GetInstance(),
            .vulkanApiVersion = VK_VERSION,
        };

        vmaImportVulkanFunctionsFromVolk(&allocatorCreateInfo, &vulkanFunctions);

        VkResult result = vmaCreateAllocator(&allocatorCreateInfo, &g_allocator);
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
}
