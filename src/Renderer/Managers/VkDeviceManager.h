#include <volk.h>

namespace VkDeviceManager {
	bool Init();
	VkPhysicalDevice GetPhysicalDevice();
	VkDevice GetLogicalDevice();
	VkQueue GetQueue();
}