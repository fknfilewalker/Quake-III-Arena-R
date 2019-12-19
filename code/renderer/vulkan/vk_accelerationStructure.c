#include "../tr_local.h"

#define max(a,b) (((a)>(b))?(a):(b))

void VK_CreateBottomAS(VkCommandBuffer commandBuffer, 
						vkbottomAS_t* bas, vkbuffer_t *bottomASBuffer, 
						VkDeviceSize* offset, VkBuildAccelerationStructureFlagsNV flag) {
	// create
	VkAccelerationStructureInfoNV accelerationStructureInfo = { 0 };
	accelerationStructureInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
	accelerationStructureInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
	accelerationStructureInfo.flags = flag;
	accelerationStructureInfo.instanceCount = 0;
	accelerationStructureInfo.geometryCount = 1;
	accelerationStructureInfo.pGeometries = &bas->geometries;

	VkAccelerationStructureCreateInfoNV accelerationStructureCI = { 0 };
	accelerationStructureCI.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
	accelerationStructureCI.info = accelerationStructureInfo;
	VK_CHECK(vkCreateAccelerationStructureNV(vk.device, &accelerationStructureCI, NULL, &bas->accelerationStructure), "failed to create Bottom Level Acceleration Structure NV");

	// Get Memory Info
	VkMemoryRequirements2 memoryRequirements2Scratch = { 0 };
	VK_GetAccelerationStructureMemoryRequirements(bas->accelerationStructure, VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV, &memoryRequirements2Scratch);
	VkMemoryRequirements2 memoryRequirements2 = { 0 };
	VK_GetAccelerationStructureMemoryRequirements(bas->accelerationStructure, VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV, &memoryRequirements2);
	if (memoryRequirements2.memoryRequirements.size > bottomASBuffer->allocSize - (offset != NULL ? *offset : bas->offset)) {
		ri.Error(ERR_FATAL, "Vulkan: Top Level Buffer to small!");
	}
	if (memoryRequirements2Scratch.memoryRequirements.size > vk_d.scratchBuffer.allocSize - vk_d.scratchBufferOffset) {
		ri.Error(ERR_FATAL, "Vulkan: Scratch Buffer to small!");
	}


	VkBindAccelerationStructureMemoryInfoNV memoryInfo = { 0 };
	memoryInfo.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV;
	memoryInfo.accelerationStructure = bas->accelerationStructure;
	memoryInfo.memory = bottomASBuffer->memory;
	// if no offset it present take the one from the as
	if (offset != NULL) {
		memoryInfo.memoryOffset = bas->offset = *offset;
		*offset += (memoryRequirements2.memoryRequirements.size);
	}
	else memoryInfo.memoryOffset = bas->offset;

	
	VK_CHECK(vkBindAccelerationStructureMemoryNV(vk.device, 1, &memoryInfo), "failed to bind Acceleration Structure Memory NV");
	VK_CHECK(vkGetAccelerationStructureHandleNV(vk.device, bas->accelerationStructure, sizeof(uint64_t), &bas->handle), "failed to get Acceleration Structure Handle NV");
	
	// build
	VkAccelerationStructureInfoNV buildInfo = { 0 };
	buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
	buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
	buildInfo.flags = flag;
	buildInfo.geometryCount = 1;
	buildInfo.pGeometries = &bas->geometries;

	vkCmdBuildAccelerationStructureNV(
		commandBuffer,
		&buildInfo,
		VK_NULL_HANDLE,
		0,
		VK_FALSE,
		bas->accelerationStructure,
		VK_NULL_HANDLE,
		vk_d.scratchBuffer.buffer,
		vk_d.scratchBufferOffset);
	vk_d.scratchBufferOffset += memoryRequirements2Scratch.memoryRequirements.size;
}
void VK_UpdateBottomAS(VkCommandBuffer commandBuffer, 
						vkbottomAS_t* oldBas, vkbottomAS_t* newBas, 
						vkbuffer_t* bottomASBuffer, VkDeviceSize* offset, VkBuildAccelerationStructureFlagsNV flag) {
	
	VkMemoryRequirements2 memoryRequirements2Scratch = { 0 };
	VkMemoryRequirements2 memoryRequirements2 = { 0 };
	// if old and new are the same do not create new as
	if (oldBas == newBas) {
		// Get Memory Info
		VK_GetAccelerationStructureMemoryRequirements(oldBas->accelerationStructure, VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_UPDATE_SCRATCH_NV, &memoryRequirements2Scratch);
		VK_GetAccelerationStructureMemoryRequirements(oldBas->accelerationStructure, VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV, &memoryRequirements2);
	}
	else {
		// create new as
		VkAccelerationStructureInfoNV accelerationStructureInfo = { 0 };
		accelerationStructureInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
		accelerationStructureInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
		accelerationStructureInfo.flags = flag;
		accelerationStructureInfo.instanceCount = 0;
		accelerationStructureInfo.geometryCount = 1;
		accelerationStructureInfo.pGeometries = &newBas->geometries;

		VkAccelerationStructureCreateInfoNV accelerationStructureCI = { 0 };
		accelerationStructureCI.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
		accelerationStructureCI.info = accelerationStructureInfo;
		VK_CHECK(vkCreateAccelerationStructureNV(vk.device, &accelerationStructureCI, NULL, &newBas->accelerationStructure), "failed to create Bottom Level Acceleration Structure NV");

		// Get Memory Info
		VK_GetAccelerationStructureMemoryRequirements(newBas->accelerationStructure, VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_UPDATE_SCRATCH_NV, &memoryRequirements2Scratch);
		VK_GetAccelerationStructureMemoryRequirements(newBas->accelerationStructure, VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV, &memoryRequirements2);

		VkBindAccelerationStructureMemoryInfoNV memoryInfo = { 0 };
		memoryInfo.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV;
		memoryInfo.accelerationStructure = newBas->accelerationStructure;
		memoryInfo.memory = bottomASBuffer->memory;
		memoryInfo.memoryOffset = newBas->offset = *offset;
		*offset += (memoryRequirements2.memoryRequirements.size);

		VK_CHECK(vkBindAccelerationStructureMemoryNV(vk.device, 1, &memoryInfo), "failed to bind Acceleration Structure Memory NV");
		VK_CHECK(vkGetAccelerationStructureHandleNV(vk.device, newBas->accelerationStructure, sizeof(uint64_t), &newBas->handle), "failed to get Acceleration Structure Handle NV");
	}

	if (memoryRequirements2Scratch.memoryRequirements.size > vk_d.scratchBuffer.allocSize - vk_d.scratchBufferOffset) {
		ri.Error(ERR_FATAL, "Vulkan: Scratch Buffer to small!");
	}

	// build
	VkAccelerationStructureInfoNV buildInfo = { 0 };
	buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
	buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
	buildInfo.flags = flag;
	buildInfo.geometryCount = 1;
	buildInfo.pGeometries = &newBas->geometries;

	VkMemoryBarrier memoryBarrier = { 0 };
	memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
	memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;

	vkCmdBuildAccelerationStructureNV(
		commandBuffer,
		&buildInfo,
		VK_NULL_HANDLE,
		0,
		VK_TRUE,
		newBas->accelerationStructure,
		oldBas->accelerationStructure,
		vk_d.scratchBuffer.buffer,
		vk_d.scratchBufferOffset);
	vk_d.scratchBufferOffset += memoryRequirements2Scratch.memoryRequirements.size;

	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 0, 1, &memoryBarrier, 0, 0, 0, 0);
}

