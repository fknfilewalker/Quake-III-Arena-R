#include "../tr_local.h"

static void VK_CreatePipelineCache(vkrtpipeline_t *pipeline);
static void VK_CreatePipelineLayout(vkrtpipeline_t*pipeline);
// RTX
static void VK_CreateShaderBindingTable(vkrtpipeline_t* pipeline);
static void VK_CreateRayTracingPipeline(vkrtpipeline_t* pipeline);

/*
 * Use these functions to define pipeline before calling VK_FinishPipeline
 */
void VK_SetRayTracingDescriptorSet(vkrtpipeline_t *pipeline, vkdescriptor_t *descriptor){
    pipeline->descriptor = descriptor;
}
void VK_Set2RayTracingDescriptorSets(vkrtpipeline_t* pipeline, vkdescriptor_t* descriptor, vkdescriptor_t* descriptor2) {
	pipeline->descriptor = descriptor;
	pipeline->descriptor2 = descriptor2;
}

void VK_SetRayTracingShader(vkrtpipeline_t *pipeline, vkshader_t *shader){
    pipeline->shader = shader;
}

void VK_AddRayTracingPushConstant(vkrtpipeline_t* pipeline,
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

void VK_FinishRayTracingPipeline(vkrtpipeline_t *pipeline) {
	VK_CreatePipelineLayout(pipeline);
	VK_CreateRayTracingPipeline(pipeline);
	VK_CreateShaderBindingTable(pipeline);
}

void VK_DestroyRayTracingPipeline(vkrtpipeline_t *pipeline) {

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

	memset(pipeline, 0, sizeof(vkrtpipeline_t));
}

static void VK_CreatePipelineCache(vkrtpipeline_t*pipeline)
{
    VkPipelineCacheCreateInfo pipelineCacheInfo = { 0 };
    pipelineCacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    VK_CHECK(vkCreatePipelineCache(vk.device, &pipelineCacheInfo, NULL, &pipeline->cache), "failed to create Pipeline Cache!");
}

static void VK_CreatePipelineLayout(vkrtpipeline_t*pipeline)
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

static void VK_CreateRayTracingPipeline(vkrtpipeline_t* pipeline)
{
	VkRayTracingShaderGroupCreateInfoNV groups[3] = { 0 };
	groups[0].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
	groups[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
	groups[0].generalShader = 0;
	groups[0].closestHitShader = VK_SHADER_UNUSED_NV;
	groups[0].anyHitShader = VK_SHADER_UNUSED_NV;
	groups[0].intersectionShader = VK_SHADER_UNUSED_NV;

	groups[1].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
	groups[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
	groups[1].generalShader = 1;
	groups[1].closestHitShader = VK_SHADER_UNUSED_NV;
	groups[1].anyHitShader = VK_SHADER_UNUSED_NV;
	groups[1].intersectionShader = VK_SHADER_UNUSED_NV;

	groups[2].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
	groups[2].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
	groups[2].generalShader = VK_SHADER_UNUSED_NV;
	groups[2].closestHitShader = 2;
	groups[2].anyHitShader = VK_SHADER_UNUSED_NV;
	groups[2].intersectionShader = VK_SHADER_UNUSED_NV;

	VkRayTracingPipelineCreateInfoNV rayPipelineInfo = { 0 };
	rayPipelineInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV;
	rayPipelineInfo.stageCount = pipeline->shader->size;
	rayPipelineInfo.pStages = &pipeline->shader->shaderStageCreateInfos[0];
	rayPipelineInfo.groupCount = 3;
	rayPipelineInfo.pGroups = &groups[0];
	rayPipelineInfo.maxRecursionDepth = 1;
	rayPipelineInfo.layout = pipeline->layout;
	VK_CHECK(vkCreateRayTracingPipelinesNV(vk.device, VK_NULL_HANDLE, 1, &rayPipelineInfo, NULL, &pipeline->handle), " failed to create Ray Tracing Pipeline");
}

static void VK_CreateShaderBindingTable(vkrtpipeline_t *pipeline) {

	const uint32_t sbtSize = vk.rayTracingProperties.shaderGroupHandleSize * 3;
	VK_CreateShaderBindingTableBuffer(&vk_d.accelerationStructures.shaderBindingTableBuffer, sbtSize);


	uint8_t* shaderHandleStorage = calloc(sbtSize, sizeof(uint8_t));
	// Get shader identifiers
	VK_CHECK(vkGetRayTracingShaderGroupHandlesNV(vk.device, pipeline->handle, 0, 3, sbtSize, shaderHandleStorage), "failed to get shader handels");
	VK_UploadBufferData(&vk_d.accelerationStructures.shaderBindingTableBuffer, (void*)shaderHandleStorage);

	VK_UnmapBuffer(&vk_d.accelerationStructures.shaderBindingTableBuffer);
	free(shaderHandleStorage);
}

void VK_BindRayTracingDescriptorSet(vkrtpipeline_t *pipeline, vkdescriptor_t *descriptor) {
	VkCommandBuffer commandBuffer = vk.swapchain.commandBuffers[vk.swapchain.currentImage];
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, pipeline->layout, 0, 1,
		&descriptor->set, 0, NULL);
}

void VK_Bind2RayTracingDescriptorSets(vkrtpipeline_t* pipeline, vkdescriptor_t* descriptor1, vkdescriptor_t* descriptor2) {
	VkCommandBuffer commandBuffer = vk.swapchain.commandBuffers[vk.swapchain.currentImage];

	VkDescriptorSet sets[2] = { descriptor1->set, descriptor2->set };
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, pipeline->layout, 0, 2,
		&sets[0], 0, NULL);
}

void VK_BindRayTracingPipeline(vkrtpipeline_t *pipeline) {
	VkCommandBuffer commandBuffer = vk.swapchain.commandBuffers[vk.swapchain.currentImage];
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, pipeline->handle);
}

void VK_SetRayTracingPushConstant(vkrtpipeline_t *pipeline, VkShaderStageFlags stage, uint32_t offset, uint32_t size, void* data)
{
    VkCommandBuffer commandBuffer = vk.swapchain.commandBuffers[vk.swapchain.currentImage];
    vkCmdPushConstants(commandBuffer, pipeline->layout, stage, offset, size, data);
}

void VK_TraceRays() {
	VkCommandBuffer commandBuffer = vk.swapchain.commandBuffers[vk.swapchain.currentImage];

	VkDeviceSize bindingOffsetRayGenShader = vk.rayTracingProperties.shaderGroupHandleSize * 0;
	VkDeviceSize bindingOffsetMissShader = vk.rayTracingProperties.shaderGroupHandleSize * 1;
	VkDeviceSize bindingOffsetHitShader = vk.rayTracingProperties.shaderGroupHandleSize * 2;
	VkDeviceSize bindingStride = vk.rayTracingProperties.shaderGroupHandleSize;

	vkCmdTraceRaysNV(commandBuffer,
		vk_d.accelerationStructures.shaderBindingTableBuffer.buffer, bindingOffsetRayGenShader,
		vk_d.accelerationStructures.shaderBindingTableBuffer.buffer, bindingOffsetMissShader, bindingStride,
		vk_d.accelerationStructures.shaderBindingTableBuffer.buffer, bindingOffsetHitShader, bindingStride,
		VK_NULL_HANDLE, 0, 0,
		vk.swapchain.extent.width, vk.swapchain.extent.height, 1);
}
