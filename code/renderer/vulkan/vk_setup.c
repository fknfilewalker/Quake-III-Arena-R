
#include "../tr_local.h"

vkinstance_t vk;
vkdata_t     vk_d;

static const char* deviceExtensions[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static const char* validationLayers[] = {
		"VK_LAYER_LUNARG_standard_validation"
};

static const char* instanceExtensions[] = {
#ifndef NDEBUG
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
		VK_KHR_SURFACE_EXTENSION_NAME,
#if defined( _WIN32 )
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#elif defined(__APPLE__)
		VK_MVK_MACOS_SURFACE_EXTENSION_NAME
#elif defined( __linux__ )

#endif
};

/*
 ==============================================================================
 
 Vulkan Setup
 
 ==============================================================================
 */

// Function Declaration
static void VK_CreateInstance();
static void VK_CreateSurface(void* p1, void* p2);
static void VK_PickPhysicalDevice();
static void VK_CreateLogicalDevice();
static void VK_CreateCommandPool();
static void VK_SetupDebugCallback();

// Helper
static qboolean VK_IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);
static qboolean VK_CheckValidationLayerSupport();

void VK_Setup(void* p1, void* p2) {
    if (!VK_LoadGlobalFunctions()) return qfalse;
    VK_CreateInstance();
    if (!VK_LoadInstanceFunctions()) return qfalse;
    VK_CreateSurface(p1, p2);
    VK_PickPhysicalDevice();
    VK_CreateLogicalDevice();
    if (!VK_LoadDeviceFunctions()) return qfalse;
    VK_CreateCommandPool();
#ifndef NDEBUG
    VK_SetupDebugCallback();
#endif
    
    // Create Swapchain
	VK_SetupSwapchain();
}

void VK_Destroy() {
	vkDeviceWaitIdle(vk.device);

	VK_DestroySwapchain();

	vkDestroyCommandPool(vk.device, vk.commandPool, NULL);
	vkDestroyDevice(vk.device, NULL);
	vkDestroySurfaceKHR(vk.instance, vk.surface, NULL);

#ifndef NDEBUG
	vkDestroyDebugUtilsMessengerEXT(vk.instance, vk.callback, NULL);
#endif

	vkDestroyInstance(vk.instance, NULL);

	Com_Memset(&vk, 0, sizeof(vk));
}

static void VK_CreateInstance() {
	// check extensions availability
	{
		uint32_t count = 0;
		vkEnumerateInstanceExtensionProperties(NULL, &count, NULL);
		VkExtensionProperties *extension_properties = malloc(count * sizeof(VkExtensionProperties));
		vkEnumerateInstanceExtensionProperties(NULL, &count, &extension_properties[0]);

		for (int i = 0; i < (sizeof(instanceExtensions) / sizeof(instanceExtensions[0])); i++) {
			qboolean supported = qfalse;
			for (int j = 0; j < count; j++) {
				if (!strcmp(extension_properties[j].extensionName, instanceExtensions[i])) {
					supported = qtrue;
					break;
				}
			}
			if (!supported) ri.Error(ERR_FATAL, "Vulkan: required instance extension is not available: %s", instanceExtensions[i]);
		}
		free(extension_properties);
	}

#ifndef NDEBUG
	if (!VK_CheckValidationLayerSupport()) {
		ri.Error(ERR_FATAL, "Vulkan: validation layers requested, but not available!");
	}
#endif

	// create instance
	{
		VkInstanceCreateInfo desc = { 0 };
		desc.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		desc.pNext = NULL;
		desc.flags = 0;
		desc.pApplicationInfo = NULL;
		desc.enabledLayerCount = 0;
		desc.ppEnabledLayerNames = NULL;
		desc.enabledExtensionCount = sizeof(instanceExtensions) / sizeof(instanceExtensions[0]);
		desc.ppEnabledExtensionNames = instanceExtensions;
#ifndef NDEBUG
		desc.enabledLayerCount = (uint32_t)(sizeof(validationLayers) / sizeof(validationLayers[0]));
		desc.ppEnabledLayerNames = &validationLayers[0];
#endif
		VK_CHECK(vkCreateInstance(&desc, NULL, &vk.instance), "failed to create Instance!");
	}
}

