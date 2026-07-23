#include "VkDeviceManager.h"
#include "VkInstanceManager.h"
#include "volk.h"
#include <cstdint>
#include <vector>
#include <iostream>

namespace VkDeviceManager {

	VkPhysicalDevice g_physicalDevice = VK_NULL_HANDLE;
	VkDevice g_device = VK_NULL_HANDLE;
	uint32_t g_queueFamilyIndex = UINT32_MAX;

	bool Init() {
		
	}

	static bool findPhysicalDevice() {
		uint32_t physicalDeviceCount = 0;
		vkEnumeratePhysicalDevices(VkInstanceManager::GetInstance(), &physicalDeviceCount, nullptr);
		std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
		vkEnumeratePhysicalDevices(VkInstanceManager::GetInstance(), &physicalDeviceCount, physicalDevices.data());

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

		for (int currentFamilyIndex = 0; currentFamilyIndex < queueFamilyProperties.size(); currentFamilyIndex++) {
			VkBool32 hasPresentSupport = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(g_physicalDevice, currentFamilyIndex, VkInstanceManager::GetSurface(), &hasPresentSupport);

			const auto& properties = queueFamilyProperties[currentFamilyIndex];
			if (properties.queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT && hasPresentSupport) {
				g_queueFamilyIndex = currentFamilyIndex;
				return true;
			}
		}
		std::cerr << "[ERROR::DEVICE_MANAGER] could not find graphics-capable queue family\n";
		return false;
	}


}