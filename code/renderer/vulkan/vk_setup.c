
#include "../tr_local.h"

#define MAX_EXTENSION_PROPERTIES 50

const char* deviceExtensions[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const char* validationLayer[] = {
		"VK_LAYER_LUNARG_standard_validation"
};

//
// function declaration
//
static qboolean isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);
queueFamilyIndices_t findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
static qboolean checkValidationLayerSupport();

void VK_CreateInstance() {
	const char* instance_extensions[] = {
		VK_KHR_SURFACE_EXTENSION_NAME,
#if defined( _WIN32 )
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif defined(__APPLE__)
        VK_MVK_MACOS_SURFACE_EXTENSION_NAME,
#elif defined( __linux__ )
        
#endif
#ifndef NDEBUG
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME
#endif
	};

	// check extensions availability
	{
		uint32_t count = 0;
		vkEnumerateInstanceExtensionProperties(NULL, &count, NULL);
		VkExtensionProperties extension_properties[MAX_EXTENSION_PROPERTIES];
		vkEnumerateInstanceExtensionProperties(NULL, &count, &extension_properties[0]);


		for (int i = 0; i < (sizeof(instance_extensions) / sizeof(instance_extensions[0])); i++) {
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

	checkValidationLayerSupport();

	// create instance
	{
		VkInstanceCreateInfo desc = { 0 };
		desc.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		desc.pNext = NULL;
		desc.flags = 0;
		desc.pApplicationInfo = NULL;
		desc.enabledLayerCount = 0;
		desc.ppEnabledLayerNames = NULL;
		desc.enabledExtensionCount = sizeof(instance_extensions) / sizeof(instance_extensions[0]);
		desc.ppEnabledExtensionNames = instance_extensions;

#ifndef NDEBUG
		desc.enabledLayerCount = (uint32_t)(sizeof(validationLayer) / sizeof(validationLayer[0]));
		desc.ppEnabledLayerNames = &validationLayer[0];
#endif

		VK_CHECK(vkCreateInstance(&desc, NULL, &vkInstance.instance), "failed to create Instance!");
	}
}

/*
** VK_CreateSurface
**
** win: (hinstance, hwnd)
*/
void VK_CreateSurface(void* p1, void* p2) {
#ifdef WIN32
	VkWin32SurfaceCreateInfoKHR desc = {0};
	desc.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	desc.pNext = NULL;
	desc.flags = 0;
	desc.hinstance = p1;
	desc.hwnd = p2;
	VK_CHECK(vkCreateWin32SurfaceKHR(vkInstance.instance, &desc, NULL, &vkInstance.surface), "failed to create Win32 Surface!");
#endif
}

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
		VkExtensionProperties extensions[MAX_EXTENSION_PROPERTIES];
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

void VK_CreateLogicalDevice() {
	queueFamilyIndices_t indices = findQueueFamilies(vkInstance.physical_device, vkInstance.surface);

	VkDeviceQueueCreateInfo queueCreateInfos[2];
	memset(&queueCreateInfos, 0, sizeof(queueCreateInfos));

	uint32_t queueCreateInfosCount = indices.graphicsFamily == indices.presentFamily ? 1 : 2;
	float queuePriority = 1.0f;
	queueCreateInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfos[0].queueFamilyIndex = indices.graphicsFamily;
	queueCreateInfos[0].queueCount = 1;
	queueCreateInfos[0].pQueuePriorities = &queuePriority;

	if (indices.graphicsFamily != indices.presentFamily) {
		queueCreateInfos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfos[1].queueFamilyIndex = indices.presentFamily;
		queueCreateInfos[1].queueCount = 1;
		queueCreateInfos[1].pQueuePriorities = &queuePriority;
	}

	// activate features
	VkPhysicalDeviceFeatures deviceFeatures = { 0 };
	deviceFeatures.multiDrawIndirect = qfalse;
	deviceFeatures.drawIndirectFirstInstance = qfalse;

	VkDeviceCreateInfo createInfo = { 0 };
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.queueCreateInfoCount = queueCreateInfosCount;
	createInfo.pQueueCreateInfos = &queueCreateInfos[0];

	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = (uint32_t)(sizeof(deviceExtensions) / sizeof(deviceExtensions[0]));
	createInfo.ppEnabledExtensionNames = &deviceExtensions[0];

	createInfo.enabledLayerCount = 0;

	VK_CHECK(vkCreateDevice(vkInstance.physical_device, &createInfo, NULL, &vkInstance.device), "failed to create logical device!")

}

void VK_CreateCommandPool() {
	queueFamilyIndices_t queueFamilyIndices = findQueueFamilies(vkInstance.physical_device, vkInstance.surface);

	vkGetDeviceQueue(vkInstance.device, queueFamilyIndices.graphicsFamily, 0, &vkInstance.graphicsQueue);
	vkGetDeviceQueue(vkInstance.device, queueFamilyIndices.presentFamily, 0, &vkInstance.presentQueue);

	VkCommandPoolCreateInfo poolInfo = { 0 };
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;//VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

	VK_CHECK(vkCreateCommandPool(vkInstance.device, &poolInfo, NULL, &vkInstance.commandPool), "failed to create command pool!");


}

void VK_CreateCommandBuffers() {
	vkInstance.swapchain.commandBuffers = malloc(vkInstance.swapchain.imageCount * sizeof(VkCommandBuffer));

	VkCommandBufferAllocateInfo allocInfo = {0};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = vkInstance.commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = vkInstance.swapchain.imageCount;

	VK_CHECK(vkAllocateCommandBuffers(vkInstance.device, &allocInfo, vkInstance.swapchain.commandBuffers), "failed to allocate command buffers!");

}

void VK_CreateSyncObjects() {
	vkInstance.swapchain.imageAvailableSemaphores = malloc(vkInstance.swapchain.imageCount * sizeof(VkSemaphore));
	vkInstance.swapchain.renderFinishedSemaphores = malloc(vkInstance.swapchain.imageCount * sizeof(VkSemaphore));
	vkInstance.swapchain.inFlightFences = malloc(vkInstance.swapchain.imageCount * sizeof(VkFence));

	VkSemaphoreCreateInfo semaphoreInfo = {0};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {0};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < vkInstance.swapchain.imageCount; i++) {
		VK_CHECK(vkCreateSemaphore(vkInstance.device, &semaphoreInfo, NULL, &vkInstance.swapchain.imageAvailableSemaphores[i]), "failed to create Semaphore!");
		VK_CHECK(vkCreateSemaphore(vkInstance.device, &semaphoreInfo, NULL, &vkInstance.swapchain.renderFinishedSemaphores[i]), "failed to create Semaphore!");
		VK_CHECK(vkCreateFence(vkInstance.device, &fenceInfo, NULL, &vkInstance.swapchain.inFlightFences[i]), "failed to create Fence!");
	}

	vkInstance.swapchain.currentFrame = vkInstance.swapchain.imageCount - 1;
}

void VK_BeginFrame() {

	// wait for command buffer submission for last image
	vkWaitForFences(vkInstance.device, 1, &vkInstance.swapchain.inFlightFences[vkInstance.swapchain.currentFrame], VK_TRUE, UINT64_MAX);
	vkResetFences(vkInstance.device, 1, &vkInstance.swapchain.inFlightFences[vkInstance.swapchain.currentFrame]);

	vkAcquireNextImageKHR(vkInstance.device, vkInstance.swapchain.handle, UINT64_MAX, vkInstance.swapchain.imageAvailableSemaphores[vkInstance.swapchain.currentFrame], vkInstance.swapchain.inFlightFences[vkInstance.swapchain.currentFrame], &vkInstance.swapchain.currentImage);

	vkWaitForFences(vkInstance.device, 1, &vkInstance.swapchain.inFlightFences[vkInstance.swapchain.currentImage], VK_TRUE, UINT64_MAX);
	vkResetFences(vkInstance.device, 1, &vkInstance.swapchain.inFlightFences[vkInstance.swapchain.currentImage]);

	vkFreeCommandBuffers(vkInstance.device, vkInstance.commandPool, 1, &vkInstance.swapchain.commandBuffers[vkInstance.swapchain.currentImage]);
	VkCommandBufferAllocateInfo cmdBufInfo = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, NULL, vkInstance.commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1 };
	VkResult err = vkAllocateCommandBuffers(vkInstance.device, &cmdBufInfo, &vkInstance.swapchain.commandBuffers[vkInstance.swapchain.currentImage]);

	VkCommandBufferBeginInfo beginInfo = {0};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;//VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	VK_CHECK(vkBeginCommandBuffer(vkInstance.swapchain.commandBuffers[vkInstance.swapchain.currentImage], &beginInfo), "failed to begin recording command buffer!");
}