/*
** VK_CreateSurface
**
** win:		(HINSTANCE, HWND)
** macOS:	(NSView, NULL)
*/
static void VK_CreateSurface(void* p1, void* p2) {
#ifdef WIN32
	VkWin32SurfaceCreateInfoKHR desc = {0};
	desc.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	desc.pNext = NULL;
	desc.flags = 0;
	desc.hinstance = p1;
	desc.hwnd = p2;
	VK_CHECK(vkCreateWin32SurfaceKHR(vk.instance, &desc, NULL, &vk.surface), "failed to create Win32 Surface!");
#elif defined(__APPLE__)
    VkMacOSSurfaceCreateInfoMVK desc = {0};
    desc.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
    desc.pNext = NULL;
    desc.flags = 0;
    desc.pView = p1;
    VK_CHECK(vkCreateMacOSSurfaceMVK(vk.instance, &desc, NULL, &vk.surface), "failed to create MacOS Surface!");
#elif defined( __linux__ )
        
#endif
}

static void VK_PickPhysicalDevice()
{
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(vk.instance, &deviceCount, NULL);
		if (deviceCount == 0) {
			ri.Error(ERR_FATAL, "Vulkan: failed to find GPUs with Vulkan support!");
		}
		VkPhysicalDevice devices[10];
		vkEnumeratePhysicalDevices(vk.instance, &deviceCount, &devices[0]);

		for (int i = 0; i < deviceCount; i++) {
			if (VK_IsDeviceSuitable(devices[i], vk.surface)) {
				vk.physical_device = devices[i];
				break;
			}
		}
	}

	if (vk.physical_device == VK_NULL_HANDLE) {
		ri.Error(ERR_FATAL, "Vulkan: failed to find a suitable GPU!");
	}

	{
		uint32_t extensionCount = 0;
		vkEnumerateDeviceExtensionProperties(vk.physical_device, NULL, &extensionCount, NULL);
		VkExtensionProperties *extensions = malloc(extensionCount * sizeof(VkExtensionProperties));
		vkEnumerateDeviceExtensionProperties(vk.physical_device, NULL, &extensionCount, &extensions[0]);
		//std::cout << "LogicalDeviceExtensions..." << std::endl;

		//for (const auto& extension : extensions) {
		//	std::cout << "\t" << extension.extensionName << std::endl;
		//}
		free(extensions);
	}
	// device infos
	VkPhysicalDeviceProperties devProperties;
	vkGetPhysicalDeviceProperties(vk.physical_device, &devProperties);

	// device limits
	VkPhysicalDeviceLimits limits = devProperties.limits;

	//std::cout << "GPU in use..." << std::endl;
	//std::cout << "\t" << devProperties.deviceName << std::endl;

}

static void VK_CreateLogicalDevice()
{
	VkDeviceQueueCreateInfo queueCreateInfos[2] = {0};

	// if graphic and present indices are the same, only one queue is needed
	uint32_t queueCreateInfosCount = vk.queryFamilyIndices.graphicsFamily == vk.queryFamilyIndices.presentFamily ? 1 : 2;
	float queuePriority = 1.0f;
	queueCreateInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfos[0].queueFamilyIndex = vk.queryFamilyIndices.graphicsFamily;
	queueCreateInfos[0].queueCount = 1;
	queueCreateInfos[0].pQueuePriorities = &queuePriority;

	if (vk.queryFamilyIndices.graphicsFamily != vk.queryFamilyIndices.presentFamily) {
		queueCreateInfos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfos[1].queueFamilyIndex = vk.queryFamilyIndices.presentFamily;
		queueCreateInfos[1].queueCount = 1;
		queueCreateInfos[1].pQueuePriorities = &queuePriority;
	}

	// activate features
	VkPhysicalDeviceFeatures deviceFeatures = { 0 };
	deviceFeatures.fillModeNonSolid = qtrue;
	deviceFeatures.multiDrawIndirect = qfalse;
	deviceFeatures.drawIndirectFirstInstance = qfalse;
	deviceFeatures.shaderClipDistance = qtrue;

	VkDeviceCreateInfo desc = { 0 };
	desc.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	desc.queueCreateInfoCount = queueCreateInfosCount;
	desc.pQueueCreateInfos = &queueCreateInfos[0];

	desc.pEnabledFeatures = &deviceFeatures;

	desc.enabledExtensionCount = (uint32_t)(sizeof(deviceExtensions) / sizeof(deviceExtensions[0]));
	desc.ppEnabledExtensionNames = &deviceExtensions[0];

	VK_CHECK(vkCreateDevice(vk.physical_device, &desc, NULL, &vk.device), "failed to create logical device!")
}

static void VK_CreateCommandPool() {
	vkGetDeviceQueue(vk.device, vk.queryFamilyIndices.graphicsFamily, 0, &vk.graphicsQueue);
	vkGetDeviceQueue(vk.device, vk.queryFamilyIndices.presentFamily, 0, &vk.presentQueue);

	VkCommandPoolCreateInfo poolInfo = { 0 };
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = vk.queryFamilyIndices.graphicsFamily;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;//VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

	VK_CHECK(vkCreateCommandPool(vk.device, &poolInfo, NULL, &vk.commandPool), "failed to create command pool!");
}

