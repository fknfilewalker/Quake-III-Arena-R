#include "../tr_local.h"

void VK_CreateShaderModule(VkShaderModule *handle, const char *code, size_t size);

void VK_SingleTextureShader(vkshader_t *shader){
    shader->size = 2;
    shader->modules = malloc(shader->size * sizeof(VkShaderModule));
    shader->flags = malloc(shader->size * sizeof(VkShaderStageFlagBits));
    shader->shaderStageCreateInfos = calloc(shader->size, sizeof(VkPipelineShaderStageCreateInfo));
    
    shader->flags[0] = VK_SHADER_STAGE_VERTEX_BIT;
    shader->flags[1] = VK_SHADER_STAGE_FRAGMENT_BIT;
    
    unsigned char vert_spv[1500] = {0};
    unsigned char frag_spv[1500] = {0};
    size_t vert_spv_size;
    size_t frag_spv_size;
    
    FILE *fp;
    fp = fopen("../../code/renderer/vulkan/shader/test_tex_vert.spv", "rb");
    fseek (fp, 0, SEEK_END);
    vert_spv_size = ftell(fp);
    fseek (fp, 0, SEEK_SET);
    fread(vert_spv,sizeof(vert_spv),1,fp);
    fclose(fp);
    
    
    fp = fopen("../../code/renderer/vulkan/shader/test_tex_frag.spv", "rb");
    fseek (fp, 0, SEEK_END);
    frag_spv_size = ftell(fp);
    fseek (fp, 0, SEEK_SET);
    fread(frag_spv,sizeof(frag_spv),1,fp);
    fclose(fp);

    VK_CreateShaderModule(&shader->modules[0], vert_spv, vert_spv_size);
    VK_CreateShaderModule(&shader->modules[1], frag_spv, frag_spv_size);
    
    shader->shaderStageCreateInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader->shaderStageCreateInfos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shader->shaderStageCreateInfos[0].module = shader->modules[0];
    shader->shaderStageCreateInfos[0].pName = "main";
    
    shader->shaderStageCreateInfos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader->shaderStageCreateInfos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shader->shaderStageCreateInfos[1].module = shader->modules[1];
    shader->shaderStageCreateInfos[1].pName = "main";
}


void VK_CreateShaderModule(VkShaderModule *handle, const char *code, size_t size)
{
    VkShaderModuleCreateInfo createInfo = { 0 };
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = size;
    createInfo.pCode = (const uint32_t*)code;
    
    VK_CHECK(vkCreateShaderModule(vk.device, &createInfo, NULL, handle), "failed to create Shader Module!");
    
}