#include "../tr_local.h"

static void VK_BufferMemoryBarrier(VkCommandBuffer cb, VkBuffer* buffer,
	VkPipelineStageFlags srcStages, VkPipelineStageFlags dstStages,
	VkAccessFlags srcAccess, VkAccessFlags dstAccess);

/*
 ** INFO
 */
void VK_GetDeviceProperties(VkPhysicalDeviceProperties *devProperties)
{
    vkGetPhysicalDeviceProperties(vk.physicalDevice, devProperties);
}

void VK_BeginRenderClear()
{
	if (vk_d.imageDescriptor.needsUpdate == qtrue) {
		VK_UpdateDescriptorSet(&vk_d.imageDescriptor);
		vk_d.imageDescriptor.needsUpdate = qfalse;
	}

	// buffer offset so each cmd buffer has its own range
	vk_d.offsetIdx = vk.swapchain.currentImage * VK_INDEX_DATA_SIZE;
	vk_d.offset = vk.swapchain.currentImage * VK_VERTEX_ATTRIBUTE_DATA_SIZE;
	vk_d.currentPipeline = -1;

	VkClearColorValue cc = { 0.1f,0.1f,0.1f,1.0f };
    VkClearDepthStencilValue dsc = { 1, 0};


    VkClearValue clearValues[2] ={0};
	clearValues[0].color = cc;
    clearValues[1].depthStencil = dsc;

	VkRenderPassBeginInfo rpBeginInfo = {0};
	rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpBeginInfo.renderPass = vk.swapchain.renderpass;
	rpBeginInfo.framebuffer = vk.swapchain.CurrentFramebuffer();
	rpBeginInfo.renderArea.extent.width = vk.swapchain.extent.width;
	rpBeginInfo.renderArea.extent.height = vk.swapchain.extent.height;
	rpBeginInfo.clearValueCount = 2;//m_window->sampleCountFlagBits() > VK_SAMPLE_COUNT_1_BIT ? 3 : 2;
	rpBeginInfo.pClearValues = &clearValues;

	VkCommandBuffer cmdBuf = vk.swapchain.CurrentCommandBuffer();

	
	
	vkCmdBeginRenderPass(cmdBuf, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    VK_BindIndexBuffer(&vk_d.indexbuffer, 0);
    VK_BindAttribBuffer(&vk_d.vertexbuffer, 0, 0);
    VK_BindAttribBuffer(&vk_d.colorbuffer, 1, 0);
    VK_BindAttribBuffer(&vk_d.uvbuffer1, 2, 0);
    VK_BindAttribBuffer(&vk_d.uvbuffer2, 3, 0);
}

void beginRender()
{
   
}

void VK_EndRender()
{
	vkCmdEndRenderPass(vk.swapchain.CurrentCommandBuffer());
	
	//if(vk_d.drawMirror == qfalse) VK_CopyImageToSwapchain(&vk_d.accelerationStructures.resultImage);

}

static float vFullscreenQuad[24] = { -1.0f, -1.0f, 1.0f, 0.0f,
											 1.0f, -1.0f, 1.0f, 0.0f,
											-1.0f,  1.0f, 1.0f, 0.0f,
											 1.0f,  1.0f, 1.0f, 0.0f };
static uint32_t idxFullscreenQuad[6] = { 0, 1, 2, 2, 1, 3 };
static float uvFullscreenQuad[12] = { 0.0f, 0.0f,
									1.0f, 0.0f,
									0.0f,  1.0f,
									1.0f,  1.0f};

/*
** CLEAR ATTACHMENT
*/
void VK_ClearAttachments(qboolean clear_depth, qboolean clear_stencil, qboolean clear_color, vec4_t color) {
    

    if (!clear_depth && !clear_stencil && !clear_color)
        return;
    
    if (clear_depth) clear_depth = 1;
    if (clear_stencil) clear_stencil = 1;
    if (clear_color) clear_color = 1;

    // backup state
    VkViewport vpSave = { 0 };
    VkRect2D sSave = { 0 };
    Com_Memcpy(&vpSave, &vk_d.viewport, sizeof(vk_d.viewport));
    Com_Memcpy(&sSave, &vk_d.scissor, sizeof(vk_d.scissor));

    memset(&vk_d.viewport, 0, sizeof(vk_d.viewport));
    memset(&vk_d.scissor, 0, sizeof(vk_d.scissor));
    vk_d.viewport.x = 0;
    vk_d.viewport.y = 0;
    vk_d.viewport.height = glConfig.vidHeight;
    vk_d.viewport.width = glConfig.vidWidth;
    vk_d.viewport.minDepth = 0.0f;
    vk_d.viewport.maxDepth = 1.0f;
    vk_d.scissor.offset.x = 0;
    vk_d.scissor.offset.y = 0;
    vk_d.scissor.extent.height = glConfig.vidHeight;
    vk_d.scissor.extent.width = glConfig.vidWidth;

    VK_UploadBufferDataOffset(&vk_d.indexbuffer, vk_d.offsetIdx * sizeof(uint32_t), sizeof(idxFullscreenQuad)/sizeof(idxFullscreenQuad[0]) * sizeof(uint32_t), (void*) &idxFullscreenQuad[0]);
    VK_UploadBufferDataOffset(&vk_d.vertexbuffer, vk_d.offset * sizeof(vec4_t), sizeof(vFullscreenQuad)/sizeof(vFullscreenQuad[0]) * sizeof(vec4_t), (void*)&vFullscreenQuad[0]);
    
    vkpipeline_t p = { 0 };
    VK_GetAttachmentClearPipelines(&p, clear_color, clear_depth, clear_stencil);

    VK_SetPushConstant(&p, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(vec4_t), &color);
	VK_BindPipeline(&p);
    VK_DrawIndexed(&vk_d.indexbuffer, sizeof(idxFullscreenQuad)/sizeof(idxFullscreenQuad[0]), vk_d.offsetIdx, vk_d.offset);

    vk_d.offsetIdx += sizeof(idxFullscreenQuad)/sizeof(idxFullscreenQuad[0]);
    vk_d.offset += sizeof(vFullscreenQuad)/sizeof(vFullscreenQuad[0]);
    
    // restore state
    Com_Memcpy(&vk_d.viewport, &vpSave, sizeof(vk_d.viewport));
    Com_Memcpy(&vk_d.scissor, &sSave, sizeof(vk_d.scissor));
}

/*
** FULLSCREEN RECT
*/
void VK_DrawFullscreenRect(vkimage_t *image) {
	if (vk_d.fullscreenRectPipeline.handle == VK_NULL_HANDLE) {
	
		vkshader_t s = { 0 };
		VK_FullscreenRectShader(&s);
		
		VK_SetDescriptorSet(&vk_d.fullscreenRectPipeline, &vk_d.images[0].descriptor_set);
		VK_SetShader(&vk_d.fullscreenRectPipeline, &s);
		VK_AddBindingDescription(&vk_d.fullscreenRectPipeline, 0, sizeof(vec4_t), VK_VERTEX_INPUT_RATE_VERTEX);
		VK_AddBindingDescription(&vk_d.fullscreenRectPipeline, 2, sizeof(vec2_t), VK_VERTEX_INPUT_RATE_VERTEX);
		VK_AddAttributeDescription(&vk_d.fullscreenRectPipeline, 0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 0 * sizeof(float));
		VK_AddAttributeDescription(&vk_d.fullscreenRectPipeline, 2, 2, VK_FORMAT_R32G32_SFLOAT, 0 * sizeof(float));
		
		VK_FinishPipeline(&vk_d.fullscreenRectPipeline);

	}

	// backup state
	VkViewport vpSave = { 0 };
	VkRect2D sSave = { 0 };
	Com_Memcpy(&vpSave, &vk_d.viewport, sizeof(vk_d.viewport));
	Com_Memcpy(&sSave, &vk_d.scissor, sizeof(vk_d.scissor));

	VK_UploadBufferDataOffset(&vk_d.indexbuffer, vk_d.offsetIdx * sizeof(uint32_t), sizeof(idxFullscreenQuad) / sizeof(idxFullscreenQuad[0]) * sizeof(uint32_t), (void*)& idxFullscreenQuad[0]);
	VK_UploadBufferDataOffset(&vk_d.vertexbuffer, vk_d.offset * sizeof(vec4_t), sizeof(vFullscreenQuad) / sizeof(vFullscreenQuad[0]) * sizeof(vec4_t), (void*)& vFullscreenQuad[0]);
	VK_UploadBufferDataOffset(&vk_d.uvbuffer1, vk_d.offset * sizeof(vec2_t), sizeof(vFullscreenQuad) / sizeof(vFullscreenQuad[0]) * sizeof(vec2_t), (void*)& uvFullscreenQuad[0]);

	VK_Bind1DescriptorSet(&vk_d.fullscreenRectPipeline, &image->descriptor_set);
	VK_BindPipeline(&vk_d.fullscreenRectPipeline);
	VK_DrawIndexed(&vk_d.indexbuffer, sizeof(idxFullscreenQuad) / sizeof(idxFullscreenQuad[0]), vk_d.offsetIdx, vk_d.offset);

	vk_d.offsetIdx += sizeof(idxFullscreenQuad) / sizeof(idxFullscreenQuad[0]);
	vk_d.offset += sizeof(vFullscreenQuad) / sizeof(vFullscreenQuad[0]);

	// restore state
	Com_Memcpy(&vk_d.viewport, &vpSave, sizeof(vk_d.viewport));
	Com_Memcpy(&vk_d.scissor, &sSave, sizeof(vk_d.scissor));
}

/*
** PIPELINE LIST
*/
qboolean VK_FindPipeline() {
	for (uint32_t i = 0; i < vk_d.pipelineListSize; ++i) {
		vkrenderState_t* state = &vk_d.pipelineList[i].state;
		vkpipeline_t* found_p = &vk_d.pipelineList[i].pipeline;
		if (memcmp(&vk_d.state, state, sizeof(vkrenderState_t)) == 0) {
			return i;
		}
	}
	return -1;
}
uint32_t VK_AddPipeline(vkpipeline_t* p) {
	if (vk_d.pipelineListSize >= VK_MAX_NUM_PIPELINES) {
		ri.Error(ERR_FATAL, "Vulkan: Maximum amount of pipelines reached!");
	}
	vk_d.pipelineList[vk_d.pipelineListSize] = (vkpipe_t){ *p, vk_d.state };
	vk_d.pipelineListSize++;
	return vk_d.pipelineListSize - 1;
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
void VK_CreateBufferMemory(VkDeviceSize size, VkBufferUsageFlags usage,
	VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory) {

	VkBufferCreateInfo bufInfo = { 0 };
	bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
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
		VK_FindMemoryTypeIndex(memReq.memoryTypeBits, properties)
	};

	VK_CHECK(vkAllocateMemory(vk.device, &memAllocInfo, NULL, bufferMemory), "failed to allocate Memory!");
	VK_CHECK(vkBindBufferMemory(vk.device, *buffer, *bufferMemory, 0), "failed to bind Buffer Memory!");
}

void VK_CreateImageMemory(VkMemoryPropertyFlags properties, VkImage* image, VkDeviceMemory* bufferMemory) {
	VkMemoryRequirements memRequirements = { 0 };
	vkGetImageMemoryRequirements(vk.device, *image, &memRequirements);

	int32_t memoryTypeIndex = VK_FindMemoryTypeIndex(memRequirements.memoryTypeBits, properties);

	VkMemoryAllocateInfo allocInfo = { 0 };
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = memoryTypeIndex != -1 ? memoryTypeIndex : VK_DeviceLocalMemoryIndex();

	VK_CHECK(vkAllocateMemory(vk.device, &allocInfo, NULL, bufferMemory), "failed to allocate Image Memory!");
	VK_CHECK(vkBindImageMemory(vk.device, *image, *bufferMemory, 0), "failed to bind Image Memory!");
}

static void VK_BufferMemoryBarrier(VkCommandBuffer cb, VkBuffer* buffer,
	VkPipelineStageFlags srcStages, VkPipelineStageFlags dstStages,
	VkAccessFlags srcAccess, VkAccessFlags dstAccess) {

	VkBufferMemoryBarrier barrier;
	barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	barrier.pNext = NULL;
	barrier.srcAccessMask = srcAccess;
	barrier.dstAccessMask = dstAccess;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.buffer = *buffer;
	barrier.offset = 0;
	barrier.size = VK_WHOLE_SIZE;

	vkCmdPipelineBarrier(cb, srcStages, dstStages, 0, 0, NULL, 1, &barrier, 0, NULL);
}

uint32_t VK_FindMemoryTypeIndex(uint32_t memoryTypeBits, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties prop = { 0 };
	vkGetPhysicalDeviceMemoryProperties(vk.physicalDevice, &prop);

	for (int32_t i = 0; i < prop.memoryTypeCount; ++i)
	{
		if ( (memoryTypeBits & (1 << i)) &&
			((prop.memoryTypes[i].propertyFlags & properties) == properties) )
			return i;
	}
	return -1;
}

uint32_t VK_HostVisibleMemoryIndex()
{
	uint32_t hostVisibleMemIndex = -1;
	VkPhysicalDeviceMemoryProperties physDevMemProps = { 0 };
	vkGetPhysicalDeviceMemoryProperties(vk.physicalDevice, &physDevMemProps);

	qboolean hostVisibleMemIndexSet = qfalse;
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
	uint32_t deviceLocalMemIndex = -1;
	VkPhysicalDeviceMemoryProperties physDevMemProps = { 0 };
	vkGetPhysicalDeviceMemoryProperties(vk.physicalDevice, &physDevMemProps);
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

/*
* RTX MEMORY
*/
void VK_GetAccelerationStructureMemoryRequirements(VkAccelerationStructureNV as, VkAccelerationStructureMemoryRequirementsTypeNV type, VkMemoryRequirements2 *memreq){
	VkAccelerationStructureMemoryRequirementsInfoNV memoryRequirementsInfoScratch = { 0 };
	memoryRequirementsInfoScratch.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
	memoryRequirementsInfoScratch.accelerationStructure = as;
	memoryRequirementsInfoScratch.type = type;
	memreq->sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
	vkGetAccelerationStructureMemoryRequirementsNV(vk.device, &memoryRequirementsInfoScratch, memreq);
}

/*
* PERFORMANCE
*/

void VK_SetPerformanceMarker(VkCommandBuffer command_buffer, int index) {
	vkCmdWriteTimestamp(command_buffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		vk.queryPool, (vk.swapchain.currentImage * PROFILER_IN_FLIGHT) + index);
}

void VK_ResetQueryPool(VkCommandBuffer command_buffer) {
	vkCmdResetQueryPool(command_buffer, vk.queryPool,
		PROFILER_IN_FLIGHT * vk.swapchain.currentImage,
		PROFILER_IN_FLIGHT);
}

int VK_QueryPoolResults() {
	VkResult result = vkGetQueryPoolResults(vk.device, vk.queryPool,
		PROFILER_IN_FLIGHT * vk.swapchain.currentImage,
		PROFILER_IN_FLIGHT,
		sizeof(vk_d.queryPoolResults[0]),
		&(vk_d.queryPoolResults[vk.swapchain.currentImage][0]),
		sizeof(vk_d.queryPoolResults[0][0]),
		VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WITH_AVAILABILITY_BIT);
	int result2 = 2;
	return result2;
}
void VK_TimeFromMarkers(double* ms, int start, int end) {
	*ms = (double)(vk_d.queryPoolResults[vk.swapchain.currentImage][end] - vk_d.queryPoolResults[vk.swapchain.currentImage][start]) * 1e-6;
}