/*
==============================================================================

Vulkan Debug Function

==============================================================================
*/
static qboolean VK_CheckValidationLayerSupport() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, NULL);
	VkLayerProperties* availableLayers = malloc(layerCount * sizeof(VkLayerProperties));
	vkEnumerateInstanceLayerProperties(&layerCount, &availableLayers[0]);

	for (int i = 0; i < (sizeof(validationLayers) / sizeof(validationLayers[0])); i++) {
		qboolean layerFound = qfalse;
		for (int j = 0; j < layerCount; j++) {
			if (!strcmp(availableLayers[j].layerName, validationLayers[i])) {
				layerFound = qtrue;
				break;
			}
		}
		if (!layerFound) {
			return qfalse;
		}
	}

	free(availableLayers);
	return qtrue;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL VK_DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
	ri.Printf(PRINT_WARNING, "Vulkan Validation Layer: %s\n", pCallbackData->pMessage);

	return VK_FALSE;
}

void VK_SetupDebugCallback() {
	VkDebugUtilsMessengerCreateInfoEXT createInfo = { 0 };
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = &VK_DebugCallback;

	VK_CHECK(vkCreateDebugUtilsMessengerEXT(vk.instance, &createInfo, NULL, &vk.callback), "failed to set up debug callback!");
}

/*
==============================================================================

Vulkan Helper Function (Query Functions etc)

==============================================================================
*/

// -- Device Query -- Start
static vkqueueFamilyIndices_t VK_FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {

	vkqueueFamilyIndices_t indices;
	indices.graphicsFamily = -1;
	indices.presentFamily = -1;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);
	VkQueueFamilyProperties *queueFamilies = malloc(queueFamilyCount * sizeof(VkQueueFamilyProperties));
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

		if (indices.graphicsFamily >= 0 &&
			indices.presentFamily  >= 0) {
			break;
		}

		i++;
	}

	free(queueFamilies);
	return indices;
}

static qboolean VK_CheckDeviceExtensionSupport(VkPhysicalDevice device) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);
	VkExtensionProperties* availableExtensions = malloc(extensionCount * sizeof(VkExtensionProperties));
	vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, &availableExtensions[0]);

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

	free(availableExtensions);
	return qtrue;
}

qboolean VK_IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
	vkqueueFamilyIndices_t queueFamilyIndices = VK_FindQueueFamilies(device, surface);

	qboolean extensionsSupported = VK_CheckDeviceExtensionSupport(device);

	qboolean swapChainAdequate = qfalse;
	if (extensionsSupported) {
		swapChainSupportDetails_t swapChainSupport = querySwapChainSupport(device, surface);
		swapChainAdequate = swapChainSupport.formatCount && swapChainSupport.presentModeCount;
	}

	if (queueFamilyIndices.graphicsFamily >= 0 &&
		queueFamilyIndices.presentFamily >= 0 &&
		extensionsSupported &&
		swapChainAdequate)
	{
		Com_Memcpy(&vk.queryFamilyIndices, &queueFamilyIndices, sizeof(vkqueueFamilyIndices_t));
		return qtrue;
	}
	else return qfalse;

	return	vk.queryFamilyIndices.graphicsFamily >= 0 &&
		vk.queryFamilyIndices.presentFamily >= 0 && extensionsSupported && swapChainAdequate;;
}
// -- Device Query -- End


swapChainSupportDetails_t querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
	swapChainSupportDetails_t details;

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

char* VK_ErrorString(VkResult errorCode)
{
	switch (errorCode)
	{
#define STR(r) case VK_ ##r: return #r
		STR(SUCCESS);
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
		STR(ERROR_FRAGMENTED_POOL);
		STR(ERROR_OUT_OF_POOL_MEMORY);
		STR(ERROR_INVALID_EXTERNAL_HANDLE);
		STR(ERROR_SURFACE_LOST_KHR);
		STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
		STR(SUBOPTIMAL_KHR);
		STR(ERROR_OUT_OF_DATE_KHR);
		STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
		STR(ERROR_VALIDATION_FAILED_EXT);
		STR(ERROR_INVALID_SHADER_NV);
		STR(ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT);
		STR(ERROR_FRAGMENTATION_EXT);
		STR(ERROR_NOT_PERMITTED_EXT);
		STR(ERROR_INVALID_DEVICE_ADDRESS_EXT);
		STR(ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT);
#undef STR
	default:
		return "UNKNOWN_ERROR";
	}
}



