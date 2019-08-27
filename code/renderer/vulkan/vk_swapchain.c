#include "../tr_local.h"
#include <stdlib.h>

#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))


/*
 ==============================================================================

 SwapChain Setup

 ==============================================================================
 */

 // Function Declaration
static void VK_CreateSwapChain();
static void VK_CreateImageViews();
static void VK_CreateDepthStencil();
static void VK_CreateRenderPass();
static void VK_CreateFramebuffers();
static void VK_CreateCommandBuffers();
static void VK_CreateSyncObjects();

// Helper
static VkSurfaceFormatKHR chooseSwapSurfaceFormat(VkSurfaceFormatKHR* availableFormats, uint32_t availableFormatsCount);
static VkPresentModeKHR chooseSwapPresentMode(VkPresentModeKHR* availablePresentModes, uint32_t availablePresentModesCount);

static VkFramebuffer VK_CurrentFramebuffer();
static VkCommandBuffer VK_CurrentCommandBuffer();

void VK_SetupSwapchain() 
{
	VK_CreateSwapChain();
	VK_CreateImageViews();
	VK_CreateDepthStencil();
	VK_CreateRenderPass();
	VK_CreateFramebuffers();

	VK_CreateCommandBuffers();
	VK_CreateSyncObjects();

	vk.swapchain.CurrentCommandBuffer = VK_CurrentCommandBuffer;
	vk.swapchain.CurrentFramebuffer = VK_CurrentFramebuffer;
}

void VK_DestroySwapchain() {
	vkWaitForFences(vk.device, vk.swapchain.imageCount, vk.swapchain.inFlightFences, VK_TRUE, 10);

	for (int i = 0; i < vk.swapchain.imageCount; ++i) {
		vkDestroyFramebuffer(vk.device, vk.swapchain.framebuffers[i], NULL);
	}
	free(vk.swapchain.framebuffers);

	for (int i = 0; i < vk.swapchain.imageCount; ++i) {
		vkDestroyImageView(vk.device, vk.swapchain.imageViews[i], NULL);
	}
	free(vk.swapchain.imageViews);

	/*for (int i = 0; i < vk.swapchain.imageCount; ++i) {
		vkDestroyImage(vk.device, vk.swapchain.images[i], NULL);
	}
	*/

	vkDestroyRenderPass(vk.device, vk.swapchain.renderpass, NULL);

	vkDestroyImageView(vk.device, vk.swapchain.depthImageView, NULL);
	vkDestroyImage(vk.device, vk.swapchain.depthImage, NULL);
	vkFreeMemory(vk.device, vk.swapchain.depthImageMemory, NULL);

	vkDestroySwapchainKHR(vk.device, vk.swapchain.handle, NULL);
	free(vk.swapchain.images);

	vkFreeCommandBuffers(vk.device, vk.commandPool, vk.swapchain.imageCount, vk.swapchain.commandBuffers);
	free(vk.swapchain.commandBuffers);

	for (int i = 0; i < vk.swapchain.imageCount; ++i) {
		vkDestroySemaphore(vk.device, vk.swapchain.imageAvailableSemaphores[i], NULL);
		vkDestroySemaphore(vk.device, vk.swapchain.renderFinishedSemaphores[i], NULL);
		vkDestroyFence(vk.device, vk.swapchain.inFlightFences[i], NULL);
	}
	free(vk.swapchain.imageAvailableSemaphores);
	free(vk.swapchain.renderFinishedSemaphores);
	free(vk.swapchain.inFlightFences);

	memset(&vk.swapchain, 0, sizeof(vk.swapchain));
}

