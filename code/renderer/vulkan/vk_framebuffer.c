#include "../tr_local.h"

void VK_CreateFramebuffer(vkframebuffer_t* framebuffer, VkExtent2D extent, VkFormat format) {
	framebuffer->extent = extent;
	framebuffer->format = format;//vk.swapchain.imageFormat
	//setup(addDepthStencil, clearBeforeRender);

	VK_CreateImage(&framebuffer->image, extent.width, extent.height, format,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1);
	VK_CreateSampler(&framebuffer->image, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
	VK_CreateImage(&framebuffer->depth, extent.width, extent.height, VK_FORMAT_D24_UNORM_S8_UINT,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 1);

	VK_AddSampler(&framebuffer->image.descriptor_set, 0, VK_SHADER_STAGE_FRAGMENT_BIT);
	framebuffer->image.descriptor_set.data[0].descImageInfo->imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	VK_SetSampler(&framebuffer->image.descriptor_set, 0, VK_SHADER_STAGE_FRAGMENT_BIT, framebuffer->image.sampler, framebuffer->image.view);
	VK_FinishDescriptor(&framebuffer->image.descriptor_set);

	{
		VkAttachmentDescription colorAttachment = { 0 };
		colorAttachment.format = format;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

		VkAttachmentReference colorAttachmentRef = { 0 };
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depthAttachment = { 0 };
		depthAttachment.format = vk.swapchain.depthStencilFormat;
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef = { 0 };
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = { 0 };
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		if (vk.swapchain.depthImageView) subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependency[2] = { 0 };
		dependency[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency[0].dstSubpass = 0;
		dependency[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency[0].srcAccessMask = 0;
		dependency[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkAttachmentDescription attachments[2];
		attachments[0] = colorAttachment;
		attachments[1] = depthAttachment;

		VkRenderPassCreateInfo renderPassInfo = { 0 };
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = (uint32_t)(vk.swapchain.depthImageView ? 2 : 1);
		renderPassInfo.pAttachments = &attachments[0];
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		VK_CHECK(vkCreateRenderPass(vk.device, &renderPassInfo, NULL, &framebuffer->renderPass), "failed to create Renderpass");
	}

	{
		VkImageView attachments[2] = {
			framebuffer->image.view,
			framebuffer->depth.view
		};

		VkFramebufferCreateInfo framebufferInfo = {0};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = framebuffer->renderPass;
			framebufferInfo.attachmentCount = 2;
			framebufferInfo.pAttachments = &attachments;
			framebufferInfo.width = extent.width;
			framebufferInfo.height = extent.height;
			framebufferInfo.layers = 1;
		
			VkFramebuffer framebufferHandle = VK_NULL_HANDLE;
			VK_CHECK(vkCreateFramebuffer(vk.device, &framebufferInfo, NULL, &framebuffer->framebuffer), "failed to create Framebuffer");
	}
}

void VK_BeginFramebuffer(vkframebuffer_t* framebuffer) {

	VkCommandBuffer cmdBuf = vk.swapchain.CurrentCommandBuffer();

	VkClearColorValue cc = { 0.0f,1.0f,0.0f,1.0f };
	VkClearDepthStencilValue dsc = { 1, 0 };

	VkClearValue clearValues[2] = { 0 };
	clearValues[0].color = cc;
	clearValues[1].depthStencil = dsc;

	VkRenderPassBeginInfo rpBeginInfo;
	memset(&rpBeginInfo, 0, sizeof(rpBeginInfo));
	rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpBeginInfo.renderPass = framebuffer->renderPass;
	rpBeginInfo.framebuffer = framebuffer->framebuffer;
	rpBeginInfo.renderArea.extent.width = framebuffer->extent.width;
	rpBeginInfo.renderArea.extent.height = framebuffer->extent.height;
	rpBeginInfo.clearValueCount = 2;
	rpBeginInfo.pClearValues = &clearValues;
	vkCmdBeginRenderPass(cmdBuf, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);


	/*VkViewport viewport;
	viewport.x = viewport.y = 0;
	viewport.width = framebuffer->extent.width;
	viewport.height = framebuffer->extent.height;
	viewport.minDepth = 0;
	viewport.maxDepth = 1;
	vkCmdSetViewport(cmdBuf, 0, 1, &viewport);

	VkRect2D scissor;
	scissor.offset.x = scissor.offset.y = 0;
	scissor.extent.width = viewport.width;
	scissor.extent.height = viewport.height;
	vkCmdSetScissor(cmdBuf, 0, 1, &scissor);*/
}

void VK_EndFramebuffer(vkframebuffer_t* framebuffer) {
	vkCmdEndRenderPass(vk.swapchain.CurrentCommandBuffer());
}

//vulkan_framebuffer::~vulkan_framebuffer()
//{
//	//qDebug("releaseResources vulkan_framebuffer");
//
//	for (const auto& pair : this->framebuffer) {
//		vkDestroyFramebuffer(base->getDevice(), pair.second, nullptr);
//	}
//
//	//vkDestroyRenderPass(dev, defaultRenderPass, nullptr);
//	delete this->defaultRenderpass;
//
//	for (vulkan_image* img : images) {
//		delete img;
//	}
//
//	delete depthStencil;
//}
//