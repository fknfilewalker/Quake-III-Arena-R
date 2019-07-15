#include "../tr_local.h"

void beginRenderClear()
{
	VkClearColorValue cc = { 0.1f,0.1f,0.1f,1.0f };
    VkClearDepthStencilValue dsc = { 1, 0};

    VkClearValue clearValues[2] ={0};
	clearValues[0].color = cc;
    clearValues[1].depthStencil = dsc;

	VkRenderPassBeginInfo rpBeginInfo = {0};
	rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpBeginInfo.renderPass = vk.swapchain.renderpass;
	rpBeginInfo.framebuffer = vk.swapchain.framebuffers[vk.swapchain.currentImage];
	rpBeginInfo.renderArea.extent.width = vk.swapchain.extent.width;
	rpBeginInfo.renderArea.extent.height = vk.swapchain.extent.height;
	rpBeginInfo.clearValueCount = 2;//m_window->sampleCountFlagBits() > VK_SAMPLE_COUNT_1_BIT ? 3 : 2;
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
** CMD RECORD
*/
VkCommandBuffer VK_BeginSingleTimeCommands(VkCommandBuffer *commandBuffer) {

	VkCommandBufferAllocateInfo allocInfo = {0};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = vk.commandPool;
	allocInfo.commandBufferCount = 1;

	VK_CHECK(vkAllocateCommandBuffers(vk.device, &allocInfo, commandBuffer), "failed to allocate CMD Buffer!");

	VkCommandBufferBeginInfo beginInfo = {0};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VK_CHECK(vkBeginCommandBuffer(*commandBuffer, &beginInfo), "failed to begin CMD Buffer!");

}

void VK_EndSingleTimeCommands(VkCommandBuffer *commandBuffer) {

	VK_CHECK(vkEndCommandBuffer(*commandBuffer), "failed to end CMD Buffer!");

	VkSubmitInfo submitInfo = {0};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = commandBuffer;

	VK_CHECK(vkQueueSubmit(vk.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE), "failed to submit Queue!");
	VK_CHECK(vkQueueWaitIdle(vk.graphicsQueue), "failed to wait for Queue execution!");

	vkFreeCommandBuffers(vk.device, vk.commandPool, 1, commandBuffer);
}


/*
** MEMORY
*/
void VK_CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
	uint32_t memIndex, VkBuffer* buffer, VkDeviceMemory* bufferMemory) {


	VkBufferCreateInfo bufInfo = { 0 };
	bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufInfo.size = size;
	bufInfo.usage = usage;

	VK_CHECK(vkCreateBuffer(vk.device, &bufInfo, NULL, buffer), "failed to create Buffer!");

	VkMemoryRequirements memReq = { 0 };
	vkGetBufferMemoryRequirements(vk.device, *buffer, &memReq);
	//qDebug("allocating %u bytes for buffer", uint32_t(memReq.size));

	VkMemoryAllocateInfo memAllocInfo = {
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		NULL,
		memReq.size,
		memIndex
	};

	VK_CHECK(vkAllocateMemory(vk.device, &memAllocInfo, NULL, bufferMemory), "failed to allocate Memory!");

	VK_CHECK(vkBindBufferMemory(vk.device, *buffer, *bufferMemory, 0), "failed to bind Buffer Memory!");
}

uint32_t VK_FindMemoryTypeIndex(uint32_t memoryTypeBits, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties prop = {0};
	vkGetPhysicalDeviceMemoryProperties(vk.physical_device, &prop);

	for (int32_t i = 0; i < prop.memoryTypeCount; ++i)
	{
		if ((memoryTypeBits & (1 << i)) &&
			((prop.memoryTypes[i].propertyFlags & properties) == properties))
			return i;
	}
	return -1;
}

uint32_t VK_HostVisibleMemoryIndex()
{
	uint32_t hostVisibleMemIndex = 0;
	VkPhysicalDeviceMemoryProperties physDevMemProps = { 0 };
	qboolean hostVisibleMemIndexSet = qfalse;
	vkGetPhysicalDeviceMemoryProperties(vk.physical_device, &physDevMemProps);
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

uint32_t VK_DeviceLocalMemoryIndex()
{
	uint32_t deviceLocalMemIndex = 0;
	VkPhysicalDeviceMemoryProperties physDevMemProps = { 0 };
	vkGetPhysicalDeviceMemoryProperties(vk.physical_device, &physDevMemProps);
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
