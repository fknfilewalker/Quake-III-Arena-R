#include "../tr_local.h"

static void VK_CreateDescriptorSetLayout(vkdescriptor_t *descriptor);
static void VK_CreateDescriptorPool(vkdescriptor_t *descriptor);
void VK_CreateDescriptorSet(vkdescriptor_t *descriptor);
void VK_UpdateDescriptorSet(vkdescriptor_t *descriptor);

void VK_AddSampler(vkdescriptor_t *descriptor, uint32_t binding, VkShaderStageFlagBits stage){
    descriptor->size++;
    descriptor->bindings = realloc(descriptor->bindings, descriptor->size * sizeof(VkDescriptorSetLayoutBinding));
    descriptor->data = realloc(descriptor->data, descriptor->size * sizeof(vkdescriptorData_t));

    descriptor->bindings[descriptor->size - 1].binding = binding;
    descriptor->bindings[descriptor->size - 1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor->bindings[descriptor->size - 1].descriptorCount = 1;
    descriptor->bindings[descriptor->size - 1].stageFlags = stage;
    descriptor->bindings[descriptor->size - 1].pImmutableSamplers = NULL;
    
    descriptor->data[descriptor->size - 1].descImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    descriptor->data[descriptor->size - 1].descImageInfo.imageView = VK_NULL_HANDLE;
    descriptor->data[descriptor->size - 1].descImageInfo.sampler = VK_NULL_HANDLE;
}

void VK_SetSampler(vkdescriptor_t *descriptor, uint32_t binding, VkShaderStageFlagBits stage, VkSampler sampler, VkImageView imageView){
    for (int i = 0; i < descriptor->size; ++i) {
        if(descriptor->bindings[i].binding == binding &&
           descriptor->bindings[i].stageFlags == stage){
            descriptor->data[i].descImageInfo.sampler = sampler;
            descriptor->data[i].descImageInfo.imageView = imageView;
            return;
        }
    }
}

void VK_FinishDescriptor(vkdescriptor_t *descriptor){
    VK_CreateDescriptorSetLayout(descriptor);
    VK_CreateDescriptorPool(descriptor);
    VK_CreateDescriptorSet(descriptor);
    VK_UpdateDescriptorSet(descriptor);
}

void VK_UpdateDescriptorSet(vkdescriptor_t *descriptor) {
    
    for (uint32_t i = 0; i < vk.swapchain.imageCount; ++i) {
        VkWriteDescriptorSet *descWrite = calloc(descriptor->size, sizeof(VkWriteDescriptorSet));
        for (int j = 0; j < descriptor->size; ++j) {
            descWrite[j].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            if(descriptor->bindings[j].descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER){
                descWrite[j].dstSet = descriptor->sets[i];
                descWrite[j].dstBinding = descriptor->bindings[j].binding;
                descWrite[j].descriptorCount = 1;
                descWrite[j].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descWrite[j].pImageInfo = &descriptor->data[j].descImageInfo;
                
                assert(descriptor->data[j].descImageInfo.imageView != NULL);
            }
        }
        vkUpdateDescriptorSets(vk.device, descriptor->size, &descWrite[0], 0, NULL);
        free(descWrite);
    }
}

static void VK_CreateDescriptorSetLayout(vkdescriptor_t *descriptor) {
    
    VkDescriptorSetLayoutCreateInfo descLayoutInfo = { 0 };
    descLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descLayoutInfo.bindingCount = descriptor->size;
    descLayoutInfo.pBindings = &descriptor->bindings[0];
    
    VK_CHECK(vkCreateDescriptorSetLayout(vk.device, &descLayoutInfo, NULL, &descriptor->layout), " failed to create Descriptor Set Layout!");
}

static void VK_CreateDescriptorPool(vkdescriptor_t *descriptor) {
    VkDescriptorPoolSize *poolSizes = calloc(descriptor->size, sizeof(VkDescriptorPoolSize));
    
    for (int i = 0; i < descriptor->size; i++) {
        poolSizes[i].type = descriptor->bindings[i].descriptorType;
        poolSizes[i].descriptorCount = vk.swapchain.imageCount * descriptor->bindings[i].descriptorCount;
    }
    
    VkDescriptorPoolCreateInfo descPoolInfo = {0};
    descPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;//VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    descPoolInfo.maxSets = vk.swapchain.imageCount;
    descPoolInfo.poolSizeCount = descriptor->size;
    descPoolInfo.pPoolSizes = &poolSizes[0];
    VK_CHECK(vkCreateDescriptorPool(vk.device, &descPoolInfo, NULL, &descriptor->pool), " failed to create Descriptor Pool!");
    
    free(poolSizes);
}

void VK_CreateDescriptorSet(vkdescriptor_t *descriptor) {
    descriptor->sets = calloc(vk.swapchain.imageCount,  sizeof(VkDescriptorSet));
    
    for(int i = 0; i < vk.swapchain.imageCount; ++i){
        if(descriptor->sets[i]) vkFreeDescriptorSets(vk.device, &descriptor->pool, descriptor->size, &descriptor->sets[i]);
    }
    
    for (uint32_t i = 0; i < vk.swapchain.imageCount; ++i) {
        VkDescriptorSetAllocateInfo descSetAllocInfo = {0};
        descSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descSetAllocInfo.descriptorSetCount = 1;
        descSetAllocInfo.descriptorPool = descriptor->pool;
        descSetAllocInfo.pSetLayouts = &descriptor->layout;
        
        VK_CHECK(vkAllocateDescriptorSets(vk.device, &descSetAllocInfo, &descriptor->sets[i]), " failed to allocate Descriptor Set!");
    }
}

void VK_DestroyDescriptor(vkdescriptor_t* descriptor){
    for(int i = 0; i < vk.swapchain.imageCount; ++i){
        //if(descriptor->sets[i] != NULL) {
            vkFreeDescriptorSets(vk.device, descriptor->pool, descriptor->size, &descriptor->sets[i]);
            descriptor->sets[i] = VK_NULL_HANDLE;
        //}
    }
    //if(descriptor->layout != NULL) {
        vkDestroyDescriptorSetLayout(vk.device, descriptor->layout, NULL);
        descriptor->layout = VK_NULL_HANDLE;
    //}
    //if(descriptor->pool != NULL) {
        vkDestroyDescriptorPool(vk.device, descriptor->pool, NULL);
        descriptor->pool = VK_NULL_HANDLE;
    //}
    
    free(descriptor->bindings);
    free(descriptor->data);
    free(descriptor->sets);
    descriptor->size = 0;
    descriptor->bindings = NULL;
    descriptor->data = NULL;
    descriptor->sets = NULL;
}

