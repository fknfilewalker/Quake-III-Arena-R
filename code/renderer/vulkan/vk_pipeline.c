#include "../tr_local.h"

static vkattachmentClearPipe_t attachmentClearPipelineList[8];

void VK_DestroyPipeline(vkpipeline_t* pipeline);
void VK_DestroyAllPipelines() {
	for (uint32_t i = 0; i < vk_d.pipelineListSize; ++i) {
		VK_DestroyPipeline(&vk_d.pipelineList[i].pipeline);
	}
	vk_d.pipelineListSize = 0;
	memset(&vk_d.pipelineList[0], 0, sizeof(vk_d.pipelineList));
    
    for (uint8_t i = 0; i < 8; ++i) {
        VK_DestroyPipeline(&attachmentClearPipelineList[i].pipeline);
    }
    memset(&attachmentClearPipelineList[0], 0, sizeof(attachmentClearPipelineList));

	VK_DestroyPipeline(&vk_d.fullscreenRectPipeline);
}

static void VK_CreatePipelineCache(vkpipeline_t *pipeline);
static void VK_CreatePipelineLayout(vkpipeline_t *pipeline);
static void VK_CreatePipeline(vkpipeline_t *pipeline);

/*
 * Use these functions to define pipeline before calling VK_FinishPipeline
 */
void VK_SetDescriptorSet(vkpipeline_t *pipeline, vkdescriptor_t *descriptor){
    pipeline->descriptor = descriptor;
}
void VK_Set2DescriptorSets(vkpipeline_t *pipeline, vkdescriptor_t *descriptor, vkdescriptor_t *descriptor2){
    pipeline->descriptor = descriptor;
    pipeline->descriptor2 = descriptor2;
}

void VK_SetShader(vkpipeline_t *pipeline, vkshader_t *shader){
    pipeline->shader = shader;
}

void VK_AddBindingDescription(vkpipeline_t          *pipeline,
                              uint32_t              binding,
                              uint32_t              stride,
                              VkVertexInputRate     inputRate){
    pipeline->bindingDescription.size++;
    pipeline->bindingDescription.p = realloc(pipeline->bindingDescription.p, pipeline->bindingDescription.size * sizeof(VkVertexInputBindingDescription));
    pipeline->bindingDescription.p[pipeline->bindingDescription.size - 1].binding = binding;
    pipeline->bindingDescription.p[pipeline->bindingDescription.size - 1].stride = stride;
    pipeline->bindingDescription.p[pipeline->bindingDescription.size - 1].inputRate = inputRate;
}

void VK_AddAttributeDescription(vkpipeline_t    *pipeline,
                                  uint32_t      location,
                                  uint32_t      binding,
                                  VkFormat      format,
                                  uint32_t      offset){
    pipeline->attributeDescription.size++;
    pipeline->attributeDescription.p = realloc(pipeline->attributeDescription.p, pipeline->attributeDescription.size * sizeof(VkVertexInputAttributeDescription));
    pipeline->attributeDescription.p[pipeline->attributeDescription.size - 1].location = location;
    pipeline->attributeDescription.p[pipeline->attributeDescription.size - 1].binding = binding;
    pipeline->attributeDescription.p[pipeline->attributeDescription.size - 1].format = format;
    pipeline->attributeDescription.p[pipeline->attributeDescription.size - 1].offset = offset;
}

void VK_AddPushConstant(vkpipeline_t            *pipeline,
                        VkShaderStageFlags      stageFlags,
                        uint32_t                offset,
                        uint32_t                size){
    pipeline->pushConstantRange.size++;
    pipeline->pushConstantRange.p = realloc(pipeline->pushConstantRange.p, pipeline->pushConstantRange.size * sizeof(VkPushConstantRange));
    pipeline->pushConstantRange.p[pipeline->pushConstantRange.size - 1].stageFlags = stageFlags;
    pipeline->pushConstantRange.p[pipeline->pushConstantRange.size - 1].offset = offset;
    pipeline->pushConstantRange.p[pipeline->pushConstantRange.size - 1].size = size;
}
/*
 *
 */

void VK_FinishPipeline(vkpipeline_t *pipeline){
    VK_CreatePipelineCache(pipeline);
    VK_CreatePipelineLayout(pipeline);
    VK_CreatePipeline(pipeline);
}

