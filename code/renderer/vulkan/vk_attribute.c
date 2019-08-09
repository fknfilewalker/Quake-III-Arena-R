#include "../tr_local.h"

void VK_CreateVertexBuffer(vkattribbuffer_t *buffer, VkDeviceSize allocSize){
    buffer->allocSize = allocSize;
    VK_CreateBuffer(allocSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_HostVisibleMemoryIndex(), &buffer->buffer, &buffer->memory);
    VK_CHECK(vkMapMemory(vk.device, buffer->memory, 0, buffer->allocSize, 0, (byte**)(&buffer->p)), "failed to Map Memory!");
}

void VK_CreateIndexBuffer(vkattribbuffer_t *buffer, VkDeviceSize allocSize){
    buffer->allocSize = allocSize;
    VK_CreateBuffer(allocSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_HostVisibleMemoryIndex(), &buffer->buffer, &buffer->memory);
    VK_CHECK(vkMapMemory(vk.device, buffer->memory, 0, buffer->allocSize, 0, (byte**)(&buffer->p)), "failed to Map Memory!");
}

void VK_UploadAttribDataOffset(vkattribbuffer_t* buffer, VkDeviceSize offset, VkDeviceSize size, const byte* data) {
    if (offset + size > buffer->allocSize) {
        ri.Error(ERR_FATAL, "Vulkan: Buffer to small!");
    }

    byte*p = buffer->p + offset;
    Com_Memcpy(p, data, (size_t)(size));
}

void VK_UploadAttribData(vkattribbuffer_t* buffer, const byte* data) {
	VK_UploadAttribDataOffset(buffer, 0, buffer->allocSize, data);
}

void VK_DestroyAttribBuffer(vkattribbuffer_t* buffer)
{
    vkUnmapMemory(vk.device, buffer->memory);
	if (buffer->buffer != NULL) {
		vkDestroyBuffer(vk.device, buffer->buffer, NULL);
	}
	if (buffer->memory != NULL) {
		vkFreeMemory(vk.device, buffer->memory, NULL);
	}
	memset(buffer, 0, sizeof(vkattribbuffer_t));
}

//void VK_UploadAttribDataStride(vkattribbuffer_t *buffer, size_t element, size_t stride, const byte* data){
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

