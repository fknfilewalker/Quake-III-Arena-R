#include "../tr_local.h"

/*
 * Use these functions to define pipeline before calling VK_FinishPipeline
 */
void VK_SetComputeDescriptorSet(vkcpipeline_t* pipeline, vkdescriptor_t* descriptor) {
	pipeline->descriptor = descriptor;
}
void VK_SetCompute2DescriptorSets(vkcpipeline_t* pipeline, vkdescriptor_t* descriptor, vkdescriptor_t* descriptor2) {
	pipeline->descriptor = descriptor;
	pipeline->descriptor2 = descriptor2;
}

void VK_SetComputeShader(vkcpipeline_t* pipeline, vkshader_t* shader) {
	pipeline->shader = shader;
}
void VK_AddComputePushConstant(vkcpipeline_t* pipeline,
	VkShaderStageFlags      stageFlags,
	uint32_t                offset,
	uint32_t                size) {
	pipeline->pushConstantRange.size++;
	pipeline->pushConstantRange.p = realloc(pipeline->pushConstantRange.p, pipeline->pushConstantRange.size * sizeof(VkPushConstantRange));
	pipeline->pushConstantRange.p[pipeline->pushConstantRange.size - 1].stageFlags = stageFlags;
	pipeline->pushConstantRange.p[pipeline->pushConstantRange.size - 1].offset = offset;
	pipeline->pushConstantRange.p[pipeline->pushConstantRange.size - 1].size = size;
}
/*
 *
 */

static void VK_CreatePipelineCache(vkcpipeline_t* pipeline)
{
	VkPipelineCacheCreateInfo pipelineCacheInfo = { 0 };
	pipelineCacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	VK_CHECK(vkCreatePipelineCache(vk.device, &pipelineCacheInfo, NULL, &pipeline->cache), "failed to create Pipeline Cache!");
}

static void VK_CreatePipelineLayout(vkcpipeline_t* pipeline)
{
	// Pipeline layout
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = { 0 };
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

	VkDescriptorSetLayout dLayout[2];
	dLayout[0] = pipeline->descriptor->layout;
	if (pipeline->descriptor2 != NULL) {
		dLayout[1] = pipeline->descriptor2->layout;
		pipelineLayoutInfo.setLayoutCount = 2;
	}
	else if (pipeline->descriptor->layout != VK_NULL_HANDLE) {
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

static void VK_CreatePipeline(vkcpipeline_t* pipeline)
{
	assert(pipeline->shader->size == 1);

	VkComputePipelineCreateInfo createInfo = { 0 };
	createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	createInfo.stage = pipeline->shader->shaderStageCreateInfos[0];
	createInfo.layout = pipeline->layout;

	VK_CHECK(vkCreateComputePipelines(vk.device, pipeline->cache, 1, &createInfo, NULL, &pipeline->handle), " failed to create Pipeline!");

}
void VK_FinishComputePipeline(vkcpipeline_t* pipeline) {
	VK_CreatePipelineCache(pipeline);
	VK_CreatePipelineLayout(pipeline);
	VK_CreatePipeline(pipeline);
}

void VK_BindCompute1DescriptorSet(vkcpipeline_t* pipeline, vkdescriptor_t* descriptor) {
	VkCommandBuffer commandBuffer = vk.swapchain.commandBuffers[vk.swapchain.currentImage];
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->layout, 0, 1,
		&descriptor->set, 0, NULL);
}
void VK_BindCompute2DescriptorSets(vkcpipeline_t* pipeline, vkdescriptor_t* descriptor1, vkdescriptor_t* descriptor2) {
	VkCommandBuffer commandBuffer = vk.swapchain.commandBuffers[vk.swapchain.currentImage];

	VkDescriptorSet sets[2] = { descriptor1->set, descriptor2->set };
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->layout, 0, 2,
		&sets[0], 0, NULL);
}
void VK_SetComputePushConstant(vkcpipeline_t* pipeline, VkShaderStageFlags stage, uint32_t offset, uint32_t size, void* data)
{
	VkCommandBuffer commandBuffer = vk.swapchain.commandBuffers[vk.swapchain.currentImage];
	vkCmdPushConstants(commandBuffer, pipeline->layout, stage, offset, size, data);
}
void VK_BindComputePipeline(vkcpipeline_t* pipeline) {
	VkCommandBuffer commandBuffer = vk.swapchain.commandBuffers[vk.swapchain.currentImage];
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->handle);
}

void VK_Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
	VkCommandBuffer commandBuffer = vk.swapchain.commandBuffers[vk.swapchain.currentImage];
	vkCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
}

void VK_DestroyCPipeline(vkcpipeline_t* pipeline) {
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

	memset(pipeline, 0, sizeof(vkcpipeline_t));
}