void VK_DestroyPipeline(vkpipeline_t* pipeline) {
	free(pipeline->attributeDescription.p);
	free(pipeline->bindingDescription.p);
	free(pipeline->pushConstantRange.p);

    if (pipeline->layout) {
        vkDestroyPipelineLayout(vk.device, pipeline->layout, NULL);
        pipeline->layout = VK_NULL_HANDLE;
    }
    
	if (pipeline->handle) {
		vkDestroyPipeline(vk.device, pipeline->handle, NULL);
		pipeline->handle = VK_NULL_HANDLE;
	}

	if (pipeline->cache) {
		vkDestroyPipelineCache(vk.device, pipeline->cache, NULL);
		pipeline->cache = VK_NULL_HANDLE;
	}

	memset(pipeline, 0, sizeof(vkpipeline_t));
}

static void VK_CreatePipelineCache(vkpipeline_t *pipeline)
{
    VkPipelineCacheCreateInfo pipelineCacheInfo = { 0 };
    pipelineCacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    VK_CHECK(vkCreatePipelineCache(vk.device, &pipelineCacheInfo, NULL, &pipeline->cache), "failed to create Pipeline Cache!");
}

static void VK_CreatePipelineLayout(vkpipeline_t *pipeline)
{
    // Pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = { 0 };
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    
    VkDescriptorSetLayout dLayout[2];
    dLayout[0] = pipeline->descriptor->layout;
    if(pipeline->descriptor2 != NULL){
        dLayout[1] = pipeline->descriptor2->layout;
        pipelineLayoutInfo.setLayoutCount = 2;
    } else if (pipeline->descriptor->layout != VK_NULL_HANDLE) {
		pipelineLayoutInfo.setLayoutCount = 1;
	}
	else {
        pipelineLayoutInfo.setLayoutCount = 0;
    }
    pipelineLayoutInfo.pSetLayouts = &dLayout[0];
    pipelineLayoutInfo.pushConstantRangeCount = pipeline->pushConstantRange.size;
    pipelineLayoutInfo.pPushConstantRanges = &pipeline->pushConstantRange.p[0];

    VK_CHECK(vkCreatePipelineLayout(vk.device, &pipelineLayoutInfo, NULL, &pipeline->layout), "failed to create Pipeline Layout!");
}

static void VK_CreatePipeline(vkpipeline_t *pipeline)
{
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = { 0 };
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.pNext = NULL;
    vertexInputInfo.flags = 0;
    vertexInputInfo.vertexBindingDescriptionCount = pipeline->bindingDescription.size;
    vertexInputInfo.pVertexBindingDescriptions = &pipeline->bindingDescription.p[0];
    vertexInputInfo.vertexAttributeDescriptionCount = pipeline->attributeDescription.size;
    vertexInputInfo.pVertexAttributeDescriptions = &pipeline->attributeDescription.p[0];
    
    // Graphics pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo = { 0 };
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    
    // Shader
    pipelineInfo.stageCount = pipeline->shader->size;
    pipelineInfo.pStages = &pipeline->shader->shaderStageCreateInfos[0];
    pipelineInfo.pVertexInputState = &vertexInputInfo;

    VkPipelineInputAssemblyStateCreateInfo ia = {0};
    ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia.topology = vk_d.state.primitiveTopology;
	ia.primitiveRestartEnable = VK_FALSE;
    pipelineInfo.pInputAssemblyState = &ia;

    VkPipelineViewportStateCreateInfo vp = {0};
    vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vp.viewportCount = 1;
    vp.scissorCount = 1;
    pipelineInfo.pViewportState = &vp;

    VkPipelineRasterizationStateCreateInfo rs = { 0 };
    rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rs.polygonMode = vk_d.state.polygonMode;
    rs.cullMode = vk_d.state.cullMode;//VK_CULL_MODE_BACK_BIT; // we want the back face as well
    rs.frontFace = VK_FRONT_FACE_CLOCKWISE;//VK_FRONT_FACE_CLOCKWISE;
    rs.lineWidth = 1.0f;
	rs.depthBiasEnable = VK_TRUE;
    pipelineInfo.pRasterizationState = &rs;
    
    VkPipelineMultisampleStateCreateInfo ms = {0};
    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms.minSampleShading = 1.0f;
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pipelineInfo.pMultisampleState = &ms;

    vk_d.state.colorBlend.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo cb = { 0 };
    cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    cb.attachmentCount = 1;
    cb.pAttachments = &vk_d.state.colorBlend;
    pipelineInfo.pColorBlendState = &cb;
    
    vk_d.state.dsBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    pipelineInfo.pDepthStencilState = &vk_d.state.dsBlend;

    VkDynamicState dynEnable[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_DEPTH_BIAS};
    VkPipelineDynamicStateCreateInfo dyn = {0};
    dyn.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dyn.dynamicStateCount = sizeof(dynEnable) / sizeof(VkDynamicState);
    dyn.pDynamicStates = dynEnable;
    pipelineInfo.pDynamicState = &dyn;

    pipelineInfo.layout = pipeline->layout;

    // EDIT
    pipelineInfo.renderPass = vk.swapchain.renderpass;
    
    VK_CHECK(vkCreateGraphicsPipelines(vk.device, pipeline->cache, 1, &pipelineInfo, NULL, &pipeline->handle), " failed to create Pipeline!");
    
}