// destroy and create new AS
void VK_RecreateBottomAS(VkCommandBuffer commandBuffer, vkbottomAS_t* bas, vkbuffer_t* bottomASBuffer, VkBuildAccelerationStructureFlagsNV flag) {
	vkDestroyAccelerationStructureNV(vk.device, bas->accelerationStructure, NULL);
	bas->handle = 0;
	VK_CreateBottomAS(commandBuffer, bas, bottomASBuffer, NULL, flag);
}

void VK_MakeTopAS(VkCommandBuffer commandBuffer, 
					vktopAS_t* topAS, vkbuffer_t* topASBuffer, 
					vkbottomAS_t* basList, uint32_t basCount, vkbuffer_t* instanceBuffer,
					VkBuildAccelerationStructureFlagsNV flag) {
	VkAccelerationStructureInfoNV accelerationStructureInfo = { 0 };
	accelerationStructureInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
	accelerationStructureInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
	accelerationStructureInfo.flags = flag;
	accelerationStructureInfo.instanceCount = basCount;
	accelerationStructureInfo.geometryCount = 0;

	VkAccelerationStructureCreateInfoNV accelerationStructureCI = { 0 };
	accelerationStructureCI.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
	accelerationStructureCI.info = accelerationStructureInfo;
	VK_CHECK(vkCreateAccelerationStructureNV(vk.device, &accelerationStructureCI, NULL, &topAS->accelerationStructure), "failed to create Bottom Level Acceleration Structure NV");

	// Get Memory Info
	VkMemoryRequirements2 memoryRequirements2Scratch = { 0 };
	VK_GetAccelerationStructureMemoryRequirements(topAS->accelerationStructure, VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV, &memoryRequirements2Scratch);
	VkMemoryRequirements2 memoryRequirements2 = { 0 };
	VK_GetAccelerationStructureMemoryRequirements(topAS->accelerationStructure, VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV, &memoryRequirements2);

	if (memoryRequirements2.memoryRequirements.size > topASBuffer->allocSize) {
		ri.Error(ERR_FATAL, "Vulkan: Top Level Buffer to small!");
	}
	if (memoryRequirements2Scratch.memoryRequirements.size > vk_d.scratchBuffer.allocSize - vk_d.scratchBufferOffset) {
		ri.Error(ERR_FATAL, "Vulkan: Scratch Buffer to small!");
	}

	VkBindAccelerationStructureMemoryInfoNV memoryInfo = { 0 };
	memoryInfo.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV;
	memoryInfo.accelerationStructure = topAS->accelerationStructure;
	memoryInfo.memoryOffset = 0;
	memoryInfo.memory = topASBuffer->memory;

	VK_CHECK(vkBindAccelerationStructureMemoryNV(vk.device, 1, &memoryInfo), "failed to bind Acceleration Structure Memory NV");
	VK_CHECK(vkGetAccelerationStructureHandleNV(vk.device, topAS->accelerationStructure, sizeof(uint64_t), &topAS->handle), "failed to get Acceleration Structure Handle NV");

	// build
	VkAccelerationStructureInfoNV buildInfo = { 0 };
	buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
	buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
	buildInfo.flags = flag;
	buildInfo.pGeometries = 0;
	buildInfo.geometryCount = 0;
	buildInfo.instanceCount = basCount;

	VkMemoryBarrier memoryBarrier = { 0 };
	memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
	memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
	
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 0, 1, &memoryBarrier, 0, 0, 0, 0);
	vkCmdBuildAccelerationStructureNV(
		commandBuffer,
		&buildInfo,
		instanceBuffer->buffer,
		0,
		VK_FALSE,
		topAS->accelerationStructure,
		VK_NULL_HANDLE,
		vk_d.scratchBuffer.buffer,
		vk_d.scratchBufferOffset);
	vk_d.scratchBufferOffset += memoryRequirements2Scratch.memoryRequirements.size;
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 0, 1, &memoryBarrier, 0, 0, 0, 0);
}

