
#include "../tr_local.h"


/*
** vkGetInstanceProcAddr needs to be set by platform specific implementation
*/ 
#define VK_GLOBAL_LEVEL_FUNCTION( a ) vkGetInstanceProcAddr( NULL, a )
#define VK_INSTANCE_LEVEL_FUNCTION( a ) vkGetInstanceProcAddr( vkInstance.instance , a )
#define VK_DEVICE_LEVEL_FUNCTION( a ) vkGetDeviceProcAddr( vkInstance.device , a )


qboolean VK_LoadGlobalFunctions(void)
{
	vkCreateInstance = VK_GLOBAL_LEVEL_FUNCTION("vkCreateInstance");
	vkEnumerateInstanceExtensionProperties = VK_GLOBAL_LEVEL_FUNCTION("vkEnumerateInstanceExtensionProperties");
	vkEnumerateInstanceLayerProperties = VK_GLOBAL_LEVEL_FUNCTION("vkEnumerateInstanceLayerProperties");
}

qboolean VK_LoadInstanceFunctions(void)
{
	vkEnumeratePhysicalDevices = VK_INSTANCE_LEVEL_FUNCTION("vkEnumeratePhysicalDevices");
	vkEnumerateDeviceExtensionProperties = VK_INSTANCE_LEVEL_FUNCTION("vkEnumerateDeviceExtensionProperties");

	/* Surface */
	vkCreateWin32SurfaceKHR = VK_INSTANCE_LEVEL_FUNCTION("vkCreateWin32SurfaceKHR");

	/* Physical Device */
	vkGetPhysicalDeviceProperties = VK_INSTANCE_LEVEL_FUNCTION("vkGetPhysicalDeviceProperties");
	vkGetPhysicalDeviceSurfaceSupportKHR = VK_INSTANCE_LEVEL_FUNCTION("vkGetPhysicalDeviceSurfaceSupportKHR");
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR = VK_INSTANCE_LEVEL_FUNCTION("vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
	vkGetPhysicalDeviceSurfaceSupportKHR = VK_INSTANCE_LEVEL_FUNCTION("vkGetPhysicalDeviceSurfaceSupportKHR");
	vkGetPhysicalDeviceQueueFamilyProperties = VK_INSTANCE_LEVEL_FUNCTION("vkGetPhysicalDeviceQueueFamilyProperties");
	vkGetPhysicalDeviceSurfacePresentModesKHR = VK_INSTANCE_LEVEL_FUNCTION("vkGetPhysicalDeviceSurfacePresentModesKHR");
	vkGetPhysicalDeviceSurfaceFormatsKHR = VK_INSTANCE_LEVEL_FUNCTION("vkGetPhysicalDeviceSurfaceFormatsKHR");

	/* Device */
	vkGetDeviceProcAddr = VK_INSTANCE_LEVEL_FUNCTION("vkGetDeviceProcAddr");
	vkCreateDevice = VK_INSTANCE_LEVEL_FUNCTION("vkCreateDevice");

	return qtrue;
}

qboolean VK_LoadDeviceFunctions(void)
{
	vkGetDeviceQueue = VK_DEVICE_LEVEL_FUNCTION("vkGetDeviceQueue");
	vkCreateCommandPool = VK_DEVICE_LEVEL_FUNCTION("vkCreateCommandPool");

	/* Debug */
#ifndef NDEBUG
	vkCreateDebugUtilsMessengerEXT = VK_DEVICE_LEVEL_FUNCTION("vkCreateDebugUtilsMessengerEXT");
	vkDestroyDebugUtilsMessengerEXT = VK_DEVICE_LEVEL_FUNCTION("vkDestroyDebugUtilsMessengerEXT");
#endif
}