void VK_Bind1DescriptorSet(vkpipeline_t *pipeline, vkdescriptor_t *descriptor){
    VkCommandBuffer commandBuffer = vk.swapchain.commandBuffers[vk.swapchain.currentImage];
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->layout, 0, 1,
                            &descriptor->set, 0, NULL);
}
void VK_Bind2DescriptorSets(vkpipeline_t *pipeline, vkdescriptor_t *descriptor1, vkdescriptor_t *descriptor2){
    VkCommandBuffer commandBuffer = vk.swapchain.commandBuffers[vk.swapchain.currentImage];
    
    VkDescriptorSet sets[2] = {descriptor1->set, descriptor2->set};
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->layout, 0, 2,
                            &sets[0], 0, NULL);
}

void VK_SetPushConstant(vkpipeline_t *pipeline, VkShaderStageFlags stage, uint32_t offset, uint32_t size, void* data)
{
    VkCommandBuffer commandBuffer = vk.swapchain.commandBuffers[vk.swapchain.currentImage];
    vkCmdPushConstants(commandBuffer, pipeline->layout, stage, offset, size, data);
}

void VK_BindIndexBuffer(vkbuffer_t *idxBuffer, VkDeviceSize offset){
    VkCommandBuffer commandBuffer = vk.swapchain.commandBuffers[vk.swapchain.currentImage];
    VkDeviceSize bOffset = 0;
    vkCmdBindIndexBuffer(commandBuffer, idxBuffer->buffer, offset, VK_INDEX_TYPE_UINT32);
}

void VK_BindAttribBuffer(vkbuffer_t *attribBuffer, uint32_t binding, VkDeviceSize offset){
    VkCommandBuffer commandBuffer = vk.swapchain.commandBuffers[vk.swapchain.currentImage];
    VkDeviceSize bOffset = 0;
    vkCmdBindVertexBuffers(commandBuffer, binding, 1, &attribBuffer->buffer, &offset);
}

void VK_BindPipeline(vkpipeline_t* pipeline) {
	VkCommandBuffer commandBuffer = vk.swapchain.commandBuffers[vk.swapchain.currentImage];
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->handle);
}

void VK_Draw(int count)
{
    VkCommandBuffer commandBuffer = vk.swapchain.commandBuffers[vk.swapchain.currentImage];
	vkCmdSetViewport(commandBuffer, 0, 1, &vk_d.viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &vk_d.scissor);
	vkCmdSetDepthBias(commandBuffer, 0.0f, 0.0f, 0.0f);
    vkCmdDraw(commandBuffer, count, 1, 0, 0);
}

void VK_DrawIndexed(vkbuffer_t *idxBuffer, int count, uint32_t firstIndex, uint32_t vertexOffset)
{
	//return;
	VkCommandBuffer commandBuffer = vk.swapchain.commandBuffers[vk.swapchain.currentImage];
	vkCmdSetViewport(commandBuffer, 0, 1, &vk_d.viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &vk_d.scissor);
	if (vk_d.polygonOffset)
	{
		vkCmdSetDepthBias(commandBuffer, r_offsetUnits->value, 0.0f, r_offsetFactor->value - 3);
	} else vkCmdSetDepthBias(commandBuffer, 0.0f, 0.0f, 0.0f);

    VkDeviceSize bOffset = 0;
    vkCmdDrawIndexed(commandBuffer, count, 1, firstIndex, vertexOffset, 0);
}

