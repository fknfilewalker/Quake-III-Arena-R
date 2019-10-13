#include "../tr_local.h"

#define max(a,b) (((a)>(b))?(a):(b))

//static void VK_CreateBottomLevelAccelerationStructure(vkaccelerationStructures_t* as, const VkGeometryNV* geometries);
//static void VK_CreateTopLevelAccelerationStructure(vkaccelerationStructures_t* as);
//static void VK_BuildAccelerationStructure(vkaccelerationStructures_t* as, const VkGeometryNV* geometries);

void VK_MakeBottomSingle(vkbottomAS_t* bas) {

	// create
	VkAccelerationStructureInfoNV accelerationStructureInfo = { 0 };
	accelerationStructureInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
	accelerationStructureInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
	accelerationStructureInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV;
	accelerationStructureInfo.instanceCount = 0;
	accelerationStructureInfo.geometryCount = 1;
	accelerationStructureInfo.pGeometries = &bas->geometries;

	VkAccelerationStructureCreateInfoNV accelerationStructureCI = { 0 };
	accelerationStructureCI.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
	accelerationStructureCI.info = accelerationStructureInfo;
	VK_CHECK(vkCreateAccelerationStructureNV(vk.device, &accelerationStructureCI, NULL, &bas->accelerationStructure), "failed to create Bottom Level Acceleration Structure NV");

	// Get Memory Info
	VkAccelerationStructureMemoryRequirementsInfoNV memoryRequirementsInfo = { 0 };
	memoryRequirementsInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
	memoryRequirementsInfo.accelerationStructure = bas->accelerationStructure;
	VkMemoryRequirements2 memoryRequirements2 = { 0 };
	vkGetAccelerationStructureMemoryRequirementsNV(vk.device, &memoryRequirementsInfo, &memoryRequirements2);

	VkBindAccelerationStructureMemoryInfoNV memoryInfo = { 0 };
	memoryInfo.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV;
	memoryInfo.accelerationStructure = bas->accelerationStructure;
	memoryInfo.memoryOffset = vk_d.asBufferOffset;
	memoryInfo.memory = vk_d.asBuffer.memory;
	vk_d.asBufferOffset += (memoryRequirements2.memoryRequirements.size);
	
	VK_CHECK(vkBindAccelerationStructureMemoryNV(vk.device, 1, &memoryInfo), "failed to bind Acceleration Structure Memory NV");
	VK_CHECK(vkGetAccelerationStructureHandleNV(vk.device, bas->accelerationStructure, sizeof(uint64_t), &bas->handle), "failed to get Acceleration Structure Handle NV");
	
	// build
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
		VK_FALSE,
		bas->accelerationStructure,
		VK_NULL_HANDLE,
		vk_d.scratchBuffer.buffer,
		0);

	VK_EndSingleTimeCommands(&commandBuffer);
	
}