void VK_UpdateTopAS(VkCommandBuffer commandBuffer,
	vktopAS_t* topASold, vktopAS_t* topASnew, vkbuffer_t* topASBuffer,
	vkbottomAS_t* basList, uint32_t basCount, vkbuffer_t* instanceBuffer,
	VkBuildAccelerationStructureFlagsNV flag) {

	// Get Memory Info
	VkMemoryRequirements2 memoryRequirements2Scratch = { 0 };
	VK_GetAccelerationStructureMemoryRequirements(topASold->accelerationStructure, VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_UPDATE_SCRATCH_NV, &memoryRequirements2Scratch);
	VkMemoryRequirements2 memoryRequirements2 = { 0 };
	VK_GetAccelerationStructureMemoryRequirements(topASold->accelerationStructure, VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV, &memoryRequirements2);

	if (memoryRequirements2.memoryRequirements.size > topASBuffer->allocSize) {
		ri.Error(ERR_FATAL, "Vulkan: Top Level Buffer to small!");
	}
	if (memoryRequirements2Scratch.memoryRequirements.size > vk_d.scratchBuffer.allocSize - vk_d.scratchBufferOffset) {
		ri.Error(ERR_FATAL, "Vulkan: Scratch Buffer to small!");
	}

	// build
	VkAccelerationStructureInfoNV buildInfo = { 0 };
	buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
	buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
	buildInfo.flags = flag;
	buildInfo.pGeometries = 0;
	buildInfo.geometryCount = 0;
	buildInfo.instanceCount = basCount;

	VkMemoryBarrier memoryBarrier = { 0 };
	memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
	memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;

	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 0, 1, &memoryBarrier, 0, 0, 0, 0);
	vkCmdBuildAccelerationStructureNV(
		commandBuffer,
		&buildInfo,
		instanceBuffer->buffer,
		0,
		VK_TRUE,
		topASnew->accelerationStructure,
		topASold->accelerationStructure,
		vk_d.scratchBuffer.buffer,
		vk_d.scratchBufferOffset);
	vk_d.scratchBufferOffset += memoryRequirements2Scratch.memoryRequirements.size;
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 0, 1, &memoryBarrier, 0, 0, 0, 0);
}

void VK_DestroyTopAccelerationStructure(vktopAS_t* as) {
	if (as->accelerationStructure != VK_NULL_HANDLE) {
		vkDestroyAccelerationStructureNV(vk.device, as->accelerationStructure, NULL);
		as->accelerationStructure = VK_NULL_HANDLE;
	}
	memset(as, 0, sizeof(vktopAS_t));
}

void VK_DestroyBottomAccelerationStructure(vkbottomAS_t* as) {
	if (as->accelerationStructure != VK_NULL_HANDLE) {
		vkDestroyAccelerationStructureNV(vk.device, as->accelerationStructure, NULL);
		as->accelerationStructure = VK_NULL_HANDLE;
	}
	memset(as, 0, sizeof(vkbottomAS_t));
}

void VK_DestroyAllAccelerationStructures() {
	for (int i = 0; i < vk.swapchain.imageCount; i++) {
		VK_DestroyTopAccelerationStructure(&vk_d.topAS[i]);
	}
	for (int i = 0; i < vk_d.bottomASCount; ++i) {
		VK_DestroyBottomAccelerationStructure(&vk_d.bottomASList[i]);
	}
	vk_d.bottomASCount = 0;
}