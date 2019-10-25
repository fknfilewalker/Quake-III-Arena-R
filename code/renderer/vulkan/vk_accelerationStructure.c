#include "../tr_local.h"

#define max(a,b) (((a)>(b))?(a):(b))

//static void VK_CreateBottomLevelAccelerationStructure(vkaccelerationStructures_t* as, const VkGeometryNV* geometries);
//static void VK_CreateTopLevelAccelerationStructure(vkaccelerationStructures_t* as);
//static void VK_BuildAccelerationStructure(vkaccelerationStructures_t* as, const VkGeometryNV* geometries);

void VK_MakeBottomSingle(vkbottomAS_t* bas, VkBuildAccelerationStructureFlagsNV flag) {

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

	VkBindAccelerationStructureMemoryInfoNV memoryInfo = { 0 };
	memoryInfo.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV;
	memoryInfo.accelerationStructure = bas->accelerationStructure;
	memoryInfo.memoryOffset = vk_d.basBufferOffset;
	memoryInfo.memory = vk_d.basBuffer.memory;
	bas->offset = vk_d.basBufferOffset;
	vk_d.basBufferOffset += (memoryRequirements2.memoryRequirements.size * 4);
	
	VK_CHECK(vkBindAccelerationStructureMemoryNV(vk.device, 1, &memoryInfo), "failed to bind Acceleration Structure Memory NV");
	VK_CHECK(vkGetAccelerationStructureHandleNV(vk.device, bas->accelerationStructure, sizeof(uint64_t), &bas->handle), "failed to get Acceleration Structure Handle NV");
	
	// build
	VkCommandBuffer commandBuffer = { 0 };
	VK_BeginSingleTimeCommands(&commandBuffer);

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
		0);

	VK_EndSingleTimeCommands(&commandBuffer);
	
}
void VK_UpdateBottomSingleDelete(vkbottomAS_t* bas, VkBuildAccelerationStructureFlagsNV flag) {
	vkDestroyAccelerationStructureNV(vk.device, bas->accelerationStructure, NULL);
	bas->handle = 0;
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


	VkBindAccelerationStructureMemoryInfoNV memoryInfo = { 0 };
	memoryInfo.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV;
	memoryInfo.accelerationStructure = bas->accelerationStructure;
	memoryInfo.memoryOffset = bas->offset;
	memoryInfo.memory = vk_d.basBuffer.memory;

	//bas->offset = vk_d.asBufferOffset;
	if ((bas+1) != NULL) {
		VkDeviceSize size = (bas + 1)->offset - bas->offset;
		if (memoryRequirements2.memoryRequirements.size > size) {
			ri.Error(ERR_FATAL, "Vulkan: Buffer to small!");
		}
	}
	/*memoryInfo.memoryOffset = vk_d.asBufferOffset;
	memoryInfo.memory = vk_d.asBuffer.memory;
	bas->offset = vk_d.asBufferOffset;*/
	
	VK_CHECK(vkBindAccelerationStructureMemoryNV(vk.device, 1, &memoryInfo), "failed to bind Acceleration Structure Memory NV");
	VK_CHECK(vkGetAccelerationStructureHandleNV(vk.device, bas->accelerationStructure, sizeof(uint64_t), &bas->handle), "failed to get Acceleration Structure Handle NV");

	// build
	VkCommandBuffer commandBuffer = { 0 };
	VK_BeginSingleTimeCommands(&commandBuffer);

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
		0);

	VK_EndSingleTimeCommands(&commandBuffer);

}
void VK_UpdateBottomSingle(vkbottomAS_t* bas) {
	
	// Get Memory Info
	VkMemoryRequirements2 memoryRequirements2Scratch = { 0 };
	VK_GetAccelerationStructureMemoryRequirements(bas->accelerationStructure, VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_UPDATE_SCRATCH_NV, &memoryRequirements2Scratch);

	VkMemoryRequirements2 memoryRequirements2 = { 0 };
	VK_GetAccelerationStructureMemoryRequirements(bas->accelerationStructure, VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV, &memoryRequirements2);


	VkCommandBuffer commandBuffer = { 0 };
	VK_BeginSingleTimeCommands(&commandBuffer);

	VkAccelerationStructureInfoNV buildInfo = { 0 };
	buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
	buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
	buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV;
	buildInfo.geometryCount = 1;
	buildInfo.pGeometries = &bas->geometries;

	vkCmdBuildAccelerationStructureNV(
		commandBuffer,
		&buildInfo,
		VK_NULL_HANDLE,
		0,
		VK_TRUE,
		bas->accelerationStructure,
		bas->accelerationStructure,
		vk_d.scratchBuffer.buffer,
		0);

	VK_EndSingleTimeCommands(&commandBuffer);
}