void VK_UpdateBottomSingle(vkbottomAS_t* bas) {
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

void VK_MakeTopSingle(vktopAS_t* tas) {
	VkAccelerationStructureInfoNV accelerationStructureInfo = { 0 };
	accelerationStructureInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
	accelerationStructureInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
	accelerationStructureInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV;
	accelerationStructureInfo.instanceCount = vk_d.bottomASCount;
	accelerationStructureInfo.geometryCount = 0;

	VkAccelerationStructureCreateInfoNV accelerationStructureCI = { 0 };
	accelerationStructureCI.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
	accelerationStructureCI.info = accelerationStructureInfo;
	VK_CHECK(vkCreateAccelerationStructureNV(vk.device, &accelerationStructureCI, NULL, &tas->accelerationStructure), "failed to create Bottom Level Acceleration Structure NV");

	VkAccelerationStructureMemoryRequirementsInfoNV memoryRequirementsInfo = { 0 };
	memoryRequirementsInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
	memoryRequirementsInfo.accelerationStructure = tas->accelerationStructure;

	VkMemoryRequirements2 memoryRequirements2 = { 0 };
	vkGetAccelerationStructureMemoryRequirementsNV(vk.device, &memoryRequirementsInfo, &memoryRequirements2);

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
	memoryInfo.memoryOffset = vk_d.asBufferOffset;
	memoryInfo.memory = vk_d.asBuffer.memory;
	vk_d.asBufferOffset += memoryRequirements2.memoryRequirements.size;

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
	buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV;
	buildInfo.pGeometries = 0;
	buildInfo.geometryCount = 0;
	buildInfo.instanceCount = vk_d.bottomASCount;

	uint32_t numIndex = 0;
	for (int i = 0; i < vk_d.bottomASCount; i++) {
		geometryInstance.accelerationStructureHandle = vk_d.bottomASList[i].handle;
		geometryInstance.flags = vk_d.bottomASList[i].flags;
		//geometryInstance.instanceCustomIndex = numIndex;
		//geometryInstance.mask = 0xff;
		//numIndex += bList->bottomASList[i].geometries.geometry.triangles.indexCount;
		VK_UploadBufferDataOffset(&vk_d.instanceBuffer, i * sizeof(VkGeometryInstanceNV), sizeof(VkGeometryInstanceNV), (void*)&geometryInstance);
	}

	VkCommandBuffer commandBuffer = { 0 };
	VK_BeginSingleTimeCommands(&commandBuffer);

	VkMemoryBarrier memoryBarrier = { 0 };
	memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
	memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 0, 1, &memoryBarrier, 0, 0, 0, 0);

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

	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 0, 1, &memoryBarrier, 0, 0, 0, 0);

	VK_EndSingleTimeCommands(&commandBuffer);

}


void VK_UpdateTop(vkbottomASList_t* bList)
{
	VkAccelerationStructureMemoryRequirementsInfoNV memoryRequirementsInfo = { 0 };
	memoryRequirementsInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
	memoryRequirementsInfo.accelerationStructure = bList->topAS.accelerationStructure;

	VkMemoryRequirements2 memoryRequirements2 = { 0 };
	vkGetAccelerationStructureMemoryRequirementsNV(vk.device, &memoryRequirementsInfo, &memoryRequirements2);
	// build
	VkMemoryRequirements2 memReqTopLevelAS;
	memoryRequirementsInfo.accelerationStructure = bList->topAS.accelerationStructure;
	vkGetAccelerationStructureMemoryRequirementsNV(vk.device, &memoryRequirementsInfo, &memReqTopLevelAS);

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
	buildInfo.instanceCount = bList->asCount;

	uint32_t numIndex = 0;
	for (int i = 0; i < bList->asCount; i++) {
		geometryInstance.accelerationStructureHandle = bList->bottomASList[i].handle;
		geometryInstance.flags = vk_d.bottomASList[i].flags;
		//geometryInstance.instanceCustomIndex = numIndex;
		//geometryInstance.mask = 0xff;
		//numIndex += bList->bottomASList[i].geometries.geometry.triangles.indexCount;
		//VK_UploadBufferDataOffset(&vk_d.instanceBuffer, i * sizeof(VkGeometryInstanceNV), sizeof(VkGeometryInstanceNV), (void*)&geometryInstance);
	}

	VkCommandBuffer commandBuffer = { 0 };
	VK_BeginSingleTimeCommands(&commandBuffer);

	VkMemoryBarrier memoryBarrier = { 0 };
	memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
	memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 0, 1, &memoryBarrier, 0, 0, 0, 0);

	vkCmdBuildAccelerationStructureNV(
		commandBuffer,
		&buildInfo,
		vk_d.instanceBuffer.buffer,
		0,
		VK_TRUE,
		bList->topAS.accelerationStructure,
		bList->topAS.accelerationStructure,
		vk_d.scratchBuffer.buffer,
		0);

	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 0, 1, &memoryBarrier, 0, 0, 0, 0);

	VK_EndSingleTimeCommands(&commandBuffer);
}