static void VK_CreateSwapChain() {
	vk.swapchain.depthStencilFormat = VK_FORMAT_D24_UNORM_S8_UINT;

	swapChainSupportDetails_t swapChainSupport = querySwapChainSupport(vk.physicalDevice, vk.surface);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(&swapChainSupport.formats[0], swapChainSupport.formatCount);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(&swapChainSupport.presentModes[0], swapChainSupport.presentModeCount);
	
	// set extent
	vk.swapchain.extent = swapChainSupport.capabilities.currentExtent;
	vk.swapchain.extent.width = max(swapChainSupport.capabilities.minImageExtent.width, min(swapChainSupport.capabilities.maxImageExtent.width, vk.swapchain.extent.width));
	vk.swapchain.extent.height = max(swapChainSupport.capabilities.minImageExtent.height, min(swapChainSupport.capabilities.maxImageExtent.height, vk.swapchain.extent.height));

	vk.swapchain.imageFormat = surfaceFormat.format;

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {0};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = vk.surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = vk.swapchain.extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	createInfo.clipped = VK_TRUE;

	uint32_t queueFamilyIndices[] = {	vk.queryFamilyIndices.graphicsFamily, 
										vk.queryFamilyIndices.presentFamily };

	if (vk.queryFamilyIndices.graphicsFamily != vk.queryFamilyIndices.presentFamily) {
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

	VK_CHECK(vkCreateSwapchainKHR(vk.device, &createInfo, NULL, &vk.swapchain.handle), "failed to create Swapchain!");

	vkGetSwapchainImagesKHR(vk.device, vk.swapchain.handle, &vk.swapchain.imageCount, NULL);
	vk.swapchain.images = malloc(vk.swapchain.imageCount * sizeof(VkImage));
	vkGetSwapchainImagesKHR(vk.device, vk.swapchain.handle, &imageCount, &vk.swapchain.images[0]);

}

static void VK_CreateImageViews() {
	vk.swapchain.imageViews = malloc(vk.swapchain.imageCount * sizeof(VkImageView));

	for (size_t i = 0; i < vk.swapchain.imageCount; i++) {
		VkImageViewCreateInfo createInfo = { 0 };
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = vk.swapchain.images[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = vk.swapchain.imageFormat;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		VK_CHECK(vkCreateImageView(vk.device, &createInfo, NULL, &vk.swapchain.imageViews[i]), "failed to create ImageView for Swapchain!");
	}
}

static void VK_CreateDepthStencil()
{
	// Buffer
	VkImageCreateInfo imageInfo = {0};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = vk.swapchain.extent.width;
	imageInfo.extent.height = vk.swapchain.extent.height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = vk.swapchain.depthStencilFormat;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VK_CHECK(vkCreateImage(vk.device, &imageInfo, NULL, &vk.swapchain.depthImage), "failed to create DepthStencil Image for Swapchain!");	
	VK_CreateImageMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &vk.swapchain.depthImage, &vk.swapchain.depthImageMemory);

	// Image View
	VkImageViewCreateInfo viewInfo = {0};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = vk.swapchain.depthImage;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = vk.swapchain.depthStencilFormat;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VK_CHECK(vkCreateImageView(vk.device, &viewInfo, NULL, &vk.swapchain.depthImageView), "failed to create DepthStencil image view for Swapchain!");
	
}

static void VK_CreateRenderPass()
{
	VkAttachmentDescription colorAttachment = {0};
	colorAttachment.format = vk.swapchain.imageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {0};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment = {0};
	depthAttachment.format = vk.swapchain.depthStencilFormat;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
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
	if (vk.swapchain.depthImageView) subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency dependency = {0};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkAttachmentDescription attachments[2];
	attachments[0] = colorAttachment;
	attachments[1] = depthAttachment;

	VkRenderPassCreateInfo renderPassInfo = {0};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = (uint32_t) (vk.swapchain.depthImageView ? 2 : 1);
	renderPassInfo.pAttachments = &attachments[0];
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	VK_CHECK(vkCreateRenderPass(vk.device, &renderPassInfo, NULL, &vk.swapchain.renderpass), "failed to create RenderPass for Swapchain!");
}

static void VK_CreateFramebuffers()
{
	vk.swapchain.framebuffers = malloc(vk.swapchain.imageCount * sizeof(VkFramebuffer));

	for (size_t i = 0; i < vk.swapchain.imageCount; i++) {
		VkImageView attachments[2];
		attachments[0] = vk.swapchain.imageViews[i];
		attachments[1] = vk.swapchain.depthImageView;

		VkFramebufferCreateInfo framebufferInfo = {0};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = vk.swapchain.renderpass;
		framebufferInfo.attachmentCount = (uint32_t) (vk.swapchain.depthImageView ? 2 : 1);
		framebufferInfo.pAttachments = &attachments[0];
		framebufferInfo.width = vk.swapchain.extent.width;
		framebufferInfo.height = vk.swapchain.extent.height;
		framebufferInfo.layers = 1;

		VK_CHECK(vkCreateFramebuffer(vk.device, &framebufferInfo, NULL, &vk.swapchain.framebuffers[i]), "failed to create Framebuffer for Swapchain!");
	}
}

static void VK_CreateCommandBuffers()
{
	vk.swapchain.commandBuffers = malloc(vk.swapchain.imageCount * sizeof(VkCommandBuffer));

	VkCommandBufferAllocateInfo allocInfo = { 0 };
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = vk.commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = vk.swapchain.imageCount;

	VK_CHECK(vkAllocateCommandBuffers(vk.device, &allocInfo, vk.swapchain.commandBuffers), "failed to allocate command buffers!");
}

static void VK_CreateSyncObjects()
{
	vk.swapchain.imageAvailableSemaphores = malloc(vk.swapchain.imageCount * sizeof(VkSemaphore));
	vk.swapchain.renderFinishedSemaphores = malloc(vk.swapchain.imageCount * sizeof(VkSemaphore));
	vk.swapchain.inFlightFences = malloc(vk.swapchain.imageCount * sizeof(VkFence));

	VkSemaphoreCreateInfo semaphoreInfo = { 0 };
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = { 0 };
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < vk.swapchain.imageCount; i++) {
		VK_CHECK(vkCreateSemaphore(vk.device, &semaphoreInfo, NULL, &vk.swapchain.imageAvailableSemaphores[i]), "failed to create Semaphore!");
		VK_CHECK(vkCreateSemaphore(vk.device, &semaphoreInfo, NULL, &vk.swapchain.renderFinishedSemaphores[i]), "failed to create Semaphore!");
		VK_CHECK(vkCreateFence(vk.device, &fenceInfo, NULL, &vk.swapchain.inFlightFences[i]), "failed to create Fence!");
	}

}

