#include "../tr_local.h"

void createSwapChain() {
	
	swapChainSupportDetails_t swapChainSupport = querySwapChainSupport(vkInstance.physical_device, vkInstance.surface);

	//VkSurfaceFormatKHR surfaceFormat = vulkan_utils::swapchain::chooseSwapSurfaceFormat(swapChainSupport.formats);
	//VkPresentModeKHR presentMode = vulkan_utils::swapchain::chooseSwapPresentMode(swapChainSupport.presentModes);
	//VkExtent2D extent = swapChainSupport.capabilities.currentExtent;//vulkan_utils::swapchain::chooseSwapExtent(swapChainSupport.capabilities);

	//extent.width = std::max(swapChainSupport.capabilities.minImageExtent.width, std::min(swapChainSupport.capabilities.maxImageExtent.width, extent.width));
	//extent.height = std::max(swapChainSupport.capabilities.minImageExtent.height, std::min(swapChainSupport.capabilities.maxImageExtent.height, extent.height));

	//extent.width = WIDTH;
	//extent.height = HEIGHT;

	//uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	//if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
	//	imageCount = swapChainSupport.capabilities.maxImageCount;
	//}

	//VkSwapchainCreateInfoKHR createInfo = {};
	//createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	//createInfo.surface = this->surface;

	//createInfo.minImageCount = imageCount;
	//createInfo.imageFormat = surfaceFormat.format;
	//createInfo.imageColorSpace = surfaceFormat.colorSpace;
	//createInfo.imageExtent = extent;
	//createInfo.imageArrayLayers = 1;
	//createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	//QueueFamilyIndices indices = vulkan_utils::setup::findQueueFamilies(this->physicalDevice, this->surface);
	//uint32_t queueFamilyIndices[] = { indices.graphicsFamily, indices.presentFamily };

	//if (indices.graphicsFamily != indices.presentFamily) {
	//	createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
	//	createInfo.queueFamilyIndexCount = 2;
	//	createInfo.pQueueFamilyIndices = queueFamilyIndices;
	//}
	//else {
	//	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	//}

	//createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	//createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	//createInfo.presentMode = presentMode;
	//createInfo.clipped = VK_TRUE;
	//createInfo.oldSwapchain = VK_NULL_HANDLE;

	//VK_CHECK(vkCreateSwapchainKHR(this->device, &createInfo, NULL, &this->swapChain));

	//vkGetSwapchainImagesKHR(this->device, this->swapChain, &imageCount, NULL);
	//this->swapChainImages.resize(imageCount);
	//vkGetSwapchainImagesKHR(this->device, this->swapChain, &imageCount, this->swapChainImages.data());

	//this->swapChainImageFormat = surfaceFormat.format;
	//this->swapChainExtent = extent;
}


