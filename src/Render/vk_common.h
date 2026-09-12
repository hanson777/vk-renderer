#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#define VK_NO_PROTOTYPES
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0

#include <volk.h>
#include <vk_mem_alloc.h>

constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

inline const char* VkResultToString(VkResult result)
{
    switch (result)
    {
#define STR(r) case VK_##r: return #r
        STR(NOT_READY);
        STR(TIMEOUT);
        STR(EVENT_SET);
        STR(EVENT_RESET);
        STR(INCOMPLETE);
        STR(ERROR_OUT_OF_HOST_MEMORY);
        STR(ERROR_OUT_OF_DEVICE_MEMORY);
        STR(ERROR_INITIALIZATION_FAILED);
        STR(ERROR_DEVICE_LOST);
        STR(ERROR_MEMORY_MAP_FAILED);
        STR(ERROR_LAYER_NOT_PRESENT);
        STR(ERROR_EXTENSION_NOT_PRESENT);
        STR(ERROR_FEATURE_NOT_PRESENT);
        STR(ERROR_INCOMPATIBLE_DRIVER);
        STR(ERROR_TOO_MANY_OBJECTS);
        STR(ERROR_FORMAT_NOT_SUPPORTED);
        STR(ERROR_SURFACE_LOST_KHR);
        STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
        STR(SUBOPTIMAL_KHR);
        STR(ERROR_OUT_OF_DATE_KHR);
        STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
        STR(ERROR_VALIDATION_FAILED_EXT);
#undef STR
        default: return "UNKNOWN_ERROR";
    }
}

#define VK_CHECK(f)                                                          \
    do {                                                                     \
        VkResult vkCheckResult_ = (f);                                       \
        if (vkCheckResult_ != VK_SUCCESS) {                                  \
            fprintf(stderr, "Fatal: VkResult is %s in %s at line %d\n",      \
                    VkResultToString(vkCheckResult_), __FILE__, __LINE__);   \
            abort();                                                        \
        }                                                                    \
    } while (0)
