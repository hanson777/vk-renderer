#define VMA_DEBUG_LOG(format, ...) do { \
        printf(format, ##__VA_ARGS__); \
        printf("\n"); \
    } while(false)

#define VMA_IMPLEMENTATION
#include "volk.h"
#include "vk_mem_alloc.h"
