#include "../tr_local.h"

/*
 ** INFO
 */
void VK_GetDeviceProperties(VkPhysicalDeviceProperties *devProperties)
{
    vkGetPhysicalDeviceProperties(vk.physical_device, devProperties);
}

void beginRenderClear()
{
	vk_d.offsetIdx = 0;
	vk_d.offset = 0;

	VkClearColorValue cc = { 0.1f,0.1f,0.1f,1.0f };
    VkClearDepthStencilValue dsc = { 1, 0};

    VkClearValue clearValues[2] ={0};
	//clearValues[0].color = cc;
    clearValues[1].depthStencil = dsc;

	VkRenderPassBeginInfo rpBeginInfo = {0};
	rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpBeginInfo.renderPass = vk.swapchain.renderpass;
	rpBeginInfo.framebuffer = vk.swapchain.CurrentFramebuffer();
	rpBeginInfo.renderArea.extent.width = vk.swapchain.extent.width;
	rpBeginInfo.renderArea.extent.height = vk.swapchain.extent.height;
	rpBeginInfo.clearValueCount = 2;//m_window->sampleCountFlagBits() > VK_SAMPLE_COUNT_1_BIT ? 3 : 2;
	rpBeginInfo.pClearValues = clearValues;

	VkCommandBuffer cmdBuf = vk.swapchain.CurrentCommandBuffer();
	vkCmdBeginRenderPass(cmdBuf, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

//    VkViewport viewport = {0};
//    viewport.x = viewport.y = 0;
//    viewport.width = vk.swapchain.extent.width;
//    viewport.height = vk.swapchain.extent.height;
//    viewport.minDepth = 0;
//    viewport.maxDepth = 1;
	//vkCmdSetViewport(cmdBuf, 0, 1, &vk_d.viewport);

//    VkRect2D scissor;
//    scissor.offset.x = scissor.offset.y = 0;
//    scissor.extent.width = viewport.width;
//    scissor.extent.height = viewport.height;
	//vkCmdSetScissor(cmdBuf, 0, 1, &vk_d.scissor);
    
    /*if ( vk_d.polygonOffset )
    {
        vkCmdSetDepthBias(cmdBuf, r_offsetUnits->value, 0.0f, r_offsetFactor->value);
    }*/
    
    float constRGBA[4] = {0, 0, 0, 0};
    vkCmdSetBlendConstants(cmdBuf, &constRGBA);
    
    VK_BindIndexBuffer(&vk_d.indexbuffer, 0);
    VK_BindAttribBuffer(&vk_d.vertexbuffer, 0, 0);
    VK_BindAttribBuffer(&vk_d.colorbuffer, 1, 0);
    VK_BindAttribBuffer(&vk_d.uvbuffer1, 2, 0);
    VK_BindAttribBuffer(&vk_d.uvbuffer2, 3, 0);
}

void beginRender()
{
   
}

void endRender()
{
	vkCmdEndRenderPass(vk.swapchain.CurrentCommandBuffer());

}

/*
** CLEAR ATTACHMENT
*/

void VK_ClearAttachments(qboolean clear_depth, qboolean clear_stencil, qboolean clear_color, vec4_t color) {
    static float vFullscreenQuad[24] = {    -1.0f, -1.0f, 1.0f, 0.0f,
                                             1.0f, -1.0f, 1.0f, 0.0f,
                                            -1.0f,  1.0f, 1.0f, 0.0f,
                                             1.0f,  1.0f, 1.0f, 0.0f};
    static uint32_t idxFullscreenQuad[6] = {0, 1, 2, 2, 1, 3};
    //return;
    if (!clear_depth && !clear_stencil && !clear_color)
        return;
    
	if (clear_depth) clear_depth = 1;
	if (clear_stencil) clear_stencil = 1;
	if (clear_color) clear_color = 1;

	VkViewport vpSave = { 0 };
	VkRect2D sSave = { 0 };

	Com_Memcpy(&vpSave, &vk_d.viewport, sizeof(vk_d.viewport));
	Com_Memcpy(&sSave, &vk_d.scissor, sizeof(vk_d.scissor));


	qboolean set[2] = { clear_color, clear_depth };

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

	/*vk_d.state.cullMode = VK_CULL_MODE_NONE;

	memset(&vk_d.state.dsBlend, 0, sizeof(vk_d.state.dsBlend));
	memset(&vk_d.state.colorBlend, 0, sizeof(vk_d.state.colorBlend));

	if (clear_depth) {
		vk_d.state.dsBlend.depthTestEnable = VK_TRUE;
		vk_d.state.dsBlend.depthWriteEnable = VK_TRUE;
		vk_d.state.dsBlend.depthCompareOp = VK_COMPARE_OP_ALWAYS;
	}
	else {
		vk_d.state.dsBlend.depthTestEnable = VK_FALSE;
		vk_d.state.dsBlend.depthWriteEnable = VK_FALSE;
	}

	vk_d.state.dsBlend.stencilTestEnable = VK_FALSE;
	if (clear_stencil) {
		vk_d.state.dsBlend.stencilTestEnable = VK_TRUE;
		VkStencilOpState stencilSettings = { 0 };
		stencilSettings.failOp = VK_STENCIL_OP_ZERO;
		stencilSettings.passOp = VK_STENCIL_OP_ZERO;
		stencilSettings.compareOp = VK_COMPARE_OP_ALWAYS;
		stencilSettings.writeMask = 0x00;

		vk_d.state.dsBlend.back = stencilSettings;
		vk_d.state.dsBlend.c = stencilSettings;

	}
	else {

	}

	
	if (clear_color) {
		vk_d.state.colorBlend.blendEnable = VK_FALSE;
	}
	else {
		vk_d.state.colorBlend.blendEnable = VK_TRUE;
		vk_d.state.colorBlend.colorBlendOp = VK_BLEND_OP_ADD;
		vk_d.state.colorBlend.alphaBlendOp = VK_BLEND_OP_ADD;
		vk_d.state.colorBlend.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
		vk_d.state.colorBlend.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		vk_d.state.colorBlend.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		vk_d.state.colorBlend.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	}*/

    VK_UploadAttribDataOffset(&vk_d.indexbuffer, vk_d.offsetIdx * sizeof(uint32_t), sizeof(idxFullscreenQuad)/sizeof(idxFullscreenQuad[0]) * sizeof(uint32_t), (void*) &idxFullscreenQuad[0]);
	VK_UploadAttribDataOffset(&vk_d.vertexbuffer, vk_d.offset * sizeof(vec4_t), sizeof(vFullscreenQuad)/sizeof(vFullscreenQuad[0]) * sizeof(vec4_t), (void*)&vFullscreenQuad[0]);
    
	

	vkpipeline_t p = { 0 };

	VK_GetAttachmentClearPipelines(&p, clear_color, clear_depth, clear_stencil);

	/*
		vkshader_t s = { 0 };
		VK_ClearAttachmentShader(&s);

		vkdescriptor_t d = {0};

		vec4_t aaa = { 0, 1,0,1 };

		VK_SetDescriptorSet(&p, &d);
		VK_SetShader(&p, &s);
		VK_AddBindingDescription(&p, 0, sizeof(vec4_t), VK_VERTEX_INPUT_RATE_VERTEX);
		VK_AddAttributeDescription(&p, 0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 0 * sizeof(float));
		VK_AddPushConstant(&p, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(vec4_t) + sizeof(set));
		VK_FinishPipeline(&p);*/
	

		//Com_Printf("new pipe \n");
	

	//VK_BindAttribBuffer(&vk_d.fullscreenquadbuffer, 0, 0);
    
    
    
    
	VK_SetPushConstant(&p, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(color), &color);

	//Com_Printf("%d", vk_d.discardModeAlpha);
	//VK_Draw(&p, 6);
    VK_DrawIndexed(&p, &vk_d.indexbuffer, sizeof(idxFullscreenQuad)/sizeof(idxFullscreenQuad[0]), vk_d.offsetIdx, vk_d.offset);

    vk_d.offsetIdx += sizeof(idxFullscreenQuad)/sizeof(idxFullscreenQuad[0]);
    vk_d.offset += sizeof(vFullscreenQuad)/sizeof(vFullscreenQuad[0]);
    
	Com_Memcpy(&vk_d.viewport, &vpSave, sizeof(vk_d.viewport));
	Com_Memcpy(&vk_d.scissor, &sSave, sizeof(vk_d.scissor));


 //   VkClearAttachment attachments[2];
 //   uint32_t attachment_count = 0;
 //   
 //   if (clear_depth || clear_stencil){
 //       if (clear_depth) {
 //           attachments[0].aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
 //           attachments[0].clearValue.depthStencil.depth = 1.0f;
 //       }
 //       if (clear_stencil) {
 //           attachments[0].aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
 //           attachments[0].clearValue.depthStencil.stencil = 0;
 //       }
 //       attachment_count = 1;
 //   }
 //   
 //   if (clear_color) {
 //       attachments[attachment_count].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
 //       attachments[attachment_count].colorAttachment = 0;
 //       attachments[attachment_count].clearValue.color = (VkClearColorValue) { color[0], color[1], color[2], color[3] };
 //       attachment_count++;
 //   }
 //   
 //   VkClearRect clear_rect[2] = {0};
 //   clear_rect[0].rect.extent.width = vk.swapchain.extent.width;
 //   clear_rect[0].rect.extent.height = vk.swapchain.extent.height;
 //   clear_rect[0].baseArrayLayer = 0;
 //   clear_rect[0].layerCount = 1;
 //   int rect_count = 1;
 //   
 //   // Split viewport rectangle into two non-overlapping rectangles.
 //   // It's a HACK to prevent Vulkan validation layer's performance warning:
 //   //        "vkCmdClearAttachments() issued on command buffer object XXX prior to any Draw Cmds.
 //   //         It is recommended you use RenderPass LOAD_OP_CLEAR on Attachments prior to any Draw."
 //   //
 //   // NOTE: we don't use LOAD_OP_CLEAR for color attachment when we begin renderpass
 //   // since at that point we don't know whether we need collor buffer clear (usually we don't).
 //   if (clear_color) {
 //       uint32_t h = clear_rect[0].rect.extent.height / 2;
 //       clear_rect[0].rect.extent.height = h;
 //       clear_rect[1] = clear_rect[0];
 //       clear_rect[1].rect.offset.y = h;
 //       rect_count = 2;
 //   }
 //   
	//VkCommandBuffer cmdBuf = vk.swapchain.CurrentCommandBuffer();
 //   vkCmdClearAttachments(cmdBuf, attachment_count, attachments, rect_count, clear_rect);
}

/*
** PIPELINE LIST
*/
qboolean VK_FindPipeline() {
	for (uint32_t i = 0; i < vk_d.pipelineListSize; ++i) {
		vkrenderState_t* state = &vk_d.pipelineList[i].state;
		vkpipeline_t* found_p = &vk_d.pipelineList[i].pipeline;
		if (memcmp(&vk_d.state, state, sizeof(vkrenderState_t)) == 0) {
			//p = found_p;
			//Com_Memcpy(p, found_p, sizeof(vkpipeline_t));
			return i;
		}
	}
	return -1;
}

uint32_t VK_AddPipeline(vkpipeline_t* p) {
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
void VK_CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
	uint32_t memIndex, VkBuffer* buffer, VkDeviceMemory* bufferMemory) {


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
