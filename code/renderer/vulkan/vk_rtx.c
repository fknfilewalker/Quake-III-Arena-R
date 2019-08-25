#include "../tr_local.h"

void VK_CreateBottomLevelAccelerationStructure(const VkGeometryNV* geometries)
{
	VkAccelerationStructureInfoNV accelerationStructureInfo = { 0 };
	accelerationStructureInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
	accelerationStructureInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
	accelerationStructureInfo.instanceCount = 0;
	accelerationStructureInfo.geometryCount = 1;
	accelerationStructureInfo.pGeometries = geometries;

	VkAccelerationStructureCreateInfoNV accelerationStructureCI = { 0 };
	accelerationStructureCI.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
	accelerationStructureCI.info = accelerationStructureInfo;
	VK_CHECK(vkCreateAccelerationStructureNV(vk.device, &accelerationStructureCI, NULL, &vk_d.bottomLevelAS.accelerationStructure), "failed to create Bottom Level Acceleration Structure NV");

	VkAccelerationStructureMemoryRequirementsInfoNV memoryRequirementsInfo = { 0 };
	memoryRequirementsInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
	memoryRequirementsInfo.accelerationStructure = &vk_d.bottomLevelAS.accelerationStructure;

	VkMemoryRequirements2 memoryRequirements2 = { 0 };
	vkGetAccelerationStructureMemoryRequirementsNV(vk.device, &memoryRequirementsInfo, &memoryRequirements2);

	VkMemoryAllocateInfo memoryAllocateInfo = { 0 };
	memoryAllocateInfo.allocationSize = memoryRequirements2.memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = VK_FindMemoryTypeIndex(memoryRequirements2.memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK(vkAllocateMemory(vk.device, &memoryAllocateInfo, NULL, &vk_d.bottomLevelAS.memory), "failed to Allocate Memory NV");

	VkBindAccelerationStructureMemoryInfoNV accelerationStructureMemoryInfo = { 0 };
	accelerationStructureMemoryInfo.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV;
	accelerationStructureMemoryInfo.accelerationStructure = &vk_d.bottomLevelAS.accelerationStructure;
	accelerationStructureMemoryInfo.memory = vk_d.bottomLevelAS.memory;
	VK_CHECK(vkBindAccelerationStructureMemoryNV(vk.device, 1, &accelerationStructureMemoryInfo), "failed to bind Acceleration Structure Memory NV");

	VK_CHECK(vkGetAccelerationStructureHandleNV(vk.device, vk_d.bottomLevelAS.accelerationStructure, sizeof(uint64_t), &vk_d.bottomLevelAS.handle), "failed to get Acceleration Structure Handle NV");
}
