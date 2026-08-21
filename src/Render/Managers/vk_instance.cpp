#include "vk_instance.h"
#include "Render/vk_common.h"
#include "Core/Window.h"
#include <GLFW/glfw3.h>
#include <cstdint>
#include <iostream>
#include <vector>

namespace vk_instance {

	constexpr uint32_t VK_VERSION = VK_API_VERSION_1_4;

    VkInstance g_instance = VK_NULL_HANDLE; 
    VkDebugUtilsMessengerEXT g_debug_messenger = VK_NULL_HANDLE;
    VkSurfaceKHR g_surface = VK_NULL_HANDLE;
    
    bool g_validation_enabled = true;

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_types, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data) {
        std::cerr << "[VALIDATION LAYER] " << callback_data->pMessage << '\n';
        return VK_FALSE;
    }

    bool Init() {
        uint32_t glfw_extension_count = 0;
		const char** glfw_extensions;
		glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

        std::vector<const char*> extensions(
            glfw_extensions,
            glfw_extensions + glfw_extension_count
        );
        
        if (g_validation_enabled) { 
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        #ifdef __APPLE__
            extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        #endif

        std::vector<const char*> layers;
        if (g_validation_enabled) {
            uint32_t layer_count;
            vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
            std::vector<VkLayerProperties> available_layers(layer_count);
            vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data()); 
            
            bool found = false;
            for (const auto& layer : available_layers) {
                if (strcmp(layer.layerName, "VK_LAYER_KHRONOS_validation") == 0) {
                    layers.push_back("VK_LAYER_KHRONOS_validation");
                    found = true;
                    break;
                }
            }
            
            if (!found) {
                std::cerr << "[ERROR::INSTANCE_MANAGER] validation layers requested but none found\n";
                g_validation_enabled = false;
            }
        }

        VkDebugUtilsMessengerCreateInfoEXT debug_create_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT    | 
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT   |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = debugCallback,
        };

		VkApplicationInfo app_info{
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pApplicationName = "hi",
			.apiVersion = VK_VERSION,
		};

		VkInstanceCreateInfo instance_create_info{
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = g_validation_enabled ? &debug_create_info : nullptr,
			.pApplicationInfo = &app_info,
            .enabledLayerCount = static_cast<uint32_t>(layers.size()),
            .ppEnabledLayerNames = layers.data(),
			.enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
			.ppEnabledExtensionNames = extensions.data(),
		};
        #ifdef __APPLE__
            instance_create_info.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        #endif

        VkResult result = vkCreateInstance(&instance_create_info, nullptr, &g_instance);
        if (result != VK_SUCCESS) {
            std::cerr << "[ERROR::INSTANCE_MANAGER] failed to create instance: " << result << '\n';
            return false;
        }

		volkLoadInstance(g_instance);

        if (vkCreateDebugUtilsMessengerEXT(g_instance, &debug_create_info, nullptr, &g_debug_messenger) != VK_SUCCESS) {
            std::cerr << "[ERROR::INSTANCE_MANAGER] failed to set up debug messenger\n";
            return false;
        }
        
        if (glfwCreateWindowSurface(g_instance, Window::GetHandle(), nullptr, &g_surface) != VK_SUCCESS) {
            std::cerr << "[ERROR::INSTANCE_MANAGER] failed to create surface\n";
            return false;
        }

        return true;
    }
    
    void Shutdown() {
        if (g_validation_enabled && g_debug_messenger != VK_NULL_HANDLE) {
            vkDestroyDebugUtilsMessengerEXT(g_instance, g_debug_messenger, nullptr);
        }
        if (g_surface != VK_NULL_HANDLE) vkDestroySurfaceKHR(g_instance, g_surface, nullptr);
        if (g_instance != VK_NULL_HANDLE) vkDestroyInstance(g_instance, nullptr);
        volkFinalize();
    }

    VkInstance GetInstance() {
        return g_instance;
    }

    VkSurfaceKHR GetSurface() {
        return g_surface;
    }
}
