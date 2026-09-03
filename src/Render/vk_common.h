#pragma once
#include <cstdint>

#define VK_NO_PROTOTYPES
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0

constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

#include <volk.h>
#include <vk_mem_alloc.h>