void VK_MakeTop(vktopAS_t* tas, vkbottomAS_t* basList, uint32_t basCount, VkBuildAccelerationStructureFlagsNV flag) {
	VkAccelerationStructureInfoNV accelerationStructureInfo = { 0 };
	accelerationStructureInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
	accelerationStructureInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
	accelerationStructureInfo.flags = flag;
	accelerationStructureInfo.instanceCount = basCount;
	accelerationStructureInfo.geometryCount = 0;

	VkAccelerationStructureCreateInfoNV accelerationStructureCI = { 0 };
	accelerationStructureCI.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
	accelerationStructureCI.info = accelerationStructureInfo;
	VK_CHECK(vkCreateAccelerationStructureNV(vk.device, &accelerationStructureCI, NULL, &tas->accelerationStructure), "failed to create Bottom Level Acceleration Structure NV");

	// Get Memory Info
	VkMemoryRequirements2 memoryRequirements2Scratch = { 0 };
	VK_GetAccelerationStructureMemoryRequirements(tas->accelerationStructure, VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV, &memoryRequirements2Scratch);

	VkMemoryRequirements2 memoryRequirements2 = { 0 };
	VK_GetAccelerationStructureMemoryRequirements(tas->accelerationStructure, VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV, &memoryRequirements2);


	/*
	VkMemoryAllocateInfo memoryAllocateInfo = { 0 };
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements2.memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = VK_FindMemoryTypeIndex(memoryRequirements2.memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK(vkAllocateMemory(vk.device, &memoryAllocateInfo, NULL, &bList->topMemory), "failed to Allocate Memory NV");
	*/

	VkBindAccelerationStructureMemoryInfoNV memoryInfo = { 0 };
	memoryInfo.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV;
	memoryInfo.accelerationStructure = tas->accelerationStructure;
	memoryInfo.memoryOffset = vk_d.tasBufferOffset;
	memoryInfo.memory = vk_d.tasBuffer.memory;
	tas->offset = vk_d.tasBufferOffset;
	vk_d.tasBufferOffset += (memoryRequirements2.memoryRequirements.size);

	VK_CHECK(vkBindAccelerationStructureMemoryNV(vk.device, 1, &memoryInfo), "failed to bind Acceleration Structure Memory NV");
	VK_CHECK(vkGetAccelerationStructureHandleNV(vk.device, tas->accelerationStructure, sizeof(uint64_t), &tas->handle), "failed to get Acceleration Structure Handle NV");

	// build
	//VkMemoryRequirements2 memReqTopLevelAS;
	//memoryRequirementsInfo.accelerationStructure = bList->topAS.accelerationStructure;
	//vkGetAccelerationStructureMemoryRequirementsNV(vk.device, &memoryRequirementsInfo, &memReqTopLevelAS);

	// fill instance buffer
	VkGeometryInstanceNV geometryInstance = { 0 };
	float transform[12] = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f
	};
	Com_Memcpy(&geometryInstance.transform, &transform, sizeof(float[12]));
	geometryInstance.instanceCustomIndex = 0;
	geometryInstance.mask = 0xff;
	geometryInstance.instanceOffset = 0;


	VkAccelerationStructureInfoNV buildInfo = { 0 };
	buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
	buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
	buildInfo.flags = flag;
	buildInfo.pGeometries = 0;
	buildInfo.geometryCount = 0;
	buildInfo.instanceCount = basCount;

	uint32_t numIndex = 0;
	//for (int i = 0; i < basCount; i++) {
	//	geometryInstance.accelerationStructureHandle = basList[i].handle;
	//	geometryInstance.flags = basList[i].flags;
	//	//geometryInstance.instanceCustomIndex = numIndex;
	//	//geometryInstance.mask = 0xff;
	//	//numIndex += bList->bottomASList[i].geometries.geometry.triangles.indexCount;
	//	//VK_UploadBufferDataOffset(&vk_d.instanceBuffer, i * sizeof(VkGeometryInstanceNV), sizeof(VkGeometryInstanceNV), (void*)&geometryInstance);
	//}

	VkCommandBuffer commandBuffer = { 0 };
	VK_BeginSingleTimeCommands(&commandBuffer);

	/*VkMemoryBarrier memoryBarrier = { 0 };
	memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
	memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 0, 1, &memoryBarrier, 0, 0, 0, 0);*/

	vkCmdBuildAccelerationStructureNV(
		commandBuffer,
		&buildInfo,
		vk_d.instanceBuffer.buffer,
		0,
		VK_FALSE,
		tas->accelerationStructure,
		VK_NULL_HANDLE,
		vk_d.scratchBuffer.buffer,
		0);

	//vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 0, 1, &memoryBarrier, 0, 0, 0, 0);

	VK_EndSingleTimeCommands(&commandBuffer);

}


