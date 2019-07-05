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

/*
** MEMORY
*/
uint32_t VK_FindMemoryTypeIndex(VkPhysicalDevice physicalDevice, uint32_t memoryTypeBits, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties prop = {0};
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &prop);

	for (int32_t i = 0; i < prop.memoryTypeCount; ++i)
	{
		if ((memoryTypeBits & (1 << i)) &&
			((prop.memoryTypes[i].propertyFlags & properties) == properties))
			return i;
	}
	return -1;
}

uint32_t VK_HostVisibleMemoryIndex(VkPhysicalDevice physicalDevice)
{
	uint32_t hostVisibleMemIndex = 0;
	VkPhysicalDeviceMemoryProperties physDevMemProps = { 0 };
	qboolean hostVisibleMemIndexSet = qfalse;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physDevMemProps);
	for (uint32_t i = 0; i < physDevMemProps.memoryTypeCount; ++i) {
		const VkMemoryType* memType = physDevMemProps.memoryTypes;
		// Find a host visible, host coherent memtype. If there is one that is
		// cached as well (in addition to being coherent), prefer that.
		const int hostVisibleAndCoherent = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		if ((memType[i].propertyFlags & hostVisibleAndCoherent) == hostVisibleAndCoherent) {
			if (!hostVisibleMemIndexSet
				|| (memType[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT)) {
				hostVisibleMemIndexSet = qtrue;
				hostVisibleMemIndex = i;
			}
		}
	}
	return hostVisibleMemIndex;
}

uint32_t VK_DeviceLocalMemoryIndex(VkPhysicalDevice physicalDevice)
{
	uint32_t deviceLocalMemIndex = 0;
	VkPhysicalDeviceMemoryProperties physDevMemProps = { 0 };
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physDevMemProps);
	for (uint32_t i = 0; i < physDevMemProps.memoryTypeCount; ++i) {
		const VkMemoryType* memType = physDevMemProps.memoryTypes;
		// Just pick the first device local memtype.
		if (memType[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
			deviceLocalMemIndex = i;
			break;
		}
	}
	return deviceLocalMemIndex;
}