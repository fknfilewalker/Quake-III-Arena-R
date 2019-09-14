#include "../tr_local.h"

#define max(a,b) (((a)>(b))?(a):(b))

static void VK_CreateBottomLevelAccelerationStructure(vkaccelerationStructures_t* as, const VkGeometryNV* geometries);
static void VK_CreateTopLevelAccelerationStructure(vkaccelerationStructures_t* as);
static void VK_BuildAccelerationStructure(vkaccelerationStructures_t* as, const VkGeometryNV* geometries);

typedef struct {
	float          transform[12];
	uint32_t       instanceCustomIndex : 24;
	uint32_t       mask : 8;
	uint32_t       instanceOffset : 24;
	uint32_t       flags : 8;
	uint64_t       accelerationStructureHandle;
} VkGeometryInstanceNV;

void VK_UploadScene(vkaccelerationStructures_t* as, vkgeometry_t* g) {
	

	// create instance buffer
	/*vkbuffer_t transformBuffer = { 0 };
	float transform[24] = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,

			1.0f, 0.0f, 0.0f, 0.3f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f
	};
	VK_CreateRayTracingBuffer(&transformBuffer, sizeof(transform));
	VK_UploadBufferData(&transformBuffer, (void*)& transform);*/
	
	VkGeometryNV* geometrys = calloc(g->numSurfaces, sizeof(VkGeometryNV));

	uint32_t offsetIDX = 0;
	uint32_t offsetXYZ = 0;
	for (int i = 0; i < g->numSurfaces; i++) {
		geometrys[i].sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
		geometrys[i].geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
		geometrys[i].geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
		geometrys[i].geometry.triangles.vertexData = g->xyz.buffer;
		geometrys[i].geometry.triangles.vertexOffset = offsetXYZ * sizeof(vec3_t);;
		geometrys[i].geometry.triangles.vertexCount = 0;//g->sizeXYZ[i];
		geometrys[i].geometry.triangles.vertexStride = 3 * sizeof(float);
		geometrys[i].geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
		geometrys[i].geometry.triangles.indexData = g->idx.buffer;
		geometrys[i].geometry.triangles.indexOffset = offsetIDX * sizeof(uint32_t);
		geometrys[i].geometry.triangles.indexCount = g->sizeIDX[i];
		geometrys[i].geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
		geometrys[i].geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
		geometrys[i].flags = VK_GEOMETRY_OPAQUE_BIT_NV;

		offsetIDX += g->sizeIDX[i];
		offsetXYZ += g->sizeXYZ[i];
	}

	
	as->bottom.geometryCount = g->numSurfaces;
	// create bottom as
	VK_CreateBottomLevelAccelerationStructure(as, &geometrys[0]);
	// create top as
	VK_CreateTopLevelAccelerationStructure(as);
	// build as
	VK_BuildAccelerationStructure(as, &geometrys[0]);

	//VK_DestroyBuffer(&transformBuffer);
	//VK_DestroyBuffer(&vBuffer);
	//VK_DestroyBuffer(&iBuffer);

	free(geometrys);

	as->init = qtrue;

}


void VK_UploadScene2(vkaccelerationStructures_t* as) {

	struct Vertex {
		float pos[3];
	};
	vkbuffer_t vBuffer = { 0 };
	vkbuffer_t iBuffer = { 0 };
	
	float vertices[9] = {
		0.5f,  0.5f, 0.0f,
		-0.5f,  0.5f, 0.0f,
		0.0f, -0.5f, 0.0f
	};

	// Setup indices
	uint32_t indices[3] = {0, 1, 2 };

	VK_CreateVertexBuffer(&vBuffer, sizeof(vertices));
	VK_CreateIndexBuffer(&iBuffer, sizeof(indices));
	VK_UploadBufferData(&vBuffer, (void*) &vertices);
	VK_UploadBufferData(&iBuffer, (void*) &indices);

	// create instance buffer
	vkbuffer_t transformBuffer = { 0 };
	float transform[24] = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,

			1.0f, 0.0f, 0.0f, 0.3f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f
	};
	VK_CreateRayTracingBuffer(&transformBuffer, sizeof(transform));
	VK_UploadBufferData(&transformBuffer, (void*)& transform);

	VkGeometryNV geometry = { 0 };
	geometry.sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
	geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
	geometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
	geometry.geometry.triangles.vertexData = vBuffer.buffer;
	geometry.geometry.triangles.vertexOffset = 0;
	geometry.geometry.triangles.vertexCount = 3;
	geometry.geometry.triangles.vertexStride = 3 * sizeof(float);
	geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
	geometry.geometry.triangles.indexData = iBuffer.buffer;
	geometry.geometry.triangles.indexOffset = 0;
	geometry.geometry.triangles.indexCount = 3;
	geometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
	geometry.geometry.triangles.transformData = transformBuffer.buffer;
	geometry.geometry.triangles.transformOffset = 0;
