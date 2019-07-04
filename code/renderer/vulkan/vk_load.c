
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
    
    return qtrue;
}

qboolean VK_LoadInstanceFunctions(void)
{
	vkEnumeratePhysicalDevices = VK_INSTANCE_LEVEL_FUNCTION("vkEnumeratePhysicalDevices");
	vkEnumerateDeviceExtensionProperties = VK_INSTANCE_LEVEL_FUNCTION("vkEnumerateDeviceExtensionProperties");

	/* Surface */
#if defined( _WIN32 )
    vkCreateWin32SurfaceKHR = VK_INSTANCE_LEVEL_FUNCTION("vkCreateWin32SurfaceKHR");
#elif defined(__APPLE__)
    vkCreateMacOSSurfaceMVK = VK_INSTANCE_LEVEL_FUNCTION("vkCreateMacOSSurfaceMVK");
#elif defined( __linux__ )
    
#endif
	
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
	vkCreateSwapchainKHR = VK_DEVICE_LEVEL_FUNCTION("vkCreateSwapchainKHR");
	vkGetSwapchainImagesKHR = VK_DEVICE_LEVEL_FUNCTION("vkGetSwapchainImagesKHR");
	vkCreateImageView = VK_DEVICE_LEVEL_FUNCTION("vkCreateImageView");
	vkCreateRenderPass = VK_DEVICE_LEVEL_FUNCTION("vkCreateRenderPass");
	vkCreateFramebuffer = VK_DEVICE_LEVEL_FUNCTION("vkCreateFramebuffer");
	vkAllocateCommandBuffers = VK_DEVICE_LEVEL_FUNCTION("vkAllocateCommandBuffers");
	vkCreateSemaphore = VK_DEVICE_LEVEL_FUNCTION("vkCreateSemaphore");
	vkCreateFence = VK_DEVICE_LEVEL_FUNCTION("vkCreateFence");

	vkWaitForFences = VK_DEVICE_LEVEL_FUNCTION("vkWaitForFences");
	vkResetFences = VK_DEVICE_LEVEL_FUNCTION("vkResetFences");
	vkAcquireNextImageKHR = VK_DEVICE_LEVEL_FUNCTION("vkAcquireNextImageKHR");
	vkFreeCommandBuffers = VK_DEVICE_LEVEL_FUNCTION("vkFreeCommandBuffers");
	vkBeginCommandBuffer = VK_DEVICE_LEVEL_FUNCTION("vkBeginCommandBuffer");

	vkEndCommandBuffer = VK_DEVICE_LEVEL_FUNCTION("vkEndCommandBuffer");
	vkQueueSubmit = VK_DEVICE_LEVEL_FUNCTION("vkQueueSubmit");
	vkQueuePresentKHR = VK_DEVICE_LEVEL_FUNCTION("vkQueuePresentKHR");

	vkCmdBeginRenderPass = VK_DEVICE_LEVEL_FUNCTION("vkCmdBeginRenderPass");
	vkCmdSetViewport = VK_DEVICE_LEVEL_FUNCTION("vkCmdSetViewport");
	vkCmdSetScissor = VK_DEVICE_LEVEL_FUNCTION("vkCmdSetScissor");
	vkCmdEndRenderPass = VK_DEVICE_LEVEL_FUNCTION("vkCmdEndRenderPass");

	/* Debug */
#ifndef NDEBUG
	vkCreateDebugUtilsMessengerEXT = VK_DEVICE_LEVEL_FUNCTION("vkCreateDebugUtilsMessengerEXT");
	vkDestroyDebugUtilsMessengerEXT = VK_DEVICE_LEVEL_FUNCTION("vkDestroyDebugUtilsMessengerEXT");
#endif
    return qtrue;
}