void VK_UpdateTop(vktopAS_t* top, vkbottomAS_t* bottomList, uint32_t bottomCount)
{
	// Get Memory Info
	VkMemoryRequirements2 memoryRequirements2Scratch = { 0 };
	VK_GetAccelerationStructureMemoryRequirements(top->accelerationStructure, VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_UPDATE_SCRATCH_NV, &memoryRequirements2Scratch);

	VkMemoryRequirements2 memoryRequirements2 = { 0 };
	VK_GetAccelerationStructureMemoryRequirements(top->accelerationStructure, VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV, &memoryRequirements2);

	// build

	// create instance buffer
	VkGeometryInstanceNV geometryInstance = { 0 };
	float transform[12] = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f
	};
	Com_Memcpy(&geometryInstance.transform, &transform, sizeof(float[12]));
	geometryInstance.instanceCustomIndex = 0;
	geometryInstance.mask = 0xff;
	geometryInstance.instanceOffset = 0;
	geometryInstance.flags = 0;//VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV;


	VkAccelerationStructureInfoNV buildInfo = { 0 };
	buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
	buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
	buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV;
	buildInfo.pGeometries = 0;
	buildInfo.geometryCount = 0;
	buildInfo.instanceCount = bottomCount;

	uint32_t numIndex = 0;
	//for (int i = 0; i < bottomCount; i++) {
	//	geometryInstance.accelerationStructureHandle = bottomList[i].handle;
	//	geometryInstance.flags = bottomList[i].flags;
	//	//geometryInstance.instanceCustomIndex = numIndex;
	//	//geometryInstance.mask = 0xff;
	//	//numIndex += bList->bottomASList[i].geometries.geometry.triangles.indexCount;
	//	VK_UploadBufferDataOffset(&vk_d.instanceBuffer, i * sizeof(VkGeometryInstanceNV), sizeof(VkGeometryInstanceNV), (void*)&geometryInstance);
	//}

	VkCommandBuffer commandBuffer = { 0 };
	VK_BeginSingleTimeCommands(&commandBuffer);

	/*VkMemoryBarrier memoryBarrier = { 0 };
	memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
	memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 0, 1, &memoryBarrier, 0, 0, 0, 0);*/

	vkCmdBuildAccelerationStructureNV(
		commandBuffer,
		&buildInfo,
		vk_d.instanceBuffer.buffer,
		0,
		VK_TRUE,
		top->accelerationStructure,
		top->accelerationStructure,
		vk_d.scratchBuffer.buffer,
		0);

	//vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 0, 1, &memoryBarrier, 0, 0, 0, 0);

	VK_EndSingleTimeCommands(&commandBuffer);
}
//
//void VK_DestroyAccelerationStructure2(vkbottomASList_t* as) {
//	if (as->bottomMemory != VK_NULL_HANDLE) {
//		vkFreeMemory(vk.device, as->bottomMemory, NULL);
//		as->bottomMemory = VK_NULL_HANDLE;
//	}
//	if (as->topMemory != VK_NULL_HANDLE) {
//		vkFreeMemory(vk.device, as->topMemory, NULL);
//		as->topMemory = VK_NULL_HANDLE;
//	}
//
//	for (int i = 0; i < as->asCount; i++) {
//		if (as->bottomASList[i].accelerationStructure != VK_NULL_HANDLE) {
//			vkDestroyAccelerationStructureNV(vk.device, as->bottomASList[i].accelerationStructure, NULL);
//			as->bottomASList[i].accelerationStructure = VK_NULL_HANDLE;
//		}
//	}
//
//
//	if (as->topAS.accelerationStructure != VK_NULL_HANDLE) {
//		vkDestroyAccelerationStructureNV(vk.device, as->topAS.accelerationStructure, NULL);
//		as->topAS.accelerationStructure = VK_NULL_HANDLE;
//	}
//
//	free(as->bottomASList);
//	as->asCount = 0;
//}
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
	int i = 0;
	for (i = 0; i < vk_d.bottomASCount; ++i) {
		VK_DestroyBottomAccelerationStructure(&vk_d.bottomASList[i]);
	}
	VK_DestroyTopAccelerationStructure(&vk_d.topAS);

	VK_DestroyBuffer(&vk_d.basBuffer);
	VK_DestroyBuffer(&vk_d.tasBuffer);
	vk_d.basBufferOffset = 0;
	vk_d.tasBufferOffset = 0;
}