#include "../tr_local.h"
#include <stdlib.h>

#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

static VkSurfaceFormatKHR chooseSwapSurfaceFormat(VkSurfaceFormatKHR* availableFormats, uint32_t availableFormatsCount);
static VkPresentModeKHR chooseSwapPresentMode(VkPresentModeKHR* availablePresentModes, uint32_t availablePresentModesCount);

void VK_CreateSwapChain() {
	
	swapChainSupportDetails_t swapChainSupport = querySwapChainSupport(vkInstance.physical_device, vkInstance.surface);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(&swapChainSupport.formats[0], swapChainSupport.formatCount);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(&swapChainSupport.presentModes[0], swapChainSupport.presentModeCount);
	
	// set extent
	vkInstance.swapchain.extent = swapChainSupport.capabilities.currentExtent;
	vkInstance.swapchain.extent.width = max(swapChainSupport.capabilities.minImageExtent.width, min(swapChainSupport.capabilities.maxImageExtent.width, vkInstance.swapchain.extent.width));
	vkInstance.swapchain.extent.height = max(swapChainSupport.capabilities.minImageExtent.height, min(swapChainSupport.capabilities.maxImageExtent.height, vkInstance.swapchain.extent.height));

	vkInstance.swapchain.imageFormat = surfaceFormat.format;

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {0};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = vkInstance.surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = vkInstance.swapchain.extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	queueFamilyIndices_t indices = findQueueFamilies(vkInstance.physical_device, vkInstance.surface);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily, indices.presentFamily };

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	VK_CHECK(vkCreateSwapchainKHR(vkInstance.device, &createInfo, NULL, &vkInstance.swapchain.handle), "failed to create Swapchain!");

	vkGetSwapchainImagesKHR(vkInstance.device, vkInstance.swapchain.handle, &vkInstance.swapchain.imageCount, NULL);
	vkInstance.swapchain.images = malloc(vkInstance.swapchain.imageCount * sizeof(VkImage));
	vkGetSwapchainImagesKHR(vkInstance.device, vkInstance.swapchain.handle, &imageCount, &vkInstance.swapchain.images[0]);

}

void VK_CreateImageViews() {
	vkInstance.swapchain.imageViews = malloc(vkInstance.swapchain.imageCount * sizeof(VkImageView));

	for (size_t i = 0; i < vkInstance.swapchain.imageCount; i++) {
		VkImageViewCreateInfo createInfo = { 0 };
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = vkInstance.swapchain.images[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = vkInstance.swapchain.imageFormat;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		VK_CHECK(vkCreateImageView(vkInstance.device, &createInfo, NULL, &vkInstance.swapchain.imageViews[i]), "failed to create ImageView for Swapchain!");
	}
}

void VK_CreateRenderPass() {
	VkAttachmentDescription colorAttachment = {0};
	colorAttachment.format = vkInstance.swapchain.imageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {0};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment = {0};
	depthAttachment.format = vkInstance.swapchain.depthStencilFormat;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {0};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {0};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	//if (hasDepthStencil) subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency dependency = {0};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkAttachmentDescription attachments[2];
	attachments[0] = colorAttachment;
	//attachments[1] = depthAttachment;
	//std::vector<VkAttachmentDescription> attachments = { colorAttachment };
	//if (hasDepthStencil) attachments.push_back(depthAttachment);

	VkRenderPassCreateInfo renderPassInfo = {0};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = (uint32_t) 1;
	renderPassInfo.pAttachments = &attachments[0];
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	VK_CHECK(vkCreateRenderPass(vkInstance.device, &renderPassInfo, NULL, &vkInstance.swapchain.renderpass), "failed to create RenderPass for Swapchain!");
}

void VK_CreateFramebuffers() {
	vkInstance.swapchain.framebuffers = malloc(vkInstance.swapchain.imageCount * sizeof(VkFramebuffer));

	for (size_t i = 0; i < vkInstance.swapchain.imageCount; i++) {
		VkImageView attachments[2];
		attachments[0] = vkInstance.swapchain.imageViews[i];

		//std::vector<VkImageView> attachments = { swapChainImageViews[i] };
		//if (hasDepthStencil) attachments.push_back(depthImageView);

		VkFramebufferCreateInfo framebufferInfo = {0};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = vkInstance.swapchain.renderpass;
		framebufferInfo.attachmentCount = (uint32_t) 1;
		framebufferInfo.pAttachments = &attachments[0];
		framebufferInfo.width = vkInstance.swapchain.extent.width;
		framebufferInfo.height = vkInstance.swapchain.extent.height;
		framebufferInfo.layers = 1;

		VK_CHECK(vkCreateFramebuffer(vkInstance.device, &framebufferInfo, NULL, &vkInstance.swapchain.framebuffers[i]), "failed to create Framebuffer for Swapchain!");
	}
}

/*
==============================================================================

SwapChain Helper Function

==============================================================================
*/

static VkSurfaceFormatKHR chooseSwapSurfaceFormat(VkSurfaceFormatKHR *availableFormats, uint32_t availableFormatsCount) {

	if (availableFormatsCount == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
		return (VkSurfaceFormatKHR) {	.format = VK_FORMAT_B8G8R8A8_UNORM,
										.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
									};
	}

	for (int i = 0; i < availableFormatsCount; i++) {
		if (availableFormats[i].format == VK_FORMAT_B8G8R8A8_UNORM && availableFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormats[i];
		}
	}

	return availableFormats[0];
}

static VkPresentModeKHR chooseSwapPresentMode(VkPresentModeKHR *availablePresentModes, uint32_t availablePresentModesCount) {
	VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

	for (int i = 0; i < availablePresentModesCount; i++) {
		if (availablePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentModes[i];
		}
		else if (availablePresentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR) {
			bestMode = availablePresentModes[i];
		}
	}

	/*
	std::cout << "Present Mode..." << std::endl;
	switch (bestMode) {
	case VK_PRESENT_MODE_IMMEDIATE_KHR:
		std::cout << "\t" << "VK_PRESENT_MODE_IMMEDIATE_KHR" << std::endl;
		break;
	case VK_PRESENT_MODE_MAILBOX_KHR:
		std::cout << "\t" << "VK_PRESENT_MODE_MAILBOX_KHR" << std::endl;
		break;
	case VK_PRESENT_MODE_FIFO_KHR:
		std::cout << "\t" << "VK_PRESENT_MODE_FIFO_KHR" << std::endl;
		break;
	default:
		std::cout << "\t" << "Other" << std::endl;
		break;
	}
	*/
	return bestMode;
}