//
//void VK_MakeBottom(vkbottomASList_t *bList) {
//
//	VkBindAccelerationStructureMemoryInfoNV* memoryInfo = calloc(bList->asCount, sizeof(VkBindAccelerationStructureMemoryInfoNV));
//
//	//VkDeviceSize memorySize = 0;
//	//uint32_t memoryTypBits = 0;
//	for (int i = 0; i < bList->asCount; i++) {
//		VkAccelerationStructureInfoNV accelerationStructureInfo = { 0 };
//		accelerationStructureInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
//		accelerationStructureInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
//		accelerationStructureInfo.instanceCount = 0;
//		accelerationStructureInfo.geometryCount = 1;
//		accelerationStructureInfo.pGeometries = &bList->bottomASList[i].geometries;
//
//		VkAccelerationStructureCreateInfoNV accelerationStructureCI = { 0 };
//		accelerationStructureCI.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
//		accelerationStructureCI.info = accelerationStructureInfo;
//		VK_CHECK(vkCreateAccelerationStructureNV(vk.device, &accelerationStructureCI, NULL, &bList->bottomASList[i].accelerationStructure), "failed to create Bottom Level Acceleration Structure NV");
//
//		// Get Memory Info
//		VkAccelerationStructureMemoryRequirementsInfoNV memoryRequirementsInfo = { 0 };
//		memoryRequirementsInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
//		memoryRequirementsInfo.accelerationStructure = bList->bottomASList[i].accelerationStructure;
//		VkMemoryRequirements2 memoryRequirements2 = { 0 };
//		vkGetAccelerationStructureMemoryRequirementsNV(vk.device, &memoryRequirementsInfo, &memoryRequirements2);
//		
//		memoryInfo[i].sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV;
//		memoryInfo[i].accelerationStructure = bList->bottomASList[i].accelerationStructure;	
//		memoryInfo[i].memoryOffset = vk_d.asBufferOffset;
//		memoryInfo[i].memory = vk_d.asBuffer.memory;
//		vk_d.asBufferOffset += memoryRequirements2.memoryRequirements.size;
//
//		//memorySize += memoryRequirements2.memoryRequirements.size;
//		//memoryTypBits = memoryRequirements2.memoryRequirements.memoryTypeBits;
//	}
//	/*
//	VkMemoryAllocateInfo memoryAllocateInfo = { 0 };
//	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//	memoryAllocateInfo.allocationSize = memorySize;
//	memoryAllocateInfo.memoryTypeIndex = VK_FindMemoryTypeIndex(memoryTypBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
//	VK_CHECK(vkAllocateMemory(vk.device, &memoryAllocateInfo, NULL, &bList->bottomMemory), "failed to Allocate Memory NV");
//	*/
//	/*
//	for (int i = 0; i < bList->asCount; i++) {
//		memoryInfo[i].memory = vk_d.asBuffer.memory;//bList->bottomMemory;
//	}
//	*/
//	VK_CHECK(vkBindAccelerationStructureMemoryNV(vk.device, bList->asCount, memoryInfo), "failed to bind Acceleration Structure Memory NV");
//
//	for (int i = 0; i < bList->asCount; i++) {
//		VK_CHECK(vkGetAccelerationStructureHandleNV(vk.device, bList->bottomASList[i].accelerationStructure, sizeof(uint64_t), &bList->bottomASList[i].handle), "failed to get Acceleration Structure Handle NV");
//	}
//
//	free(memoryInfo);
//
//	// build
//	// create scratch buffer
//	
//	for (int i = 0; i < bList->asCount; i++) {
//		VkCommandBuffer commandBuffer = { 0 };
//		VK_BeginSingleTimeCommands(&commandBuffer);
//
//		VkAccelerationStructureInfoNV buildInfo = { 0 };
//		buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
//		buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
//		//buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV;
//		buildInfo.geometryCount = 1;
//		buildInfo.pGeometries = &bList->bottomASList[i].geometries;
//
//		vkCmdBuildAccelerationStructureNV(
//			commandBuffer,
//			&buildInfo,
//			VK_NULL_HANDLE,
//			0,
//			VK_FALSE,
//			bList->bottomASList[i].accelerationStructure,
//			VK_NULL_HANDLE,
//			vk_d.scratchBuffer.buffer,
//			0);
//
//		VK_EndSingleTimeCommands(&commandBuffer);
//	}
//	
//}
//
//void VK_MakeTop(vkbottomASList_t* bList) {
//	VkAccelerationStructureInfoNV accelerationStructureInfo = { 0 };
//	accelerationStructureInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
//	accelerationStructureInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
//	accelerationStructureInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV;
//	accelerationStructureInfo.instanceCount = bList->asCount;
//	accelerationStructureInfo.geometryCount = 0;
//
//	VkAccelerationStructureCreateInfoNV accelerationStructureCI = { 0 };
//	accelerationStructureCI.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
//	accelerationStructureCI.info = accelerationStructureInfo;
//	VK_CHECK(vkCreateAccelerationStructureNV(vk.device, &accelerationStructureCI, NULL, &bList->topAS.accelerationStructure), "failed to create Bottom Level Acceleration Structure NV");
//
//	VkAccelerationStructureMemoryRequirementsInfoNV memoryRequirementsInfo = { 0 };
//	memoryRequirementsInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
//	memoryRequirementsInfo.accelerationStructure = bList->topAS.accelerationStructure;
//
//	VkMemoryRequirements2 memoryRequirements2 = { 0 };
//	vkGetAccelerationStructureMemoryRequirementsNV(vk.device, &memoryRequirementsInfo, &memoryRequirements2);
//	
//	/*
//	VkMemoryAllocateInfo memoryAllocateInfo = { 0 };
//	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//	memoryAllocateInfo.allocationSize = memoryRequirements2.memoryRequirements.size;
//	memoryAllocateInfo.memoryTypeIndex = VK_FindMemoryTypeIndex(memoryRequirements2.memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
//	VK_CHECK(vkAllocateMemory(vk.device, &memoryAllocateInfo, NULL, &bList->topMemory), "failed to Allocate Memory NV");
//	*/
//
//	VkBindAccelerationStructureMemoryInfoNV memoryInfo = { 0 };
//	memoryInfo.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV;
//	memoryInfo.accelerationStructure = bList->topAS.accelerationStructure;
//	memoryInfo.memoryOffset = vk_d.asBufferOffset;
//	memoryInfo.memory = vk_d.asBuffer.memory;
//	vk_d.asBufferOffset += memoryRequirements2.memoryRequirements.size;
//
//	VK_CHECK(vkBindAccelerationStructureMemoryNV(vk.device, 1, &memoryInfo), "failed to bind Acceleration Structure Memory NV");
//	VK_CHECK(vkGetAccelerationStructureHandleNV(vk.device, bList->topAS.accelerationStructure, sizeof(uint64_t), &bList->topAS.handle), "failed to get Acceleration Structure Handle NV");
//
//	// build
//	//VkMemoryRequirements2 memReqTopLevelAS;
//	//memoryRequirementsInfo.accelerationStructure = bList->topAS.accelerationStructure;
//	//vkGetAccelerationStructureMemoryRequirementsNV(vk.device, &memoryRequirementsInfo, &memReqTopLevelAS);
//	
//	// fill instance buffer
//	VkGeometryInstanceNV geometryInstance = { 0 };
//	float transform[12] = {
//			1.0f, 0.0f, 0.0f, 0.0f,
//			0.0f, 1.0f, 0.0f, 0.0f,
//			0.0f, 0.0f, 1.0f, 0.0f
//	};
//	Com_Memcpy(&geometryInstance.transform, &transform, sizeof(float[12]));
//	geometryInstance.instanceCustomIndex = 0;
//	geometryInstance.mask = 0xff;
//	geometryInstance.instanceOffset = 0;
//	geometryInstance.flags = 0;//VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV;
//	
//
//	VkAccelerationStructureInfoNV buildInfo = { 0 };
//	buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
//	buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
//	buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV;
//	buildInfo.pGeometries = 0;
//	buildInfo.geometryCount = 0;
//	buildInfo.instanceCount = bList->asCount;
//
//	uint32_t numIndex = 0;
//	for (int i = 0; i < bList->asCount; i++) {
//		geometryInstance.accelerationStructureHandle = bList->bottomASList[i].handle;
//		//geometryInstance.instanceCustomIndex = numIndex;
//		//geometryInstance.mask = 0xff;
//		//numIndex += bList->bottomASList[i].geometries.geometry.triangles.indexCount;
//		VK_UploadBufferDataOffset(&vk_d.instanceBuffer, i * sizeof(VkGeometryInstanceNV), sizeof(VkGeometryInstanceNV), (void*)&geometryInstance);
//	}
//
//	VkCommandBuffer commandBuffer = { 0 };
//	VK_BeginSingleTimeCommands(&commandBuffer);
// 
//	VkMemoryBarrier memoryBarrier = { 0 };
//	memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
//	memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
//	memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
//	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 0, 1, &memoryBarrier, 0, 0, 0, 0);
//
//	vkCmdBuildAccelerationStructureNV(
//		commandBuffer,
//		&buildInfo,
//		vk_d.instanceBuffer.buffer,
//		0,
//		VK_FALSE,
//		bList->topAS.accelerationStructure,
//		VK_NULL_HANDLE,
//		vk_d.scratchBuffer.buffer,
//		0);
//	
//	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 0, 1, &memoryBarrier, 0, 0, 0, 0);
//
//	VK_EndSingleTimeCommands(&commandBuffer);
//
//}