qboolean VK_GetAttachmentClearPipelines(vkpipeline_t *pipeline, qboolean clearColor, qboolean clearDepth, qboolean clearStencil) {
    vkattachmentClearPipe_t p;
    for (uint8_t i = 0; i < 8; ++i) {
		 p = attachmentClearPipelineList[i];

		if (p.clearColor == clearColor && p.clearDepth == clearDepth && p.clearStencil == clearStencil) {
			Com_Memcpy(pipeline, &p.pipeline, sizeof(vkpipeline_t));
			return qtrue;
		}
	}
	return qfalse;
}

void VK_InitPipelines() {
	int clearColor = 0;
	int clearDepth = 0;
	int clearStencil = 0;

	int attachmentClearPipelinePos = 0;
	for (clearColor = 0; clearColor < 2; ++clearColor) {
		for (clearDepth = 0; clearDepth < 2; ++clearDepth) {
			for (clearStencil = 0; clearStencil < 2; ++clearStencil) {
				vkattachmentClearPipe_t pipeline;

				//memset(&vk_d.state.dsBlend, 0, sizeof(vk_d.state.dsBlend));
				//memset(&vk_d.state.colorBlend, 0, sizeof(vk_d.state.colorBlend));
				vk_d.state.polygonMode = VK_POLYGON_MODE_FILL;
				vk_d.state.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
				vk_d.state.cullMode = VK_CULL_MODE_NONE;
				if (clearColor) {
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
				}

				if (clearDepth) {
					vk_d.state.dsBlend.depthTestEnable = VK_TRUE;
					vk_d.state.dsBlend.depthWriteEnable = VK_TRUE;
					vk_d.state.dsBlend.depthCompareOp = VK_COMPARE_OP_ALWAYS;
				}
				else {
					vk_d.state.dsBlend.depthTestEnable = VK_FALSE;
					vk_d.state.dsBlend.depthWriteEnable = VK_FALSE;
				}

				vk_d.state.dsBlend.stencilTestEnable = VK_FALSE;
				if (clearStencil) {
					vk_d.state.dsBlend.stencilTestEnable = VK_TRUE;
					VkStencilOpState stencilSettings = { 0 };
					stencilSettings.failOp = VK_STENCIL_OP_ZERO;
                    stencilSettings.depthFailOp = VK_STENCIL_OP_ZERO;
					stencilSettings.passOp = VK_STENCIL_OP_ZERO;
					stencilSettings.compareOp = VK_COMPARE_OP_ALWAYS;
					stencilSettings.reference = 0xFF;
                    stencilSettings.writeMask = 0xFF;
                    stencilSettings.compareMask = 0xFF;
					vk_d.state.dsBlend.back = stencilSettings;
					vk_d.state.dsBlend.front = stencilSettings;

				}
				else {
				}

				vkpipeline_t p = { 0 };
				vkshader_t s = { 0 };
				VK_ClearAttachmentShader(&s);
				vkdescriptor_t d = { 0 };

				VK_SetDescriptorSet(&p, &d);
				VK_SetShader(&p, &s);
                VK_AddBindingDescription(&p, 0, sizeof(vec4_t), VK_VERTEX_INPUT_RATE_VERTEX);
                VK_AddBindingDescription(&p, 1, sizeof(color4ub_t), VK_VERTEX_INPUT_RATE_VERTEX);
                VK_AddBindingDescription(&p, 2, sizeof(vec2_t), VK_VERTEX_INPUT_RATE_VERTEX);
                VK_AddAttributeDescription(&p, 0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 0 * sizeof(float));
                VK_AddAttributeDescription(&p, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, 0 * sizeof(float));
                VK_AddAttributeDescription(&p, 2, 2, VK_FORMAT_R32G32_SFLOAT, 0 * sizeof(float));
				VK_AddPushConstant(&p, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(vec4_t));
				VK_FinishPipeline(&p);

				pipeline.pipeline = p;
				pipeline.clearColor = clearColor;
				pipeline.clearDepth = clearDepth;
				pipeline.clearStencil = clearStencil;
				attachmentClearPipelineList[attachmentClearPipelinePos] = pipeline;
				attachmentClearPipelinePos++;
			}
		}
	}
	return;
}