void record_buffer_memory_barrier(VkCommandBuffer cb, VkBuffer buffer,
	VkPipelineStageFlags src_stages, VkPipelineStageFlags dst_stages,
	VkAccessFlags src_access, VkAccessFlags dst_access) {

	VkBufferMemoryBarrier barrier;
	barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;;
	barrier.pNext = NULL;
	barrier.srcAccessMask = src_access;
	barrier.dstAccessMask = dst_access;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.buffer = buffer;
	barrier.offset = 0;
	barrier.size = VK_WHOLE_SIZE;

	vkCmdPipelineBarrier(cb, src_stages, dst_stages, 0, 0, NULL, 1, &barrier, 0, NULL);
}

void VK_BeginFrame()
{
	if (vk.swapchain.frameStarted) return;
	vk.swapchain.frameStarted = qtrue;

    // wait for command buffer submission for last imageclock_t start = clock();
    
    
	vkAcquireNextImageKHR(vk.device, vk.swapchain.handle, UINT64_MAX, vk.swapchain.imageAvailableSemaphores[vk.swapchain.currentFrame], VK_NULL_HANDLE, &vk.swapchain.currentImage);

    //Com_Printf("fence %d %d %d\n", vkGetFenceStatus(vk.device, vk.swapchain.inFlightFences[0]) == VK_SUCCESS,
    //           vkGetFenceStatus(vk.device, vk.swapchain.inFlightFences[1]) == VK_SUCCESS,
    //           vkGetFenceStatus(vk.device, vk.swapchain.inFlightFences[2]) == VK_SUCCESS);
    clock_t start = clock();
    vkWaitForFences(vk.device, 1, &vk.swapchain.inFlightFences[vk.swapchain.currentImage], VK_TRUE, UINT64_MAX);
    vkResetFences(vk.device, 1, &vk.swapchain.inFlightFences[vk.swapchain.currentImage]);
    clock_t end = clock();
    float seconds = (float)(end - start) / CLOCKS_PER_SEC;
    //Com_Printf("new pipe %f\n", seconds);
    
    
    
//    vkFreeCommandBuffers(vk.device, vk.commandPool, 1, &vk.swapchain.commandBuffers[vk.swapchain.currentImage]);
//    VkCommandBufferAllocateInfo cmdBufInfo = {
//        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, NULL, vk.commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1 };
//    VkResult err = vkAllocateCommandBuffers(vk.device, &cmdBufInfo, &vk.swapchain.commandBuffers[vk.swapchain.currentImage]);

	VkCommandBufferBeginInfo beginInfo = { 0 };
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;//VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	VK_CHECK(vkBeginCommandBuffer(vk.swapchain.commandBuffers[vk.swapchain.currentImage], &beginInfo), "failed to begin recording command buffer!");

	// Ensure visibility of geometry buffers writes.
//    record_buffer_memory_barrier(vk.swapchain.commandBuffers[vk.swapchain.currentImage], vk_d.vertexbuffer.buffer,
//        VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
//        VK_ACCESS_HOST_WRITE_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
//
//    record_buffer_memory_barrier(vk.swapchain.commandBuffers[vk.swapchain.currentImage], vk_d.indexbuffer.buffer,
//        VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
//        VK_ACCESS_HOST_WRITE_BIT, VK_ACCESS_INDEX_READ_BIT);
}

