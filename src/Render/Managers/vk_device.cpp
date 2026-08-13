#include "vk_device.h"
#include "Render/vk_common.h"
#include "vk_instance.h"
#include <cstdint>
#include <algorithm>
#include <limits>
#include <vector>
#include <iostream>

namespace vk_device {

	VkPhysicalDevice g_physical_device = VK_NULL_HANDLE;
	uint32_t g_graphics_queue_index = std::numeric_limits<uint32_t>::max();
	VkQueue g_graphics_queue = VK_NULL_HANDLE;
	VkDevice g_device = VK_NULL_HANDLE;

	static bool findPhysicalDevice();
	static bool findQueueFamilyIndex();
	static bool createDevice();

	bool Init() {
		if (!findPhysicalDevice())   return false;
		if (!findQueueFamilyIndex()) return false;
		if (!createDevice())  return false;
		return true;
	}

	void Shutdown() {
		if (g_device != VK_NULL_HANDLE) {
			vkDestroyDevice(g_device, nullptr);
		}
	}

	static bool findPhysicalDevice() {
        VkInstance instance = vk_instance::GetInstance();

		uint32_t physical_device_count = 0;
		vkEnumeratePhysicalDevices(instance, &physical_device_count, nullptr);
		std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
		vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices.data());

		if (physical_device_count != 0) {
			g_physical_device = physical_devices[0];
		}
		else {
			std::cerr << "[ERROR::DEVICE_MANAGER] could not find any physical devices\n";
			return false;
		}

