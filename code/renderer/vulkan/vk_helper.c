#include "../tr_local.h"

void beginRenderClear()
{
	VkClearColorValue cc = { 1.0f,0.0f,0.0f,1.0f };

	VkClearValue clearValues[1];
	memset(clearValues, 0, sizeof(clearValues));
	clearValues[0].color = cc;

	VkRenderPassBeginInfo rpBeginInfo = {0};
	rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpBeginInfo.renderPass = vk.swapchain.renderpass;
	rpBeginInfo.framebuffer = vk.swapchain.framebuffers[vk.swapchain.currentImage];
	rpBeginInfo.renderArea.extent.width = vk.swapchain.extent.width;
	rpBeginInfo.renderArea.extent.height = vk.swapchain.extent.height;
	rpBeginInfo.clearValueCount = 1;//m_window->sampleCountFlagBits() > VK_SAMPLE_COUNT_1_BIT ? 3 : 2;
	rpBeginInfo.pClearValues = clearValues;

	VkCommandBuffer cmdBuf = vk.swapchain.commandBuffers[vk.swapchain.currentImage];
	vkCmdBeginRenderPass(cmdBuf, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport = {0};
	viewport.x = viewport.y = 0;
	viewport.width = vk.swapchain.extent.width;
	viewport.height = vk.swapchain.extent.height;
	viewport.minDepth = 0;
	viewport.maxDepth = 1;
	vkCmdSetViewport(cmdBuf, 0, 1, &viewport);

	VkRect2D scissor;
	scissor.offset.x = scissor.offset.y = 0;
	scissor.extent.width = viewport.width;
	scissor.extent.height = viewport.height;
	vkCmdSetScissor(cmdBuf, 0, 1, &scissor);
}

void endRender()
{
	vkCmdEndRenderPass(vk.swapchain.commandBuffers[vk.swapchain.currentImage]);

}