void VK_DrawFrame() {

	VK_CHECK(vkEndCommandBuffer(vkInstance.swapchain.commandBuffers[vkInstance.swapchain.currentImage]), "failed to end commandbuffer!");
	//vkResetFences(this->device, 1, &inFlightFences[currentFrame]);

	VkSemaphore waitSemaphores[] = { vkInstance.swapchain.imageAvailableSemaphores[vkInstance.swapchain.currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSemaphore signalSemaphores[] = { vkInstance.swapchain.renderFinishedSemaphores[vkInstance.swapchain.currentFrame] };

	VkSubmitInfo submitInfo = {0};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &vkInstance.swapchain.commandBuffers[vkInstance.swapchain.currentImage];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	VK_CHECK(vkQueueSubmit(vkInstance.graphicsQueue, 1, &submitInfo, vkInstance.swapchain.inFlightFences[vkInstance.swapchain.currentImage]), "failed to submit draw command buffer!");
	

	VkSwapchainKHR swapChains[] = { vkInstance.swapchain.handle };

	VkPresentInfoKHR presentInfo = {0};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &vkInstance.swapchain.currentImage;

	VK_CHECK(vkQueuePresentKHR(vkInstance.presentQueue, &presentInfo), "failed to Queue Present!");

	vkInstance.swapchain.currentFrame = (vkInstance.swapchain.currentFrame + 1) % vkInstance.swapchain.imageCount;
}

/*
==============================================================================

Vulkan Debug Function

==============================================================================
*/
static qboolean checkValidationLayerSupport() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, NULL);
	//VkLayerProperties* availableLayers = malloc(layerCount * sizeof(VkLayerProperties));
	VkLayerProperties availableLayers[20];
	vkEnumerateInstanceLayerProperties(&layerCount, &availableLayers[0]);

	for (int i = 0; i < (sizeof(validationLayer) / sizeof(validationLayer[0])); i++) {
		qboolean layerFound = qfalse;
		for (int j = 0; j < layerCount; j++) {
			if (!strcmp(availableLayers[j].layerName, validationLayer[i])) {
				layerFound = qtrue;
				break;
			}
		}
		if (!layerFound) {
			return qfalse;
		}
	}

	return qtrue;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL VK_DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
	ri.Printf(PRINT_WARNING, "Vulkan Validation Layer: %s", pCallbackData->pMessage);

	return VK_FALSE;
}

void VK_SetupDebugCallback() {
	VkDebugUtilsMessengerCreateInfoEXT createInfo = { 0 };
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = &VK_DebugCallback;

	VK_CHECK(vkCreateDebugUtilsMessengerEXT(vkInstance.instance, &createInfo, NULL, &vkInstance.callback), "failed to set up debug callback!");
}

/*
==============================================================================

Vulkan Helper Function

==============================================================================
*/

queueFamilyIndices_t findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {

	queueFamilyIndices_t indices;
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

static qboolean checkDeviceExtensionSupport(VkPhysicalDevice device) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);
	VkExtensionProperties availableExtensions[60];
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
	return qtrue;
}

static qboolean isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
	queueFamilyIndices_t q = findQueueFamilies(device, surface);

	qboolean extensionsSupported = checkDeviceExtensionSupport(device);

	qboolean swapChainAdequate = qfalse;
	if (extensionsSupported) {
		swapChainSupportDetails_t swapChainSupport = querySwapChainSupport(device, surface);
		swapChainAdequate = swapChainSupport.formatCount && swapChainSupport.presentModeCount;
	}

	return q.graphicsFamily >= 0 && q.presentFamily >= 0 && extensionsSupported && swapChainAdequate;;
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