		for (const auto& physical_device : physical_devices) {
			VkPhysicalDeviceProperties properties{};
			vkGetPhysicalDeviceProperties(physical_device, &properties);
			if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
				g_physical_device = physical_device;
				break;
			}
		}
		return true;
	}

	static bool findQueueFamilyIndex() {
		uint32_t queue_family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties2(g_physical_device, &queue_family_count, nullptr);
		std::vector<VkQueueFamilyProperties2> queue_family_properties(queue_family_count, { .sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2 });
		vkGetPhysicalDeviceQueueFamilyProperties2(g_physical_device, &queue_family_count, queue_family_properties.data());

		for (int i = 0; i < queue_family_properties.size(); i++) {
			VkBool32 has_present_support = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(g_physical_device, i, vk_instance::GetSurface(), &has_present_support);

			const auto& properties = queue_family_properties[i];
			if (properties.queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT && has_present_support) {
				g_graphics_queue_index = i;
				return true;
			}
		}
		std::cerr << "[ERROR::DEVICE_MANAGER] could not find graphics-capable queue family\n";
		return false;
	}

	static bool createDevice() {
		// query supported features
		VkPhysicalDeviceVulkan14Features supported_features_14{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES,
			.pNext = nullptr,
		};

		VkPhysicalDeviceVulkan13Features supported_features_13{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
			.pNext = &supported_features_14,
		};

		VkPhysicalDeviceVulkan12Features supported_features_12{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
			.pNext = &supported_features_13,
		};

		VkPhysicalDeviceVulkan11Features supported_features_11{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
			.pNext = &supported_features_12,
		};

		VkPhysicalDeviceFeatures2 supported_features{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
			.pNext = &supported_features_11,
		};

		vkGetPhysicalDeviceFeatures2(g_physical_device, &supported_features);
		if (!supported_features_13.dynamicRendering  || !supported_features_13.synchronization2 || 
			!supported_features_12.timelineSemaphore || !supported_features_12.separateDepthStencilLayouts ||
			!supported_features_14.maintenance5      || !supported_features_11.shaderDrawParameters) {
			std::cerr << "[ERROR::DEVICE_MANAGER] physical device doesn't meet feature requirements\n";
			return false;
		}
		
		// enable the features
		VkPhysicalDeviceVulkan14Features features14{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES,
			.pNext = nullptr,
			.maintenance5 = VK_TRUE,
		};

		VkPhysicalDeviceVulkan13Features features13{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
			.pNext = &features14,
			.synchronization2 = VK_TRUE,
			.dynamicRendering = VK_TRUE,
		};

		VkPhysicalDeviceVulkan12Features features12{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
			.pNext = &features13,
			.separateDepthStencilLayouts = VK_TRUE,
			.timelineSemaphore = VK_TRUE,
		};

		VkPhysicalDeviceVulkan11Features features11{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
			.pNext = &features12,
			.shaderDrawParameters = VK_TRUE,
		};

		VkPhysicalDeviceFeatures2 features{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
			.pNext = &features11,
		};

		std::vector<float> queue_priorities{ 1.0f };
		VkDeviceQueueCreateInfo graphics_queue_create_info{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = g_graphics_queue_index,
			.queueCount = 1,
			.pQueuePriorities = queue_priorities.data(),
		};

        uint32_t extension_count;
        vkEnumerateDeviceExtensionProperties(g_physical_device, nullptr, &extension_count, nullptr);
        std::vector<VkExtensionProperties> available_extensions(extension_count);
        vkEnumerateDeviceExtensionProperties(g_physical_device, nullptr, &extension_count, available_extensions.data());

		std::vector<const char*> device_extensions;
        bool supports_swapchain = std::find_if(available_extensions.begin(), available_extensions.end(), [](const auto& prop) { return strcmp(prop.extensionName, "VK_KHR_swapchain") == 0; }) != available_extensions.end();
        if (supports_swapchain) { 
            device_extensions.push_back("VK_KHR_swapchain"); 
        } else { 
            std::cerr << "[ERROR::VK_DEVICE] device does not support swapchain extension\n"; 
			return false;
        }

		bool supports_shader_draw_params = std::find_if(available_extensions.begin(), available_extensions.end(), [](const auto& prop) { return strcmp(prop.extensionName, "VK_KHR_shader_draw_parameters") == 0; }) != available_extensions.end();
		if (supports_shader_draw_params) {
			device_extensions.push_back("VK_KHR_shader_draw_parameters");
		}
		else {
			std::cerr << "[ERROR::VK_DEVICE] device does not support shader draw parameters extension\n";
			return false;
		}

		bool supports_pageable_device_local_memory = std::find_if(available_extensions.begin(), available_extensions.end(), [](const auto& prop) { return strcmp(prop.extensionName, "VK_EXT_pageable_device_local_memory") == 0; }) != available_extensions.end();
		if (supports_pageable_device_local_memory) {
			device_extensions.push_back("VK_EXT_pageable_device_local_memory");
			device_extensions.push_back("VK_EXT_memory_priority");
		}

        #ifdef __APPLE__
            bool supports_portability = std::find_if(available_extensions.begin(), available_extensions.end(), [](const auto& prop) { return strcmp(prop.extensionName, "VK_KHR_portability_subset") == 0; }) != available_extensions.end();
            if (supports_portability) { device_extensions.push_back("VK_KHR_portability_subset"); }
        #endif

		VkDeviceCreateInfo device_create_info{
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext = &features,
			.queueCreateInfoCount = 1,
			.pQueueCreateInfos = &graphics_queue_create_info,
			.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size()),
			.ppEnabledExtensionNames = device_extensions.data(),
			.pEnabledFeatures = nullptr,
		};

		VkResult result = vkCreateDevice(g_physical_device, &device_create_info, nullptr, &g_device);
		if (result != VK_SUCCESS) {
			std::cerr << "[ERROR::DEVICE_MANAGER] failed to create logical device: " << result << '\n';
			return false;
		}
        volkLoadDevice(g_device);

		vkGetDeviceQueue(g_device, g_graphics_queue_index, 0, &g_graphics_queue);
		if (g_graphics_queue == VK_NULL_HANDLE) {
			std::cerr << "[ERROR::DEVICE_MANAGER] failed to get graphics queue device\n";
			return false;
		}

		return true;
	}

	const VkPhysicalDevice& GetPhysicalDevice() {
		return g_physical_device;
	}

	const VkDevice& GetDevice() {
		return g_device;
	}

	VkQueue GetQueue() {
		return g_graphics_queue;
	}

	uint32_t GetQueueIndex() {
		return g_graphics_queue_index;
	}
}
