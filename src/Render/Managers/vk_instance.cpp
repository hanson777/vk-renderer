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
    
    bool g_validationEnabled = true;

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* callbackData, void* userData) {
        std::cerr << "[VALIDATION LAYER] " << callbackData->pMessage << '\n';
        return VK_FALSE;
    }

    bool Init() {
        uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(
            glfwExtensions,
            glfwExtensions + glfwExtensionCount
        );
        
        if (g_validationEnabled) { 
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        #ifdef __APPLE__
            extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        #endif

        std::vector<const char*> layers;
        if (g_validationEnabled) {
            uint32_t layerCount;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
            std::vector<VkLayerProperties> availableLayers(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()); 
            
            bool found = false;
            for (const auto& layer : availableLayers) {
                if (strcmp(layer.layerName, "VK_LAYER_KHRONOS_validation") == 0) {
                    layers.push_back("VK_LAYER_KHRONOS_validation");
                    found = true;
                    break;
                }
            }
            
            if (!found) {
                std::cerr << "[ERROR::INSTANCE_MANAGER] validation layers requested but none found\n";
                g_validationEnabled = false;
            }
        }

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT    | 
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT   |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = debugCallback,
        };

		VkApplicationInfo appInfo{
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pApplicationName = "hi",
			.apiVersion = VK_VERSION,
		};

		VkInstanceCreateInfo instanceCreateInfo{
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = g_validationEnabled ? &debugCreateInfo : nullptr,
			.pApplicationInfo = &appInfo,
            .enabledLayerCount = static_cast<uint32_t>(layers.size()),
            .ppEnabledLayerNames = layers.data(),
			.enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
			.ppEnabledExtensionNames = extensions.data(),
		};
        #ifdef __APPLE__
            instanceCreateInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        #endif

        VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &g_instance);
        if (result != VK_SUCCESS) {
            std::cerr << "[ERROR::INSTANCE_MANAGER] failed to create instance: " << result << '\n';
            return false;
        }

		volkLoadInstance(g_instance);

        if (vkCreateDebugUtilsMessengerEXT(g_instance, &debugCreateInfo, nullptr, &g_debug_messenger) != VK_SUCCESS) {
            std::cerr << "[ERROR::INSTANCE_MANAGER] failed to set up debug messenger\n";
            return false;
        }
        
        if (glfwCreateWindowSurface(g_instance, Window::GetWindowPointer(), nullptr, &g_surface) != VK_SUCCESS) {
            std::cerr << "[ERROR::INSTANCE_MANAGER] failed to create surface\n";
            return false;
        }

        return true;
    }
    
    void Shutdown() {
        if (g_validationEnabled && g_debug_messenger != VK_NULL_HANDLE) {
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
