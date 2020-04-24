
#include "../tr_local.h"


/*
** vkGetInstanceProcAddr needs to be set by platform specific implementation
*/ 
#define VK_GLOBAL_LEVEL_FUNCTION( a ) vkGetInstanceProcAddr( NULL, a ); if(!a) return qfalse
#define VK_INSTANCE_LEVEL_FUNCTION( a ) vkGetInstanceProcAddr( vk.instance , a ); if(!a) return qfalse
#define VK_DEVICE_LEVEL_FUNCTION( a ) vkGetDeviceProcAddr( vk.device , a ); if(!a) return qfalse


qboolean VK_LoadGlobalFunctions(void)
{
	vkCreateInstance = VK_GLOBAL_LEVEL_FUNCTION("vkCreateInstance");
	vkEnumerateInstanceExtensionProperties = VK_GLOBAL_LEVEL_FUNCTION("vkEnumerateInstanceExtensionProperties");
	vkEnumerateInstanceLayerProperties = VK_GLOBAL_LEVEL_FUNCTION("vkEnumerateInstanceLayerProperties");
    
    return qtrue;
}

qboolean VK_LoadInstanceFunctions(void)
{
    vkDestroyInstance = VK_INSTANCE_LEVEL_FUNCTION("vkDestroyInstance");
    
	vkEnumeratePhysicalDevices = VK_INSTANCE_LEVEL_FUNCTION("vkEnumeratePhysicalDevices");
	vkEnumerateDeviceExtensionProperties = VK_INSTANCE_LEVEL_FUNCTION("vkEnumerateDeviceExtensionProperties");
	vkGetPhysicalDeviceMemoryProperties = VK_INSTANCE_LEVEL_FUNCTION("vkGetPhysicalDeviceMemoryProperties");

	/* Surface */
#if defined( _WIN32 )
    vkCreateWin32SurfaceKHR = VK_INSTANCE_LEVEL_FUNCTION("vkCreateWin32SurfaceKHR");
#elif defined(__APPLE__)
    vkCreateMacOSSurfaceMVK = VK_INSTANCE_LEVEL_FUNCTION("vkCreateMacOSSurfaceMVK");
#elif defined( __linux__ )
    
#endif
    vkDestroySurfaceKHR = VK_INSTANCE_LEVEL_FUNCTION("vkDestroySurfaceKHR");
	
	/* Physical Device */
	vkGetPhysicalDeviceProperties = VK_INSTANCE_LEVEL_FUNCTION("vkGetPhysicalDeviceProperties");
	vkGetPhysicalDeviceProperties2 = VK_INSTANCE_LEVEL_FUNCTION("vkGetPhysicalDeviceProperties2");
	vkGetPhysicalDeviceSurfaceSupportKHR = VK_INSTANCE_LEVEL_FUNCTION("vkGetPhysicalDeviceSurfaceSupportKHR");
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR = VK_INSTANCE_LEVEL_FUNCTION("vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
	vkGetPhysicalDeviceSurfaceSupportKHR = VK_INSTANCE_LEVEL_FUNCTION("vkGetPhysicalDeviceSurfaceSupportKHR");
	vkGetPhysicalDeviceQueueFamilyProperties = VK_INSTANCE_LEVEL_FUNCTION("vkGetPhysicalDeviceQueueFamilyProperties");
	vkGetPhysicalDeviceSurfacePresentModesKHR = VK_INSTANCE_LEVEL_FUNCTION("vkGetPhysicalDeviceSurfacePresentModesKHR");
	vkGetPhysicalDeviceSurfaceFormatsKHR = VK_INSTANCE_LEVEL_FUNCTION("vkGetPhysicalDeviceSurfaceFormatsKHR");

	/* Device */
	vkGetDeviceProcAddr = VK_INSTANCE_LEVEL_FUNCTION("vkGetDeviceProcAddr");
	vkCreateDevice = VK_INSTANCE_LEVEL_FUNCTION("vkCreateDevice");
    
    /* Debug */
#ifndef NDEBUG
    vkCreateDebugUtilsMessengerEXT = VK_INSTANCE_LEVEL_FUNCTION("vkCreateDebugUtilsMessengerEXT");
    vkDestroyDebugUtilsMessengerEXT = VK_INSTANCE_LEVEL_FUNCTION("vkDestroyDebugUtilsMessengerEXT");
#endif

	return qtrue;
}

qboolean VK_LoadDeviceFunctions(void)
{
	vkGetDeviceQueue = VK_DEVICE_LEVEL_FUNCTION("vkGetDeviceQueue");
	vkCreateCommandPool = VK_DEVICE_LEVEL_FUNCTION("vkCreateCommandPool");
	vkCreateSwapchainKHR = VK_DEVICE_LEVEL_FUNCTION("vkCreateSwapchainKHR");
	vkGetSwapchainImagesKHR = VK_DEVICE_LEVEL_FUNCTION("vkGetSwapchainImagesKHR");
	vkCreateImageView = VK_DEVICE_LEVEL_FUNCTION("vkCreateImageView");
	vkCreateSampler = VK_DEVICE_LEVEL_FUNCTION("vkCreateSampler");
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
	vkQueueWaitIdle = VK_DEVICE_LEVEL_FUNCTION("vkQueueWaitIdle");
	vkQueuePresentKHR = VK_DEVICE_LEVEL_FUNCTION("vkQueuePresentKHR");

	vkCmdBeginRenderPass = VK_DEVICE_LEVEL_FUNCTION("vkCmdBeginRenderPass");
	vkCmdSetViewport = VK_DEVICE_LEVEL_FUNCTION("vkCmdSetViewport");
	vkCmdSetScissor = VK_DEVICE_LEVEL_FUNCTION("vkCmdSetScissor");
	vkCmdEndRenderPass = VK_DEVICE_LEVEL_FUNCTION("vkCmdEndRenderPass");
    vkCmdBindVertexBuffers = VK_DEVICE_LEVEL_FUNCTION("vkCmdBindVertexBuffers");
    vkCmdBindIndexBuffer = VK_DEVICE_LEVEL_FUNCTION("vkCmdBindIndexBuffer");
    vkCmdPushConstants = VK_DEVICE_LEVEL_FUNCTION("vkCmdPushConstants");
    vkCmdClearAttachments = VK_DEVICE_LEVEL_FUNCTION("vkCmdClearAttachments");
	vkCmdClearColorImage = VK_DEVICE_LEVEL_FUNCTION("vkCmdClearColorImage");
    vkCmdPushConstants = VK_DEVICE_LEVEL_FUNCTION("vkCmdPushConstants");

	vkCreateImage = VK_DEVICE_LEVEL_FUNCTION("vkCreateImage");
	vkGetImageMemoryRequirements = VK_DEVICE_LEVEL_FUNCTION("vkGetImageMemoryRequirements");
	vkGetBufferMemoryRequirements = VK_DEVICE_LEVEL_FUNCTION("vkGetBufferMemoryRequirements");
	vkGetBufferMemoryRequirements2 = VK_DEVICE_LEVEL_FUNCTION("vkGetBufferMemoryRequirements2");
	vkGetImageSubresourceLayout = VK_DEVICE_LEVEL_FUNCTION("vkGetImageSubresourceLayout");

	vkCreateBuffer = VK_DEVICE_LEVEL_FUNCTION("vkCreateBuffer");
	vkAllocateMemory = VK_DEVICE_LEVEL_FUNCTION("vkAllocateMemory");
	vkBindBufferMemory = VK_DEVICE_LEVEL_FUNCTION("vkBindBufferMemory");
	vkBindImageMemory = VK_DEVICE_LEVEL_FUNCTION("vkBindImageMemory");
	vkMapMemory = VK_DEVICE_LEVEL_FUNCTION("vkMapMemory");
	vkUnmapMemory = VK_DEVICE_LEVEL_FUNCTION("vkUnmapMemory");

	vkDestroyBuffer = VK_DEVICE_LEVEL_FUNCTION("vkDestroyBuffer");
	vkFreeMemory = VK_DEVICE_LEVEL_FUNCTION("vkFreeMemory");

	vkAllocateCommandBuffers = VK_DEVICE_LEVEL_FUNCTION("vkAllocateCommandBuffers");
	vkBeginCommandBuffer = VK_DEVICE_LEVEL_FUNCTION("vkBeginCommandBuffer");
	vkEndCommandBuffer = VK_DEVICE_LEVEL_FUNCTION("vkEndCommandBuffer");
	vkFreeCommandBuffers = VK_DEVICE_LEVEL_FUNCTION("vkFreeCommandBuffers");

	vkCmdPipelineBarrier = VK_DEVICE_LEVEL_FUNCTION("vkCmdPipelineBarrier");
	vkCmdCopyBufferToImage = VK_DEVICE_LEVEL_FUNCTION("vkCmdCopyBufferToImage");
    vkCmdBindPipeline = VK_DEVICE_LEVEL_FUNCTION("vkCmdBindPipeline");
    vkCmdBindDescriptorSets = VK_DEVICE_LEVEL_FUNCTION("vkCmdBindDescriptorSets");
    vkCmdBindVertexBuffers = VK_DEVICE_LEVEL_FUNCTION("vkCmdBindVertexBuffers");
    vkCmdDraw = VK_DEVICE_LEVEL_FUNCTION("vkCmdDraw");
    vkCmdDrawIndexed = VK_DEVICE_LEVEL_FUNCTION("vkCmdDrawIndexed");
    vkCmdClearAttachments = VK_DEVICE_LEVEL_FUNCTION("vkCmdClearAttachments");
    vkCmdSetDepthBias = VK_DEVICE_LEVEL_FUNCTION("vkCmdSetDepthBias");
    vkCmdSetBlendConstants = VK_DEVICE_LEVEL_FUNCTION("vkCmdSetBlendConstants");
	vkCmdCopyImage = VK_DEVICE_LEVEL_FUNCTION("vkCmdCopyImage");
	vkCmdDispatch = VK_DEVICE_LEVEL_FUNCTION("vkCmdDispatch");
	vkCmdWriteTimestamp = VK_DEVICE_LEVEL_FUNCTION("vkCmdWriteTimestamp");
	vkCmdBeginDebugUtilsLabelEXT = VK_DEVICE_LEVEL_FUNCTION("vkCmdBeginDebugUtilsLabelEXT");
	vkCmdEndDebugUtilsLabelEXT = VK_DEVICE_LEVEL_FUNCTION("vkCmdEndDebugUtilsLabelEXT");

	vkCreateQueryPool = VK_DEVICE_LEVEL_FUNCTION("vkCreateQueryPool");
	vkGetQueryPoolResults = VK_DEVICE_LEVEL_FUNCTION("vkGetQueryPoolResults");
	vkCmdResetQueryPool = VK_DEVICE_LEVEL_FUNCTION("vkCmdResetQueryPool");
	vkDestroyQueryPool = VK_DEVICE_LEVEL_FUNCTION("vkDestroyQueryPool");
    vkCreatePipelineCache = VK_DEVICE_LEVEL_FUNCTION("vkCreatePipelineCache");
    vkCreatePipelineLayout = VK_DEVICE_LEVEL_FUNCTION("vkCreatePipelineLayout");
    vkCreateGraphicsPipelines = VK_DEVICE_LEVEL_FUNCTION("vkCreateGraphicsPipelines");
	vkCreateComputePipelines = VK_DEVICE_LEVEL_FUNCTION("vkCreateComputePipelines");
    
    vkCreateShaderModule = VK_DEVICE_LEVEL_FUNCTION("vkCreateShaderModule");
    vkCreateDescriptorSetLayout = VK_DEVICE_LEVEL_FUNCTION("vkCreateDescriptorSetLayout");
    vkCreateDescriptorPool = VK_DEVICE_LEVEL_FUNCTION("vkCreateDescriptorPool");
    vkUpdateDescriptorSets = VK_DEVICE_LEVEL_FUNCTION("vkUpdateDescriptorSets");
    
    vkDestroySampler = VK_DEVICE_LEVEL_FUNCTION("vkDestroySampler");
    vkDestroyImage = VK_DEVICE_LEVEL_FUNCTION("vkDestroyImage");
    vkDestroyImageView = VK_DEVICE_LEVEL_FUNCTION("vkDestroyImageView");
	vkDestroyFramebuffer = VK_DEVICE_LEVEL_FUNCTION("vkDestroyFramebuffer");
    vkFreeDescriptorSets = VK_DEVICE_LEVEL_FUNCTION("vkFreeDescriptorSets");
    vkDestroyDescriptorSetLayout = VK_DEVICE_LEVEL_FUNCTION("vkDestroyDescriptorSetLayout");
    vkDestroyDescriptorPool = VK_DEVICE_LEVEL_FUNCTION("vkDestroyDescriptorPool");
	vkDestroyRenderPass = VK_DEVICE_LEVEL_FUNCTION("vkDestroyRenderPass");
	vkDestroySwapchainKHR = VK_DEVICE_LEVEL_FUNCTION("vkDestroySwapchainKHR");
	vkDestroySemaphore = VK_DEVICE_LEVEL_FUNCTION("vkDestroySemaphore");
	vkDestroyFence = VK_DEVICE_LEVEL_FUNCTION("vkDestroyFence");
	vkDestroyCommandPool = VK_DEVICE_LEVEL_FUNCTION("vkDestroyCommandPool");
	vkDestroyDevice = VK_DEVICE_LEVEL_FUNCTION("vkDestroyDevice");

	vkDeviceWaitIdle = VK_DEVICE_LEVEL_FUNCTION("vkDeviceWaitIdle");
	vkWaitForFences = VK_DEVICE_LEVEL_FUNCTION("vkWaitForFences");
    vkGetFenceStatus = VK_DEVICE_LEVEL_FUNCTION("vkGetFenceStatus");
	vkAllocateDescriptorSets = VK_DEVICE_LEVEL_FUNCTION("vkAllocateDescriptorSets");

	vkDestroyShaderModule = VK_DEVICE_LEVEL_FUNCTION("vkDestroyShaderModule");
	vkDestroyPipeline = VK_DEVICE_LEVEL_FUNCTION("vkDestroyPipeline");
	vkDestroyPipelineLayout = VK_DEVICE_LEVEL_FUNCTION("vkDestroyPipelineLayout");
	vkDestroyPipelineCache = VK_DEVICE_LEVEL_FUNCTION("vkDestroyPipelineCache");

	/*
	** NV RTX
	*/
	vkCreateAccelerationStructureNV = VK_DEVICE_LEVEL_FUNCTION("vkCreateAccelerationStructureNV");
	vkDestroyAccelerationStructureNV = VK_DEVICE_LEVEL_FUNCTION("vkDestroyAccelerationStructureNV");
	vkBindAccelerationStructureMemoryNV = VK_DEVICE_LEVEL_FUNCTION("vkBindAccelerationStructureMemoryNV");
	vkGetAccelerationStructureHandleNV = VK_DEVICE_LEVEL_FUNCTION("vkGetAccelerationStructureHandleNV");
	vkGetAccelerationStructureMemoryRequirementsNV = VK_DEVICE_LEVEL_FUNCTION("vkGetAccelerationStructureMemoryRequirementsNV");
	vkCmdBuildAccelerationStructureNV = VK_DEVICE_LEVEL_FUNCTION("vkCmdBuildAccelerationStructureNV");
	vkCreateRayTracingPipelinesNV = VK_DEVICE_LEVEL_FUNCTION("vkCreateRayTracingPipelinesNV");
	vkGetRayTracingShaderGroupHandlesNV = VK_DEVICE_LEVEL_FUNCTION("vkGetRayTracingShaderGroupHandlesNV");
	vkCmdTraceRaysNV = VK_DEVICE_LEVEL_FUNCTION("vkCmdTraceRaysNV");

    return qtrue;
}
