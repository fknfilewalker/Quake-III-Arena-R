#include "../tr_local.h"

void VK_CreateAttributeBuffer(vkbuffer_t* buffer, VkDeviceSize allocSize, VkBufferUsageFlagBits usage) {
	buffer->allocSize = allocSize;
	VK_CreateBufferMemory(allocSize, usage, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &buffer->buffer, &buffer->memory);
	VK_CHECK(vkMapMemory(vk.device, buffer->memory, 0, buffer->allocSize, 0, (byte * *)(&buffer->p)), "failed to Map Memory!");
}

void VK_CreateVertexBuffer(vkbuffer_t *buffer, VkDeviceSize allocSize){
    buffer->allocSize = allocSize;
	VK_CreateBufferMemory(allocSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &buffer->buffer, &buffer->memory);
    VK_CHECK(vkMapMemory(vk.device, buffer->memory, 0, buffer->allocSize, 0, (byte**)(&buffer->p)), "failed to Map Memory!");
}

void VK_CreateIndexBuffer(vkbuffer_t *buffer, VkDeviceSize allocSize){
    buffer->allocSize = allocSize;
	VK_CreateBufferMemory(allocSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &buffer->buffer, &buffer->memory);
    VK_CHECK(vkMapMemory(vk.device, buffer->memory, 0, buffer->allocSize, 0, (byte**)(&buffer->p)), "failed to Map Memory!");
}

void VK_CreateUniformBuffer(vkbuffer_t* buffer, VkDeviceSize allocSize) {
	buffer->allocSize = allocSize;
	VK_CreateBufferMemory(allocSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &buffer->buffer, &buffer->memory);
	VK_CHECK(vkMapMemory(vk.device, buffer->memory, 0, buffer->allocSize, 0, (byte * *)(&buffer->p)), "failed to Map Memory!");
}

// RTX
void VK_CreateRayTracingASBuffer(vkbuffer_t* buffer, VkDeviceSize allocSize) {
	buffer->allocSize = allocSize;
	VK_CreateBufferMemory(allocSize, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &buffer->buffer, &buffer->memory);
	buffer->onGpu = VK_TRUE;
}
void VK_CreateRayTracingBuffer(vkbuffer_t* buffer, VkDeviceSize allocSize) {
	buffer->allocSize = allocSize;
	VK_CreateBufferMemory(allocSize, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &buffer->buffer, &buffer->memory);
	VK_CHECK(vkMapMemory(vk.device, buffer->memory, 0, buffer->allocSize, 0, (byte * *)(&buffer->p)), "failed to Map Memory!");
}
void VK_CreateRayTracingScratchBuffer(vkbuffer_t* buffer, VkDeviceSize allocSize) {
	buffer->allocSize = allocSize;
	VK_CreateBufferMemory(allocSize, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &buffer->buffer, &buffer->memory);
	buffer->onGpu = VK_TRUE;
}
void VK_CreateShaderBindingTableBuffer(vkbuffer_t* buffer, VkDeviceSize allocSize) {
	buffer->allocSize = allocSize;
	VK_CreateBufferMemory(allocSize, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &buffer->buffer, &buffer->memory);
	VK_CHECK(vkMapMemory(vk.device, buffer->memory, 0, buffer->allocSize, 0, (byte * *)(&buffer->p)), "failed to Map Memory!");
}

void VK_UploadBufferDataOffset(vkbuffer_t* buffer, VkDeviceSize offset, VkDeviceSize size, const byte* data) {
    if (offset + size > buffer->allocSize) {
        ri.Error(ERR_FATAL, "Vulkan: Buffer to small!");
    }
	if (buffer->p == NULL) {
		ri.Error(ERR_FATAL, "Vulkan: Buffer not mapped!");
	}

    byte*p = buffer->p + offset;
    Com_Memcpy(p, data, (size_t)(size));
}

void VK_UploadBufferData(vkbuffer_t* buffer, const byte* data) {
	VK_UploadBufferDataOffset(buffer, 0, buffer->allocSize, data);
}

void VK_MapBuffer(vkbuffer_t* buffer) {
	if (!buffer->p) {
		VK_CHECK(vkMapMemory(vk.device, buffer->memory, 0, buffer->allocSize, 0, (byte * *)(&buffer->p)), "failed to Map Memory!");
	}
}

void VK_UnmapBuffer(vkbuffer_t* buffer) {
	if (buffer->p) {
		vkUnmapMemory(vk.device, buffer->memory);
		buffer->p = NULL;
	}
}

void VK_DestroyBuffer(vkbuffer_t* buffer)
{
	if (buffer->p) {
		vkUnmapMemory(vk.device, buffer->memory);
		buffer->p = NULL;
	}
	if (buffer->buffer != NULL) {
		vkDestroyBuffer(vk.device, buffer->buffer, NULL);
	}
	if (buffer->memory != NULL) {
		vkFreeMemory(vk.device, buffer->memory, NULL);
	}
	memset(buffer, 0, sizeof(vkbuffer_t));
}

//void VK_UploadAttribDataStride(vkbuffer_t *buffer, size_t element, size_t stride, const byte* data){
//    VkDeviceSize offset = 0;
//
//    byte*p;
//    VK_CHECK(vkMapMemory(vk.device, buffer->memory, 0, buffer->allocSize, 0, (byte**)(&p)), "failed to Map Memory!");
//    while(offset < buffer->allocSize){
//        Com_Memcpy(p, data + offset, (size_t)(element));
//        p += stride;
//        offset += stride;
//    }
//    vkUnmapMemory(vk.device, buffer->memory);
//
//}

