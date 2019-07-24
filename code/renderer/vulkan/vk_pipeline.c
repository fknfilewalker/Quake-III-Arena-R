#include "../tr_local.h"

typedef struct {
    vkpipeline_t        pipeline;
    vkrenderState_t     state;
} vkpipe_t;

static uint8_t pipelineListSize;
static vkpipe_t pipelineList[100];

qboolean getPipeline(vkpipeline_t *p){
    for (uint8_t i = 0; i < pipelineListSize; ++i) {
        vkrenderState_t *state = &pipelineList[i].state;
        vkpipeline_t *found_p = &pipelineList[i].pipeline;
        if(memcmp(&vk_d.state, state, sizeof(vkrenderState_t)) == 0) {
			//p = found_p;
			Com_Memcpy(p, found_p, sizeof(vkpipeline_t));
			return qtrue;
        }
    }
    return qfalse;
}

void addPipeline(vkpipeline_t *p){
    pipelineList[pipelineListSize] = (vkpipe_t){*p, vk_d.state};
    pipelineListSize++;
}

void VK_DestroyPipeline(vkpipeline_t* pipeline);
void destroyAllPipeline() {
	for (uint8_t i = 0; i < pipelineListSize; ++i) {
		VK_DestroyPipeline(&pipelineList[i].pipeline);
	}
	pipelineListSize = 0;
	memset(&pipelineList[0], 0, sizeof(pipelineList));
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

	if (pipeline->handle) {
		vkDestroyPipeline(vk.device, pipeline->handle, NULL);
		pipeline->handle = VK_NULL_HANDLE;
	}

	if (pipeline->layout) {
		vkDestroyPipelineLayout(vk.device, pipeline->layout, NULL);
		pipeline->layout = VK_NULL_HANDLE;
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
    pipelineLayoutInfo.setLayoutCount = pipeline->descriptor->size;
    pipelineLayoutInfo.pSetLayouts = &pipeline->descriptor->layout;
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
    ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
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
    pipelineInfo.pRasterizationState = &rs;
    
    VkPipelineMultisampleStateCreateInfo ms = {0};
    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    // Enable multisampling.
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pipelineInfo.pMultisampleState = &ms;

//    VkPipelineDepthStencilStateCreateInfo depthStencil = { 0 };
//    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
//    depthStencil.depthTestEnable = VK_TRUE;
//    depthStencil.depthWriteEnable = VK_TRUE;
//    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

//
//    VkPipelineColorBlendAttachmentState colorBlend = {0};
//    colorBlend.colorWriteMask = 0xF;
//    colorBlend.blendEnable = VK_TRUE;
//    colorBlend.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
//    colorBlend.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
//    colorBlend.colorBlendOp = VK_BLEND_OP_ADD;
//    colorBlend.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
//    colorBlend.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
//    colorBlend.alphaBlendOp = VK_BLEND_OP_ADD;
    
    vk_d.state.colorBlend.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	//vk_d.state.colorBlend.colorWriteMask = 0xF;

    VkPipelineColorBlendStateCreateInfo cb = { 0 };
    cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    cb.attachmentCount = 1;
    cb.pAttachments = &vk_d.state.colorBlend;
    pipelineInfo.pColorBlendState = &cb;
    
    vk_d.state.dsBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    pipelineInfo.pDepthStencilState = &vk_d.state.dsBlend;

    VkDynamicState dynEnable[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_DEPTH_BIAS, VK_DYNAMIC_STATE_BLEND_CONSTANTS};
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

void VK_BindDescriptorSet(vkpipeline_t *pipeline, vkdescriptor_t *descriptor){
    VkCommandBuffer commandBuffer = vk.swapchain.commandBuffers[vk.swapchain.currentImage];
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->layout, 0, 1,
                            &descriptor->sets[vk.swapchain.currentFrame], 0, NULL);
}

void VK_SetPushConstant(vkpipeline_t *pipeline, VkShaderStageFlags stage, uint32_t offset, uint32_t size, void* data)
{
    VkCommandBuffer commandBuffer = vk.swapchain.commandBuffers[vk.swapchain.currentImage];
    vkCmdPushConstants(commandBuffer, pipeline->layout, stage, offset, size, data);
}

void VK_BindAttribBuffer(vkattribbuffer_t *attribBuffer, uint32_t binding, VkDeviceSize offset){
    VkCommandBuffer commandBuffer = vk.swapchain.commandBuffers[vk.swapchain.currentImage];
    VkDeviceSize bOffset = 0;
    vkCmdBindVertexBuffers(commandBuffer, binding, 1, &attribBuffer->buffer, &offset);
}

void VK_Draw(vkpipeline_t *pipeline, int count)
{
    VkCommandBuffer commandBuffer = vk.swapchain.commandBuffers[vk.swapchain.currentImage];
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->handle);
    vkCmdDraw(commandBuffer, count, 1, 0, 0);
}

void VK_DrawIndexed(vkpipeline_t *pipeline, vkattribbuffer_t *idxBuffer, int count, VkDeviceSize offset)
{
	VkCommandBuffer commandBuffer = vk.swapchain.commandBuffers[vk.swapchain.currentImage];
	vkCmdSetViewport(commandBuffer, 0, 1, &vk_d.viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &vk_d.scissor);
	if (vk_d.polygonOffset)
	{
		vkCmdSetDepthBias(commandBuffer, r_offsetUnits->value, 0.0f, r_offsetFactor->value);
	}

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->handle);

    VkDeviceSize bOffset = 0;
    vkCmdBindIndexBuffer(commandBuffer, idxBuffer->buffer, offset, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(commandBuffer, count, 1, 0, 0, 0);
}