void VK_EndFrame()
{
	if (!vk.swapchain.frameStarted) return;
	vk.swapchain.frameStarted = qfalse;

	VK_CHECK(vkEndCommandBuffer(vk.swapchain.commandBuffers[vk.swapchain.currentImage]), "failed to end commandbuffer!");
	//vkResetFences(this->device, 1, &inFlightFences[currentFrame]);

	VkSemaphore waitSemaphores[] = { vk.swapchain.imageAvailableSemaphores[vk.swapchain.currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSemaphore signalSemaphores[] = { vk.swapchain.renderFinishedSemaphores[vk.swapchain.currentFrame] };

	VkSubmitInfo submitInfo = { 0 };
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &vk.swapchain.commandBuffers[vk.swapchain.currentImage];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	VK_CHECK(vkQueueSubmit(vk.graphicsQueue, 1, &submitInfo, vk.swapchain.inFlightFences[vk.swapchain.currentImage]), "failed to submit draw command buffer!");


	VkSwapchainKHR swapChains[] = { vk.swapchain.handle };

	VkPresentInfoKHR presentInfo = { 0 };
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &vk.swapchain.currentImage;

	VK_CHECK(vkQueuePresentKHR(vk.presentQueue, &presentInfo), "failed to Queue Present!");

	vk.swapchain.currentFrame = (vk.swapchain.currentFrame + 1) % vk.swapchain.imageCount;
}

/*
==============================================================================

SwapChain Helper Function

==============================================================================
*/

static VkFramebuffer VK_CurrentFramebuffer()
{
	return vk.swapchain.framebuffers[vk.swapchain.currentImage];
}

static VkCommandBuffer VK_CurrentCommandBuffer()
{
	return vk.swapchain.commandBuffers[vk.swapchain.currentImage];
}

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
