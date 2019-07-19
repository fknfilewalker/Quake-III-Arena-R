#include "../tr_local.h"

void VK_CreateVertexBuffer(vkattribbuffer_t *buffer, VkDeviceSize allocSize){
    buffer->allocSize = allocSize;
    VK_CreateBuffer(allocSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_HostVisibleMemoryIndex(), &buffer->buffer, &buffer->memory);
}

void VK_CreateIndexBuffer(vkattribbuffer_t *buffer, VkDeviceSize allocSize){
    buffer->allocSize = allocSize;
    VK_CreateBuffer(allocSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_HostVisibleMemoryIndex(), &buffer->buffer, &buffer->memory);
}

void VK_UploadAttribDataStride(vkattribbuffer_t *buffer, size_t element, size_t stride, const byte* data){
    VkDeviceSize offset = 0;
    
	byte*p;
    VK_CHECK(vkMapMemory(vk.device, buffer->memory, 0, buffer->allocSize, 0, (byte**)(&p)), "failed to Map Memory!");
    while(offset < buffer->allocSize){
        Com_Memcpy(p, data + offset, (size_t)(element));
        p += stride;
        offset += stride;
    }
    vkUnmapMemory(vk.device, buffer->memory);
    
}

void VK_UploadAttribData(vkattribbuffer_t *buffer, const byte* data){
	byte*p;
    VK_CHECK(vkMapMemory(vk.device, buffer->memory, 0, buffer->allocSize, 0, (byte**)(&p)), "failed to Map Memory!");
    Com_Memcpy(p, data, (size_t)(buffer->allocSize));
    vkUnmapMemory(vk.device, buffer->memory);
}