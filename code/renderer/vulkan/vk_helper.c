#include "../tr_local.h"

void beginRenderClear()
{
	VkClearColorValue cc = { 1.0f,0.0f,0.0f,1.0f };

	VkClearValue clearValues[1];
	memset(clearValues, 0, sizeof(clearValues));
	clearValues[0].color = cc;

	VkRenderPassBeginInfo rpBeginInfo = {0};
	rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpBeginInfo.renderPass = vkInstance.swapchain.renderpass;
	rpBeginInfo.framebuffer = vkInstance.swapchain.framebuffers[vkInstance.swapchain.currentImage];
	rpBeginInfo.renderArea.extent.width = vkInstance.swapchain.extent.width;
	rpBeginInfo.renderArea.extent.height = vkInstance.swapchain.extent.height;
	rpBeginInfo.clearValueCount = 1;//m_window->sampleCountFlagBits() > VK_SAMPLE_COUNT_1_BIT ? 3 : 2;
	rpBeginInfo.pClearValues = clearValues;

	VkCommandBuffer cmdBuf = vkInstance.swapchain.commandBuffers[vkInstance.swapchain.currentImage];
	vkCmdBeginRenderPass(cmdBuf, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport = {0};
	viewport.x = viewport.y = 0;
	viewport.width = vkInstance.swapchain.extent.width;
	viewport.height = vkInstance.swapchain.extent.height;
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
	vkCmdEndRenderPass(vkInstance.swapchain.commandBuffers[vkInstance.swapchain.currentImage]);

}