//void VK_UploadScene(vkaccelerationStructures_t* as, vkgeometry_t* g) {
//	
//
//	// create instance buffer
//	/*vkbuffer_t transformBuffer = { 0 };
//	float transform[24] = {
//			1.0f, 0.0f, 0.0f, 0.0f,
//			0.0f, 1.0f, 0.0f, 0.0f,
//			0.0f, 0.0f, 1.0f, 0.0f,
//
//			1.0f, 0.0f, 0.0f, 0.3f,
//			0.0f, 1.0f, 0.0f, 0.0f,
//			0.0f, 0.0f, 1.0f, 0.0f
//	};
//	VK_CreateRayTracingBuffer(&transformBuffer, sizeof(transform));
//	VK_UploadBufferData(&transformBuffer, (void*)& transform);*/
//	
//	VkGeometryNV* geometrys = calloc(g->numSurfaces, sizeof(VkGeometryNV));
//
//	uint32_t offsetIDX = 0;
//	uint32_t offsetXYZ = 0;
//	for (int i = 0; i < g->numSurfaces; i++) {
//		geometrys[i].sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
//		geometrys[i].geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
//		geometrys[i].geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
//		geometrys[i].geometry.triangles.vertexData = g->xyz.buffer;
//		geometrys[i].geometry.triangles.vertexOffset = 0;//offsetXYZ * 8 * sizeof(float);
//		geometrys[i].geometry.triangles.vertexCount = g->sizeXYZ[i];
//		geometrys[i].geometry.triangles.vertexStride = 12 * sizeof(float);
//		geometrys[i].geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
//		geometrys[i].geometry.triangles.indexData = g->idx.buffer;
//		geometrys[i].geometry.triangles.indexOffset = offsetIDX * sizeof(uint32_t);
//		geometrys[i].geometry.triangles.indexCount = g->sizeIDX[i];
//		geometrys[i].geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
//		geometrys[i].geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
//		//geometrys[i].flags = VK_GEOMETRY_OPAQUE_BIT_NV;
//
//		offsetIDX += g->sizeIDX[i];
//		offsetXYZ += g->sizeXYZ[i];
//	}
//
//	geometrys[0].sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
//	geometrys[0].geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
//	geometrys[0].geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
//	geometrys[0].geometry.triangles.vertexData = g->xyz.buffer;
//	geometrys[0].geometry.triangles.vertexOffset = 0;//offsetXYZ * 8 * sizeof(float);
//	geometrys[0].geometry.triangles.vertexCount = offsetXYZ;
//	geometrys[0].geometry.triangles.vertexStride = 12 * sizeof(float);
//	geometrys[0].geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
//	geometrys[0].geometry.triangles.indexData = g->idx.buffer;
//	geometrys[0].geometry.triangles.indexOffset = 0;//offsetIDX * sizeof(uint32_t);
//	geometrys[0].geometry.triangles.indexCount = offsetIDX;
//	geometrys[0].geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
//	geometrys[0].geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
//	//geometrys[0].flags = VK_GEOMETRY_OPAQUE_BIT_NV;
//	
//	as->bottom.geometryCount = 1;
//	// create bottom as
//	VK_CreateBottomLevelAccelerationStructure(as, &geometrys[0]);
//	// create top as
//	VK_CreateTopLevelAccelerationStructure(as);
//	// build as
//	VK_BuildAccelerationStructure(as, &geometrys[0]);
//
//	//VK_DestroyBuffer(&transformBuffer);
//	//VK_DestroyBuffer(&vBuffer);
//	//VK_DestroyBuffer(&iBuffer);
//
//	free(geometrys);
//
//	as->init = qtrue;
//
//}
//
//
//static void VK_CreateBottomLevelAccelerationStructure(vkaccelerationStructures_t *as, const VkGeometryNV* geometries)
//{
//	VkAccelerationStructureInfoNV accelerationStructureInfo = { 0 };
//	accelerationStructureInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
//	accelerationStructureInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
//	accelerationStructureInfo.instanceCount = 0;
//	accelerationStructureInfo.geometryCount = as->bottom.geometryCount;
//	accelerationStructureInfo.pGeometries = geometries;
//
//	VkAccelerationStructureCreateInfoNV accelerationStructureCI = { 0 };
//	accelerationStructureCI.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
//	accelerationStructureCI.info = accelerationStructureInfo;
//	VK_CHECK(vkCreateAccelerationStructureNV(vk.device, &accelerationStructureCI, NULL, &as->bottom.accelerationStructure), "failed to create Bottom Level Acceleration Structure NV");
//
//	VkAccelerationStructureMemoryRequirementsInfoNV memoryRequirementsInfo = { 0 };
//	memoryRequirementsInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
//	memoryRequirementsInfo.accelerationStructure = as->bottom.accelerationStructure;
//
//	VkMemoryRequirements2 memoryRequirements2 = { 0 };
//	vkGetAccelerationStructureMemoryRequirementsNV(vk.device, &memoryRequirementsInfo, &memoryRequirements2);
//
//	VkMemoryAllocateInfo memoryAllocateInfo = { 0 };
//	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//	memoryAllocateInfo.allocationSize = memoryRequirements2.memoryRequirements.size;
//	memoryAllocateInfo.memoryTypeIndex = VK_FindMemoryTypeIndex(memoryRequirements2.memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
//	VK_CHECK(vkAllocateMemory(vk.device, &memoryAllocateInfo, NULL, &as->bottom.memory), "failed to Allocate Memory NV");
//
//	VkBindAccelerationStructureMemoryInfoNV accelerationStructureMemoryInfo = { 0 };
//	accelerationStructureMemoryInfo.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV;
//	accelerationStructureMemoryInfo.accelerationStructure = as->bottom.accelerationStructure;
//	accelerationStructureMemoryInfo.memory = as->bottom.memory;
//	VK_CHECK(vkBindAccelerationStructureMemoryNV(vk.device, 1, &accelerationStructureMemoryInfo), "failed to bind Acceleration Structure Memory NV");
//
//	VK_CHECK(vkGetAccelerationStructureHandleNV(vk.device, as->bottom.accelerationStructure, sizeof(uint64_t), &as->bottom.handle), "failed to get Acceleration Structure Handle NV");
//}
//
//static void VK_CreateTopLevelAccelerationStructure(vkaccelerationStructures_t* as)
//{
//	// create top as
//	VkAccelerationStructureInfoNV accelerationStructureInfo = { 0 };
//	accelerationStructureInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
//	accelerationStructureInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
//	accelerationStructureInfo.instanceCount = 1;
//	accelerationStructureInfo.geometryCount = 0;
//
//	VkAccelerationStructureCreateInfoNV accelerationStructureCI = { 0 };
//	accelerationStructureCI.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
//	accelerationStructureCI.info = accelerationStructureInfo;
//	VK_CHECK(vkCreateAccelerationStructureNV(vk.device, &accelerationStructureCI, NULL, &as->top.accelerationStructure), "failed to create Bottom Level Acceleration Structure NV");
//
//	VkAccelerationStructureMemoryRequirementsInfoNV memoryRequirementsInfo = { 0 };
//	memoryRequirementsInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
//	memoryRequirementsInfo.accelerationStructure = as->top.accelerationStructure;
//
//	VkMemoryRequirements2 memoryRequirements2 = { 0 };
//	vkGetAccelerationStructureMemoryRequirementsNV(vk.device, &memoryRequirementsInfo, &memoryRequirements2);
//
//	VkMemoryAllocateInfo memoryAllocateInfo = { 0 };
//	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//	memoryAllocateInfo.allocationSize = memoryRequirements2.memoryRequirements.size;
//	memoryAllocateInfo.memoryTypeIndex = VK_FindMemoryTypeIndex(memoryRequirements2.memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
//	VK_CHECK(vkAllocateMemory(vk.device, &memoryAllocateInfo, NULL, &as->top.memory), "failed to Allocate Memory NV");
//
//	VkBindAccelerationStructureMemoryInfoNV accelerationStructureMemoryInfo = { 0 };
//	accelerationStructureMemoryInfo.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV;
//	accelerationStructureMemoryInfo.accelerationStructure = as->top.accelerationStructure;
//	accelerationStructureMemoryInfo.memory = as->top.memory;
//	VK_CHECK(vkBindAccelerationStructureMemoryNV(vk.device, 1, &accelerationStructureMemoryInfo), "failed to bind Acceleration Structure Memory NV");
//
//	VK_CHECK(vkGetAccelerationStructureHandleNV(vk.device, as->top.accelerationStructure, sizeof(uint64_t), &as->top.handle), "failed to get Acceleration Structure Handle NV");
//
//}
//
//static void VK_BuildAccelerationStructure(vkaccelerationStructures_t* as, const VkGeometryNV* geometries) {
//	// create scratch buffer
//	vkbuffer_t scratchBuffer = { 0 };
//
//	VkAccelerationStructureMemoryRequirementsInfoNV memoryRequirementsInfo = { 0 };
//	memoryRequirementsInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
//	memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV;
//
//	VkMemoryRequirements2 memReqBottomLevelAS;
//	memoryRequirementsInfo.accelerationStructure = as->bottom.accelerationStructure;
//	vkGetAccelerationStructureMemoryRequirementsNV(vk.device, &memoryRequirementsInfo, &memReqBottomLevelAS);
//	VkMemoryRequirements2 memReqTopLevelAS;
//	memoryRequirementsInfo.accelerationStructure = as->top.accelerationStructure;
//	vkGetAccelerationStructureMemoryRequirementsNV(vk.device, &memoryRequirementsInfo, &memReqTopLevelAS);
//
//	const VkDeviceSize scratchBufferSize = max(memReqBottomLevelAS.memoryRequirements.size, memReqTopLevelAS.memoryRequirements.size);
//	VK_CreateRayTracingScratchBuffer(&scratchBuffer, scratchBufferSize);
//
//	// create instance buffer
//	vkbuffer_t instanceBuffer = { 0 };
//	float transform[12] = {
//			1.0f, 0.0f, 0.0f, 0.0f,
//			0.0f, 1.0f, 0.0f, 0.0f,
//			0.0f, 0.0f, 1.0f, 0.0f
//	};
//
//	VkGeometryInstanceNV geometryInstance = { 0 };
//	Com_Memcpy(&geometryInstance.transform, &transform, sizeof(float[12]));
//	geometryInstance.instanceCustomIndex = 0;
//	geometryInstance.mask = 0xff;
//	geometryInstance.instanceOffset = 0;
//	geometryInstance.flags;//VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV;
//	geometryInstance.accelerationStructureHandle = as->bottom.handle;
//
//	VK_CreateRayTracingBuffer(&instanceBuffer, sizeof(geometryInstance));
//	VK_UploadBufferData(&instanceBuffer, (void*)& geometryInstance);
//
//	// record build cmds
//	VkCommandBuffer commandBuffer = { 0 };
//	VK_BeginSingleTimeCommands(&commandBuffer);
//
//	VkAccelerationStructureInfoNV buildInfo = { 0 };
//	buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
//	buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
//	buildInfo.geometryCount = as->bottom.geometryCount;
//	buildInfo.pGeometries = geometries;
//
//	vkCmdBuildAccelerationStructureNV(
//		commandBuffer,
//		&buildInfo,
//		VK_NULL_HANDLE,
//		0,
//		VK_FALSE,
//		as->bottom.accelerationStructure,
//		VK_NULL_HANDLE,
//		scratchBuffer.buffer,
//		0);
//
//	VkMemoryBarrier memoryBarrier = { 0 };
//	memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
//	memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
//	memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
//	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 0, 1, &memoryBarrier, 0, 0, 0, 0);
//
//	/*
//		Build top-level acceleration structure
//	*/
//	buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
//	buildInfo.pGeometries = 0;
//	buildInfo.geometryCount = 0;
//	buildInfo.instanceCount = 1;
//
//	vkCmdBuildAccelerationStructureNV(
//		commandBuffer,
//		&buildInfo,
//		instanceBuffer.buffer,
//		0,
//		VK_FALSE,
//		as->top.accelerationStructure,
//		VK_NULL_HANDLE,
//		scratchBuffer.buffer,
//		0);
//
//	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 0, 1, &memoryBarrier, 0, 0, 0, 0);
//
//	VK_EndSingleTimeCommands(&commandBuffer);
//
//	// destroy scratch Buffer
//	VK_DestroyBuffer(&scratchBuffer);
//	// destroy instance buffer
//	VK_DestroyBuffer(&instanceBuffer);
//}

