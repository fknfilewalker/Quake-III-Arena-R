#include "../tr_local.h"

static void VK_CreateDescriptorSetLayout(vkdescriptor_t *descriptor);
static void VK_CreateDescriptorPool(vkdescriptor_t *descriptor);
void VK_CreateDescriptorSet(vkdescriptor_t *descriptor);
void VK_UpdateDescriptorSet(vkdescriptor_t *descriptor);
void VK_SetUpdateSize(vkdescriptor_t* descriptor, uint32_t binding, VkShaderStageFlagBits stage, uint32_t updateSize);

// add descriptor binding without data
void VK_AddSamplerCount(vkdescriptor_t* descriptor, uint32_t binding, VkShaderStageFlagBits stage, uint32_t count) {
	descriptor->size++;
	descriptor->bindings = realloc(descriptor->bindings, descriptor->size * sizeof(VkDescriptorSetLayoutBinding));
	descriptor->data = realloc(descriptor->data, descriptor->size * sizeof(vkdescriptorData_t));

	descriptor->bindings[descriptor->size - 1].binding = binding;
	descriptor->bindings[descriptor->size - 1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptor->bindings[descriptor->size - 1].descriptorCount = count;
	descriptor->bindings[descriptor->size - 1].stageFlags = stage;
	descriptor->bindings[descriptor->size - 1].pImmutableSamplers = NULL;

	descriptor->data[descriptor->size - 1].size = count;
	descriptor->data[descriptor->size - 1].descImageInfo = malloc(count * sizeof(VkDescriptorImageInfo));
	for (int i = 0; i < count; i++) {
		descriptor->data[descriptor->size - 1].descImageInfo[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		descriptor->data[descriptor->size - 1].descImageInfo[i].imageView = VK_NULL_HANDLE;
		descriptor->data[descriptor->size - 1].descImageInfo[i].sampler = VK_NULL_HANDLE;
	}
}
void VK_AddSampler(vkdescriptor_t* descriptor, uint32_t binding, VkShaderStageFlagBits stage) {
	VK_AddSamplerCount(descriptor, binding, stage, 1);
	VK_SetUpdateSize(descriptor, binding, stage, 1);
}

void VK_AddStorageImage(vkdescriptor_t* descriptor, uint32_t binding, VkShaderStageFlagBits stage) {
	descriptor->size++;
	descriptor->bindings = realloc(descriptor->bindings, descriptor->size * sizeof(VkDescriptorSetLayoutBinding));
	descriptor->data = realloc(descriptor->data, descriptor->size * sizeof(vkdescriptorData_t));

	descriptor->bindings[descriptor->size - 1].binding = binding;
	descriptor->bindings[descriptor->size - 1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	descriptor->bindings[descriptor->size - 1].descriptorCount = 1;
	descriptor->bindings[descriptor->size - 1].stageFlags = stage;
	descriptor->bindings[descriptor->size - 1].pImmutableSamplers = NULL;

	int count = 1;
	descriptor->data[descriptor->size - 1].size = count;
	descriptor->data[descriptor->size - 1].descImageInfo = malloc(count * sizeof(VkDescriptorImageInfo));
	for (int i = 0; i < count; i++) {
		descriptor->data[descriptor->size - 1].descImageInfo[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		descriptor->data[descriptor->size - 1].descImageInfo[i].imageView = VK_NULL_HANDLE;
		descriptor->data[descriptor->size - 1].descImageInfo[i].sampler = VK_NULL_HANDLE;
	}
}
void VK_AddStorageBuffer(vkdescriptor_t* descriptor, uint32_t binding, VkShaderStageFlagBits stage) {
	descriptor->size++;
	descriptor->bindings = realloc(descriptor->bindings, descriptor->size * sizeof(VkDescriptorSetLayoutBinding));
	descriptor->data = realloc(descriptor->data, descriptor->size * sizeof(vkdescriptorData_t));

	descriptor->bindings[descriptor->size - 1].binding = binding;
	descriptor->bindings[descriptor->size - 1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptor->bindings[descriptor->size - 1].descriptorCount = 1;
	descriptor->bindings[descriptor->size - 1].stageFlags = stage;
	descriptor->bindings[descriptor->size - 1].pImmutableSamplers = NULL;

	descriptor->data[descriptor->size - 1].descBufferInfo.buffer = VK_NULL_HANDLE;
	descriptor->data[descriptor->size - 1].descBufferInfo.offset = 0;
	descriptor->data[descriptor->size - 1].descBufferInfo.range = 0;
}
void VK_AddUniformBuffer(vkdescriptor_t* descriptor, uint32_t binding, VkShaderStageFlagBits stage) {
	descriptor->size++;
	descriptor->bindings = realloc(descriptor->bindings, descriptor->size * sizeof(VkDescriptorSetLayoutBinding));
	descriptor->data = realloc(descriptor->data, descriptor->size * sizeof(vkdescriptorData_t));

	descriptor->bindings[descriptor->size - 1].binding = binding;
	descriptor->bindings[descriptor->size - 1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptor->bindings[descriptor->size - 1].descriptorCount = 1;
	descriptor->bindings[descriptor->size - 1].stageFlags = stage;
	descriptor->bindings[descriptor->size - 1].pImmutableSamplers = NULL;

	descriptor->data[descriptor->size - 1].descBufferInfo.buffer = VK_NULL_HANDLE;
	descriptor->data[descriptor->size - 1].descBufferInfo.offset = 0;
	descriptor->data[descriptor->size - 1].descBufferInfo.range = 0;
}
void VK_AddAccelerationStructure(vkdescriptor_t* descriptor, uint32_t binding, VkShaderStageFlagBits stage) {
	descriptor->size++;
	descriptor->bindings = realloc(descriptor->bindings, descriptor->size * sizeof(VkDescriptorSetLayoutBinding));
	descriptor->data = realloc(descriptor->data, descriptor->size * sizeof(vkdescriptorData_t));

	descriptor->bindings[descriptor->size - 1].binding = binding;
	descriptor->bindings[descriptor->size - 1].descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
	descriptor->bindings[descriptor->size - 1].descriptorCount = 1;
	descriptor->bindings[descriptor->size - 1].stageFlags = stage;
	descriptor->bindings[descriptor->size - 1].pImmutableSamplers = NULL;

	descriptor->data[descriptor->size - 1].descAccelerationStructureInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV;
	descriptor->data[descriptor->size - 1].descAccelerationStructureInfo.pNext = VK_NULL_HANDLE;
	descriptor->data[descriptor->size - 1].descAccelerationStructureInfo.accelerationStructureCount = 1;
	descriptor->data[descriptor->size - 1].descAccelerationStructureInfo.pAccelerationStructures = VK_NULL_HANDLE;
}

// set descriptor data
void VK_SetSamplerPosition(vkdescriptor_t *descriptor, uint32_t binding, VkShaderStageFlagBits stage, VkSampler sampler, VkImageView imageView, uint32_t pos){
    for (int i = 0; i < descriptor->size; ++i) {
        if(descriptor->bindings[i].binding == binding &&
           descriptor->bindings[i].stageFlags == stage){
            descriptor->data[i].descImageInfo[pos].sampler = sampler;
            descriptor->data[i].descImageInfo[pos].imageView = imageView;
            return;
        }
    }
}
void VK_SetSampler(vkdescriptor_t* descriptor, uint32_t binding, VkShaderStageFlagBits stage, VkSampler sampler, VkImageView imageView) {
	VK_SetSamplerPosition(descriptor, binding, stage, sampler, imageView, 0);
}

void VK_SetStorageImage(vkdescriptor_t* descriptor, uint32_t binding, VkShaderStageFlagBits stage, VkImageView imageView) {
	for (int i = 0; i < descriptor->size; ++i) {
		if (descriptor->bindings[i].binding == binding &&
			descriptor->bindings[i].stageFlags == stage) {
			descriptor->data[i].descImageInfo[0].sampler = VK_NULL_HANDLE;
			descriptor->data[i].descImageInfo[0].imageView = imageView;
			return;
		}
	}
}
void VK_SetStorageBuffer(vkdescriptor_t* descriptor, uint32_t binding, VkShaderStageFlagBits stage, VkBuffer buffer) {
	for (int i = 0; i < descriptor->size; ++i) {
		if (descriptor->bindings[i].binding == binding &&
			descriptor->bindings[i].stageFlags == stage) {
			descriptor->data[i].descBufferInfo.buffer = buffer;
			descriptor->data[i].descBufferInfo.offset = 0;
			descriptor->data[i].descBufferInfo.range = VK_WHOLE_SIZE;
			return;
		}
	}
}
void VK_SetUniformBuffer(vkdescriptor_t* descriptor, uint32_t binding, VkShaderStageFlagBits stage, VkBuffer buffer) {
	for (int i = 0; i < descriptor->size; ++i) {
		if (descriptor->bindings[i].binding == binding &&
			descriptor->bindings[i].stageFlags == stage) {
			descriptor->data[i].descBufferInfo.buffer = buffer;
			descriptor->data[i].descBufferInfo.offset = 0;
			descriptor->data[i].descBufferInfo.range = VK_WHOLE_SIZE;
			return;
		}
	}
}
void VK_SetAccelerationStructure(vkdescriptor_t* descriptor, uint32_t binding, VkShaderStageFlagBits stage, VkAccelerationStructureNV* as) {
	for (int i = 0; i < descriptor->size; ++i) {
		if (descriptor->bindings[i].binding == binding &&
			descriptor->bindings[i].stageFlags == stage) {
			descriptor->data[i].descAccelerationStructureInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV;
			descriptor->data[i].descAccelerationStructureInfo.pAccelerationStructures = as;
			descriptor->data[i].descAccelerationStructureInfo.accelerationStructureCount = 1;
			return;
		}
	}
}

void VK_SetUpdateSize(vkdescriptor_t* descriptor, uint32_t binding, VkShaderStageFlagBits stage, uint32_t updateSize) {
	for (int i = 0; i < descriptor->size; ++i) {
		if (descriptor->bindings[i].binding == binding &&
			descriptor->bindings[i].stageFlags == stage) {
			descriptor->data[i].updateSize = updateSize;
			return;
		}
	}
}

static void VK_CreateDescriptorSetLayout(vkdescriptor_t *descriptor) {
    
	// VK_EXT_descriptor_indexing 
	VkDescriptorBindingFlagBitsEXT* flags = calloc(descriptor->size, sizeof(VkDescriptorBindingFlagBitsEXT));
	if(descriptor->size > 0) flags[descriptor->size-1] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT;

	VkDescriptorSetLayoutBindingFlagsCreateInfoEXT layoutCreateInfo = { 0 };
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
	layoutCreateInfo.pNext = VK_NULL_HANDLE;
	layoutCreateInfo.bindingCount = descriptor->size;
	layoutCreateInfo.pBindingFlags = &flags[0];
	//

    VkDescriptorSetLayoutCreateInfo descLayoutInfo = { 0 };
    descLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	if (descriptor->lastBindingVariableSizeExt)descLayoutInfo.pNext = &layoutCreateInfo;
    descLayoutInfo.bindingCount = descriptor->size;
    descLayoutInfo.pBindings = &descriptor->bindings[0];
  
    VK_CHECK(vkCreateDescriptorSetLayout(vk.device, &descLayoutInfo, NULL, &descriptor->layout), " failed to create Descriptor Set Layout!");

	free(flags);
}

static void VK_CreateDescriptorPool(vkdescriptor_t *descriptor) {
    VkDescriptorPoolSize *poolSizes = calloc(descriptor->size, sizeof(VkDescriptorPoolSize));
    
    for (int i = 0; i < descriptor->size; i++) {
        poolSizes[i].type = descriptor->bindings[i].descriptorType;
		poolSizes[i].descriptorCount = descriptor->bindings[i].descriptorCount;
    }
    
    VkDescriptorPoolCreateInfo descPoolInfo = {0};
    descPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;//VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    descPoolInfo.maxSets = 1;
    descPoolInfo.poolSizeCount = descriptor->size;
    descPoolInfo.pPoolSizes = &poolSizes[0];
    VK_CHECK(vkCreateDescriptorPool(vk.device, &descPoolInfo, NULL, &descriptor->pool), " failed to create Descriptor Pool!");
    
    free(poolSizes);
}

void VK_CreateDescriptorSet(vkdescriptor_t *descriptor) {
    VkDescriptorSetAllocateInfo descSetAllocInfo = {0};
    descSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descSetAllocInfo.descriptorSetCount = 1;
    descSetAllocInfo.descriptorPool = descriptor->pool;
    descSetAllocInfo.pSetLayouts = &descriptor->layout;
        
    VK_CHECK(vkAllocateDescriptorSets(vk.device, &descSetAllocInfo, &descriptor->set), " failed to allocate Descriptor Set!");
}

void VK_FinishDescriptor(vkdescriptor_t* descriptor) {
	VK_CreateDescriptorSetLayout(descriptor);
	VK_CreateDescriptorPool(descriptor);
	VK_CreateDescriptorSet(descriptor);
	VK_UpdateDescriptorSet(descriptor);
}

void VK_FinishDescriptorWithoutUpdate(vkdescriptor_t* descriptor) {
	VK_CreateDescriptorSetLayout(descriptor);
	VK_CreateDescriptorPool(descriptor);
	VK_CreateDescriptorSet(descriptor);
}

void VK_UpdateDescriptorSet(vkdescriptor_t* descriptor) {
	VkWriteDescriptorSet* descWrite = calloc(descriptor->size, sizeof(VkWriteDescriptorSet));
	for (int j = 0; j < descriptor->size; ++j) {
		descWrite[j].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		switch (descriptor->bindings[j].descriptorType) {
		case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
			descWrite[j].dstSet = descriptor->set;
			descWrite[j].dstBinding = descriptor->bindings[j].binding;
			descWrite[j].descriptorCount = descriptor->data[j].updateSize;
			descWrite[j].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descWrite[j].pImageInfo = &descriptor->data[j].descImageInfo[0];
			assert(descriptor->data[j].descImageInfo[0].imageView != NULL);
			break;
		case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
			descWrite[j].dstSet = descriptor->set;
			descWrite[j].dstBinding = descriptor->bindings[j].binding;
			descWrite[j].descriptorCount = 1;
			descWrite[j].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			descWrite[j].pImageInfo = &descriptor->data[j].descImageInfo[0];
			assert(descriptor->data[j].descImageInfo[0].imageView != NULL);
			break;
		case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
			descWrite[j].dstSet = descriptor->set;
			descWrite[j].dstBinding = descriptor->bindings[j].binding;
			descWrite[j].descriptorCount = 1;
			descWrite[j].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			descWrite[j].pBufferInfo = &descriptor->data[j].descBufferInfo;
			assert(descriptor->data[j].descBufferInfo.buffer != NULL);
			break;
		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
			descWrite[j].dstSet = descriptor->set;
			descWrite[j].dstBinding = descriptor->bindings[j].binding;
			descWrite[j].descriptorCount = 1;
			descWrite[j].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descWrite[j].pBufferInfo = &descriptor->data[j].descBufferInfo;
			assert(descriptor->data[j].descBufferInfo.buffer != NULL);
			break;
		case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:
			descWrite[j].dstSet = descriptor->set;
			descWrite[j].dstBinding = descriptor->bindings[j].binding;
			descWrite[j].descriptorCount = 1;
			descWrite[j].descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
			descWrite[j].pNext = &descriptor->data[j].descAccelerationStructureInfo;
			assert(descriptor->data[j].descAccelerationStructureInfo.pAccelerationStructures != NULL);
			break;
		}
	}
	vkUpdateDescriptorSets(vk.device, descriptor->size, &descWrite[0], 0, NULL);
	free(descWrite);
}

void VK_DestroyDescriptor(vkdescriptor_t* descriptor){
	if (descriptor->set != VK_NULL_HANDLE) {
		vkFreeDescriptorSets(vk.device, descriptor->pool, 1, &descriptor->set);
		descriptor->set = VK_NULL_HANDLE;
	}
    if(descriptor->layout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(vk.device, descriptor->layout, NULL);
        descriptor->layout = VK_NULL_HANDLE;
    }
    if(descriptor->pool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(vk.device, descriptor->pool, NULL);
		descriptor->pool = VK_NULL_HANDLE;
    }
    
	for (int i = 0; i < descriptor->size; ++i) {
		if(descriptor->bindings[i].descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) free(descriptor->data[i].descImageInfo);
	}
    free(descriptor->data);
    free(descriptor->bindings);
    
	memset(descriptor, 0, sizeof(vkdescriptor_t));
}

