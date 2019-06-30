
#include "tr_local.h"

#define foreach(item, list) \
    for(T * item = list->head; item != NULL; item = item->next)

vkinstance_t	vkInstance;

struct QueueFamilyIndices {
	uint32_t graphicsFamily;
	uint32_t presentFamily;
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	uint32_t formatCount;
	VkSurfaceFormatKHR formats[10];
	uint32_t presentModeCount;
	VkPresentModeKHR presentModes[10];
};

void VK_CreateInstance() {
	const char* instance_extensions[] = {
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#ifndef NDEBUG
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME
#endif
	};

	// check extensions availability
	{
		uint32_t count = 0;
		VK_CHECK(vkEnumerateInstanceExtensionProperties(NULL, &count, NULL));
		VkExtensionProperties extension_properties[100];
		VK_CHECK(vkEnumerateInstanceExtensionProperties(NULL, &count, &extension_properties[0]));

		
		for(int i = 0; i < (sizeof(instance_extensions) / sizeof(instance_extensions[0])); i++) {
			qboolean supported = qfalse;
			for (int j = 0; j < count; j++) {
				if (!strcmp(extension_properties[j].extensionName, instance_extensions[i])) {
					supported = qtrue;
					break;
				}
			}
			if (!supported) ri.Error(ERR_FATAL, "Vulkan: required instance extension is not available: %s", instance_extensions[i]);
		}
	}

	// create instance
	{
		VkInstanceCreateInfo desc;
		desc.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		desc.pNext = NULL;
		desc.flags = 0;
		desc.pApplicationInfo = NULL;
		desc.enabledLayerCount = 0;
		desc.ppEnabledLayerNames = NULL;
		desc.enabledExtensionCount = sizeof(instance_extensions) / sizeof(instance_extensions[0]);
		desc.ppEnabledExtensionNames = instance_extensions;

#ifndef NDEBUG
		const char* validation_layer_name = "VK_LAYER_LUNARG_standard_validation";
		desc.enabledLayerCount = 1;
		desc.ppEnabledLayerNames = &validation_layer_name;
#endif

		VK_CHECK(vkCreateInstance(&desc, NULL, &vkInstance.instance));
	}
}

/*
** VK_CreateSurface
**
** win: (hinstance, hwnd)
*/
void VK_CreateSurface(void *p1, void *p2) {
#ifdef WIN32
	VkWin32SurfaceCreateInfoKHR desc;
	desc.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	desc.pNext = NULL;
	desc.flags = 0;
	desc.hinstance = p1;
	desc.hwnd = p2;
	VK_CHECK(vkCreateWin32SurfaceKHR(vkInstance.instance, &desc, NULL, &vkInstance.surface));
#endif
}

qboolean isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);
void VK_PickPhysicalDevice()
{
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(vkInstance.instance, &deviceCount, NULL);
		if (deviceCount == 0) {
			ri.Error(ERR_FATAL, "Vulkan: failed to find GPUs with Vulkan support!");
		}
		VkPhysicalDevice devices[10];
		vkEnumeratePhysicalDevices(vkInstance.instance, &deviceCount, &devices[0]);

		for (int i = 0; i < deviceCount; i++) {
			if (isDeviceSuitable(devices[i], vkInstance.surface)) {
				vkInstance.physical_device = devices[i];
				break;
			}
		}
	}

	if (vkInstance.physical_device == VK_NULL_HANDLE) {
		ri.Error(ERR_FATAL, "Vulkan: failed to find a suitable GPU!");
	}

	{
		uint32_t extensionCount = 0;
		vkEnumerateDeviceExtensionProperties(vkInstance.physical_device, NULL, &extensionCount, NULL);
		VkExtensionProperties extensions[50];
		vkEnumerateDeviceExtensionProperties(vkInstance.physical_device, NULL, &extensionCount, &extensions[0]);
		//std::cout << "LogicalDeviceExtensions..." << std::endl;

		//for (const auto& extension : extensions) {
		//	std::cout << "\t" << extension.extensionName << std::endl;
		//}
	}
	// device infos
	VkPhysicalDeviceProperties devProperties;
	vkGetPhysicalDeviceProperties(vkInstance.physical_device, &devProperties);

	// device limits
	VkPhysicalDeviceLimits limits = devProperties.limits;

	//std::cout << "GPU in use..." << std::endl;
	//std::cout << "\t" << devProperties.deviceName << std::endl;

}

struct QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {

	struct QueueFamilyIndices indices;
	indices.graphicsFamily = -1;
	indices.presentFamily = -1;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);
	VkQueueFamilyProperties queueFamilies[20];
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, &queueFamilies[0]);

	int i = 0;
	for (int j = 0; j < queueFamilyCount; j++) {
		VkQueueFamilyProperties queueFamily = queueFamilies[j];
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport = qfalse;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

		if (queueFamily.queueCount > 0 && presentSupport) {
			indices.presentFamily = i;
		}

		if (indices.graphicsFamily >= 0 && indices.presentFamily >= 0) {
			break;
		}

		i++;
	}

	return indices;
}

struct SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
	struct SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &details.formatCount, NULL);

	if (details.formatCount != 0) {
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &details.formatCount, &details.formats[0]);
	}

	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &details.presentModeCount, NULL);

	if (details.presentModeCount != 0) {
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &details.presentModeCount, &details.presentModes[0]);
	}

	return details;
}

qboolean checkDeviceExtensionSupport(VkPhysicalDevice device) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);
	VkExtensionProperties availableExtensions[60];
	vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, &availableExtensions[0]);

	const char* deviceExtensions[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	for (int i = 0; i < (sizeof(deviceExtensions) / sizeof(deviceExtensions[0])); i++) {
		qboolean supported = qfalse;
		for (int j = 0; j < extensionCount; j++) {
			if (!strcmp(availableExtensions[j].extensionName, deviceExtensions[i])) {
				supported = qtrue;
				break;
			}
		}
		if (!supported) ri.Error(ERR_FATAL, "Vulkan: required instance extension is not available: %s", deviceExtensions[i]);
	}
	return qtrue;
}

qboolean isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
	struct QueueFamilyIndices q = findQueueFamilies(device, surface);

	qboolean extensionsSupported = checkDeviceExtensionSupport(device);

	qboolean swapChainAdequate = qfalse;
	if (extensionsSupported) {
		struct SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
		swapChainAdequate = !swapChainSupport.formatCount && !swapChainSupport.presentModeCount;
	}

	return q.graphicsFamily >= 0 && q.presentFamily >= 0;
}