void VK_DestroyAccelerationStructure2(vkbottomASList_t* as) {
	if (as->bottomMemory != VK_NULL_HANDLE) {
		vkFreeMemory(vk.device, as->bottomMemory, NULL);
		as->bottomMemory = VK_NULL_HANDLE;
	}
	if (as->topMemory != VK_NULL_HANDLE) {
		vkFreeMemory(vk.device, as->topMemory, NULL);
		as->topMemory = VK_NULL_HANDLE;
	}

	for (int i = 0; i < as->asCount; i++) {
		if (as->bottomASList[i].accelerationStructure != VK_NULL_HANDLE) {
			vkDestroyAccelerationStructureNV(vk.device, as->bottomASList[i].accelerationStructure, NULL);
			as->bottomASList[i].accelerationStructure = VK_NULL_HANDLE;
		}
	}


	if (as->topAS.accelerationStructure != VK_NULL_HANDLE) {
		vkDestroyAccelerationStructureNV(vk.device, as->topAS.accelerationStructure, NULL);
		as->topAS.accelerationStructure = VK_NULL_HANDLE;
	}

	free(as->bottomASList);
	as->asCount = 0;
}

void VK_DestroyAccelerationStructure(vkaccelerationStructures_t* as) {
	if (as->bottom.memory != VK_NULL_HANDLE) {
		vkFreeMemory(vk.device, as->bottom.memory, NULL);
		as->bottom.memory = VK_NULL_HANDLE;
	}
	if (as->top.memory != VK_NULL_HANDLE) {
		vkFreeMemory(vk.device, as->top.memory, NULL);
		as->top.memory = VK_NULL_HANDLE;
	}

	if (as->bottom.accelerationStructure != VK_NULL_HANDLE) {
		vkDestroyAccelerationStructureNV(vk.device, as->bottom.accelerationStructure, NULL);
		as->bottom.accelerationStructure = VK_NULL_HANDLE;
	}
	if (as->top.accelerationStructure != VK_NULL_HANDLE) {
		vkDestroyAccelerationStructureNV(vk.device, as->top.accelerationStructure, NULL);
		as->top.accelerationStructure = VK_NULL_HANDLE;
	}

	
}