//	geometry.geometry.aabbs = { 0 };
	geometry.geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
	geometry.flags = VK_GEOMETRY_OPAQUE_BIT_NV;

	VkGeometryNV geometrys[2] = {0};
	geometrys[0] = geometrys[1] = geometry;
	geometrys[1].geometry.triangles.transformOffset = 12 * sizeof(float);

	as->bottom.geometryCount = 2;
	// create bottom as
	VK_CreateBottomLevelAccelerationStructure(as, &geometrys[0]);
	// create top as
	VK_CreateTopLevelAccelerationStructure(as);
	// build as
	VK_BuildAccelerationStructure(as, &geometrys[0]);

	VK_DestroyBuffer(&transformBuffer);
	VK_DestroyBuffer(&vBuffer);
	VK_DestroyBuffer(&iBuffer);

	as->init = qtrue;

}

static void VK_CreateBottomLevelAccelerationStructure(vkaccelerationStructures_t *as, const VkGeometryNV* geometries)
{
	VkAccelerationStructureInfoNV accelerationStructureInfo = { 0 };
	accelerationStructureInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
	accelerationStructureInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
	accelerationStructureInfo.instanceCount = 0;
	accelerationStructureInfo.geometryCount = as->bottom.geometryCount;
	accelerationStructureInfo.pGeometries = geometries;

	VkAccelerationStructureCreateInfoNV accelerationStructureCI = { 0 };
	accelerationStructureCI.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
	accelerationStructureCI.info = accelerationStructureInfo;
	VK_CHECK(vkCreateAccelerationStructureNV(vk.device, &accelerationStructureCI, NULL, &as->bottom.accelerationStructure), "failed to create Bottom Level Acceleration Structure NV");

	VkAccelerationStructureMemoryRequirementsInfoNV memoryRequirementsInfo = { 0 };
	memoryRequirementsInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
	memoryRequirementsInfo.accelerationStructure = as->bottom.accelerationStructure;

	VkMemoryRequirements2 memoryRequirements2 = { 0 };
	vkGetAccelerationStructureMemoryRequirementsNV(vk.device, &memoryRequirementsInfo, &memoryRequirements2);

	VkMemoryAllocateInfo memoryAllocateInfo = { 0 };
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements2.memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = VK_FindMemoryTypeIndex(memoryRequirements2.memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK(vkAllocateMemory(vk.device, &memoryAllocateInfo, NULL, &as->bottom.memory), "failed to Allocate Memory NV");

	VkBindAccelerationStructureMemoryInfoNV accelerationStructureMemoryInfo = { 0 };
	accelerationStructureMemoryInfo.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV;
	accelerationStructureMemoryInfo.accelerationStructure = as->bottom.accelerationStructure;
	accelerationStructureMemoryInfo.memory = as->bottom.memory;
	VK_CHECK(vkBindAccelerationStructureMemoryNV(vk.device, 1, &accelerationStructureMemoryInfo), "failed to bind Acceleration Structure Memory NV");

	VK_CHECK(vkGetAccelerationStructureHandleNV(vk.device, as->bottom.accelerationStructure, sizeof(uint64_t), &as->bottom.handle), "failed to get Acceleration Structure Handle NV");
}

static void VK_CreateTopLevelAccelerationStructure(vkaccelerationStructures_t* as)
{
	// create top as
	VkAccelerationStructureInfoNV accelerationStructureInfo = { 0 };
	accelerationStructureInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
	accelerationStructureInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
	accelerationStructureInfo.instanceCount = 1;
	accelerationStructureInfo.geometryCount = 0;

	VkAccelerationStructureCreateInfoNV accelerationStructureCI = { 0 };
	accelerationStructureCI.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
	accelerationStructureCI.info = accelerationStructureInfo;
	VK_CHECK(vkCreateAccelerationStructureNV(vk.device, &accelerationStructureCI, NULL, &as->top.accelerationStructure), "failed to create Bottom Level Acceleration Structure NV");

	VkAccelerationStructureMemoryRequirementsInfoNV memoryRequirementsInfo = { 0 };
	memoryRequirementsInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
	memoryRequirementsInfo.accelerationStructure = as->top.accelerationStructure;

	VkMemoryRequirements2 memoryRequirements2 = { 0 };
	vkGetAccelerationStructureMemoryRequirementsNV(vk.device, &memoryRequirementsInfo, &memoryRequirements2);

	VkMemoryAllocateInfo memoryAllocateInfo = { 0 };
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements2.memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = VK_FindMemoryTypeIndex(memoryRequirements2.memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK(vkAllocateMemory(vk.device, &memoryAllocateInfo, NULL, &as->top.memory), "failed to Allocate Memory NV");

	VkBindAccelerationStructureMemoryInfoNV accelerationStructureMemoryInfo = { 0 };
	accelerationStructureMemoryInfo.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV;
	accelerationStructureMemoryInfo.accelerationStructure = as->top.accelerationStructure;
	accelerationStructureMemoryInfo.memory = as->top.memory;
	VK_CHECK(vkBindAccelerationStructureMemoryNV(vk.device, 1, &accelerationStructureMemoryInfo), "failed to bind Acceleration Structure Memory NV");

	VK_CHECK(vkGetAccelerationStructureHandleNV(vk.device, as->top.accelerationStructure, sizeof(uint64_t), &as->top.handle), "failed to get Acceleration Structure Handle NV");

}

static void VK_BuildAccelerationStructure(vkaccelerationStructures_t* as, const VkGeometryNV* geometries) {
	// create scratch buffer
	vkbuffer_t scratchBuffer = { 0 };

	VkAccelerationStructureMemoryRequirementsInfoNV memoryRequirementsInfo = { 0 };
	memoryRequirementsInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
	memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV;

	VkMemoryRequirements2 memReqBottomLevelAS;
	memoryRequirementsInfo.accelerationStructure = as->bottom.accelerationStructure;
	vkGetAccelerationStructureMemoryRequirementsNV(vk.device, &memoryRequirementsInfo, &memReqBottomLevelAS);
	VkMemoryRequirements2 memReqTopLevelAS;
	memoryRequirementsInfo.accelerationStructure = as->top.accelerationStructure;
	vkGetAccelerationStructureMemoryRequirementsNV(vk.device, &memoryRequirementsInfo, &memReqTopLevelAS);

	const VkDeviceSize scratchBufferSize = max(memReqBottomLevelAS.memoryRequirements.size, memReqTopLevelAS.memoryRequirements.size);
	VK_CreateRayTracingScratchBuffer(&scratchBuffer, scratchBufferSize);

	// create instance buffer
	vkbuffer_t instanceBuffer = { 0 };
	float transform[12] = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f
	};

	VkGeometryInstanceNV geometryInstance = { 0 };
	Com_Memcpy(&geometryInstance.transform, &transform, sizeof(float[12]));
	geometryInstance.instanceCustomIndex = 0;
	geometryInstance.mask = 0xff;
	geometryInstance.instanceOffset = 0;
	geometryInstance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV;
	geometryInstance.accelerationStructureHandle = as->bottom.handle;

	VK_CreateRayTracingBuffer(&instanceBuffer, sizeof(geometryInstance));
	VK_UploadBufferData(&instanceBuffer, (void*)& geometryInstance);

	// record build cmds
	VkCommandBuffer commandBuffer = { 0 };
	VK_BeginSingleTimeCommands(&commandBuffer);

	VkAccelerationStructureInfoNV buildInfo = { 0 };
	buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
	buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
	buildInfo.geometryCount = as->bottom.geometryCount;
	buildInfo.pGeometries = geometries;

	vkCmdBuildAccelerationStructureNV(
		commandBuffer,
		&buildInfo,
		VK_NULL_HANDLE,
		0,
		VK_FALSE,
		as->bottom.accelerationStructure,
		VK_NULL_HANDLE,
		scratchBuffer.buffer,
		0);

	VkMemoryBarrier memoryBarrier = { 0 };
	memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
	memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 0, 1, &memoryBarrier, 0, 0, 0, 0);

	/*
		Build top-level acceleration structure
	*/
	buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
	buildInfo.pGeometries = 0;
	buildInfo.geometryCount = 0;
	buildInfo.instanceCount = 1;

	vkCmdBuildAccelerationStructureNV(
		commandBuffer,
		&buildInfo,
		instanceBuffer.buffer,
		0,
		VK_FALSE,
		as->top.accelerationStructure,
		VK_NULL_HANDLE,
		scratchBuffer.buffer,
		0);

	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 0, 1, &memoryBarrier, 0, 0, 0, 0);

	VK_EndSingleTimeCommands(&commandBuffer);

	// destroy scratch Buffer
	VK_DestroyBuffer(&scratchBuffer);
	// destroy instance buffer
	VK_DestroyBuffer(&instanceBuffer);
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