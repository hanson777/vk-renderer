#include "vk_device.h"
#include "Render/vk_common.h"
#include "vk_instance.h"
#include <cstdint>
#include <algorithm>
#include <limits>
#include <vector>
#include <iostream>

namespace vk_device {

	VkPhysicalDevice g_physicalDevice = VK_NULL_HANDLE;
	uint32_t g_graphicsQueueIndex = std::numeric_limits<uint32_t>::max();
	VkQueue g_graphicsQueue = VK_NULL_HANDLE;
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

	static bool findPhysicalDevice() {
        VkInstance instance = vk_instance::GetInstance();

		uint32_t physicalDeviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
		std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
		vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());

		if (physicalDeviceCount != 0) {
			g_physicalDevice = physicalDevices[0];
		}
		else {
			std::cerr << "[ERROR::DEVICE_MANAGER] could not find any physical devices\n";
			return false;
		}

		for (const auto& physicalDevice : physicalDevices) {
			VkPhysicalDeviceProperties properties{};
			vkGetPhysicalDeviceProperties(physicalDevice, &properties);
			if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
				g_physicalDevice = physicalDevice;
				break;
			}
		}
		return true;
	}

	static bool findQueueFamilyIndex() {
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties2(g_physicalDevice, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties2> queueFamilyProperties(queueFamilyCount, { .sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2 });
		vkGetPhysicalDeviceQueueFamilyProperties2(g_physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

		for (int i = 0; i < queueFamilyProperties.size(); i++) {
			VkBool32 hasPresentSupport = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(g_physicalDevice, i, vk_instance::GetSurface(), &hasPresentSupport);

			const auto& properties = queueFamilyProperties[i];
			if (properties.queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT && hasPresentSupport) {
				g_graphicsQueueIndex = i;
				return true;
			}
		}
		std::cerr << "[ERROR::DEVICE_MANAGER] could not find graphics-capable queue family\n";
		return false;
	}

	static bool createDevice() {
		// query supported features
		VkPhysicalDeviceVulkan14Features supportedFeatures14{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES,
			.pNext = nullptr,
		};

		VkPhysicalDeviceVulkan13Features supportedFeatures13{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
			.pNext = &supportedFeatures14,
		};

		VkPhysicalDeviceVulkan12Features supportedFeatures12{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
			.pNext = &supportedFeatures13
		};

		VkPhysicalDeviceFeatures2 supportedFeatures{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
			.pNext = &supportedFeatures12,
		};

		vkGetPhysicalDeviceFeatures2(g_physicalDevice, &supportedFeatures);
		if (!supportedFeatures13.dynamicRendering || !supportedFeatures13.synchronization2 || !supportedFeatures12.timelineSemaphore || !supportedFeatures14.maintenance5) {
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
			.timelineSemaphore = VK_TRUE,
		};

		VkPhysicalDeviceFeatures2 features{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
			.pNext = &features12,
		};

		std::vector<float> queuePriorities{ 1.0f };
		VkDeviceQueueCreateInfo graphicsQueueCreateInfo{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = g_graphicsQueueIndex,
			.queueCount = 1,
			.pQueuePriorities = queuePriorities.data(),
		};

        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(g_physicalDevice, nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(g_physicalDevice, nullptr, &extensionCount, availableExtensions.data());

		std::vector<const char*> deviceExtensions;
        bool supportsSwapchain = std::find_if(availableExtensions.begin(), availableExtensions.end(), [](const auto& prop) { return strcmp(prop.extensionName, "VK_KHR_swapchain") == 0; }) != availableExtensions.end();
        if (supportsSwapchain) { 
            deviceExtensions.push_back("VK_KHR_swapchain"); 
        } else { 
            std::cerr << "[ERROR::VK_DEVICE] device does not support swapchain extension\n"; 
			return false;
        }

		bool supportsShaderDrawParams = std::find_if(availableExtensions.begin(), availableExtensions.end(), [](const auto& prop) { return strcmp(prop.extensionName, "VK_KHR_shader_draw_parameters") == 0; }) != availableExtensions.end();
		if (supportsShaderDrawParams) {
			deviceExtensions.push_back("VK_KHR_shader_draw_parameters");
		}
		else {
			std::cerr << "[ERROR::VK_DEVICE] device does not support shader draw parameters extension\n";
			return false;
		}

        #ifdef __APPLE__
            bool supportsPortability = std::find_if(availableExtensions.begin(), availableExtensions.end(), [](const auto& prop) { return strcmp(prop.extensionName, "VK_KHR_portability_subset") == 0; }) != availableExtensions.end();
            if (supportsPortability) { deviceExtensions.push_back("VK_KHR_portability_subset"); }
        #endif

		VkDeviceCreateInfo deviceCreateInfo{
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext = &features,
			.queueCreateInfoCount = 1,
			.pQueueCreateInfos = &graphicsQueueCreateInfo,
			.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
			.ppEnabledExtensionNames = deviceExtensions.data(),
			.pEnabledFeatures = nullptr,
		};

		VkResult result = vkCreateDevice(g_physicalDevice, &deviceCreateInfo, nullptr, &g_device);
		if (result != VK_SUCCESS) {
			std::cerr << "[ERROR::DEVICE_MANAGER] failed to create logical device: " << result << '\n';
			return false;
		}
        volkLoadDevice(g_device);

		vkGetDeviceQueue(g_device, g_graphicsQueueIndex, 0, &g_graphicsQueue);
		if (g_graphicsQueue == VK_NULL_HANDLE) {
			std::cerr << "[ERROR::DEVICE_MANAGER] failed to get graphics queue device\n";
			return false;
		}

		return true;
	}

	VkPhysicalDevice GetPhysicalDevice() {
		return g_physicalDevice;
	}

	const VkDevice& GetDevice() {
		return g_device;
	}

	VkQueue GetQueue() {
		return g_graphicsQueue;
	}

	uint32_t GetQueueIndex() {
		return g_graphicsQueueIndex;
	}
}
