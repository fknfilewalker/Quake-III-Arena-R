#include "../tr_local.h"

static vkshader_t *singleTexture;
static vkshader_t* singleTextureClip;
static vkshader_t* clearAttachment;

void VK_CreateShaderModule(VkShaderModule *handle, const char *code, size_t size);

void VK_SingleTextureShader(vkshader_t *shader){
	if (singleTexture) {
		Com_Memcpy(shader, singleTexture, sizeof(vkshader_t));
		return;
	}
	Com_Printf("new shader \n");

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
    fp = fopen("../../shader/spv/singleTexture.vert.spv", "rb");
    fseek (fp, 0, SEEK_END);
    vert_spv_size = ftell(fp);
    fseek (fp, 0, SEEK_SET);
    fread(vert_spv,sizeof(vert_spv),1,fp);
    fclose(fp);
    
    
    fp = fopen("../../shader/spv/singleTexture.frag.spv", "rb");
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

	if (singleTexture == NULL) {
		singleTexture = malloc(sizeof(vkshader_t));
		Com_Memcpy(singleTexture, shader, sizeof(vkshader_t));
	}
}

void VK_SingleTextureClipShader(vkshader_t* shader) {
	if (singleTextureClip) {
		Com_Memcpy(shader, singleTextureClip, sizeof(vkshader_t));
		return;
	}
	Com_Printf("new shader \n");

	shader->size = 2;
	shader->modules = malloc(shader->size * sizeof(VkShaderModule));
	shader->flags = malloc(shader->size * sizeof(VkShaderStageFlagBits));
	shader->shaderStageCreateInfos = calloc(shader->size, sizeof(VkPipelineShaderStageCreateInfo));

	shader->flags[0] = VK_SHADER_STAGE_VERTEX_BIT;
	shader->flags[1] = VK_SHADER_STAGE_FRAGMENT_BIT;

	unsigned char vert_spv[3000] = { 0 };
	unsigned char frag_spv[3000] = { 0 };
	size_t vert_spv_size;
	size_t frag_spv_size;

	FILE* fp;
	fp = fopen("../../shader/spv/singleTextureClip.vert.spv", "rb");
	fseek(fp, 0, SEEK_END);
	vert_spv_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	fread(vert_spv, sizeof(vert_spv), 1, fp);
	fclose(fp);


	fp = fopen("../../shader/spv/singleTexture.frag.spv", "rb");
	fseek(fp, 0, SEEK_END);
	frag_spv_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	fread(frag_spv, sizeof(frag_spv), 1, fp);
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

	if (singleTextureClip == NULL) {
		singleTextureClip = malloc(sizeof(vkshader_t));
		Com_Memcpy(singleTextureClip, shader, sizeof(vkshader_t));
	}
}

void VK_ClearAttachmentShader(vkshader_t* shader) {
	if (clearAttachment) {
		Com_Memcpy(shader, clearAttachment, sizeof(vkshader_t));
		return;
	}
	Com_Printf("new shader \n");

	shader->size = 2;
	shader->modules = malloc(shader->size * sizeof(VkShaderModule));
	shader->flags = malloc(shader->size * sizeof(VkShaderStageFlagBits));
	shader->shaderStageCreateInfos = calloc(shader->size, sizeof(VkPipelineShaderStageCreateInfo));

	shader->flags[0] = VK_SHADER_STAGE_VERTEX_BIT;
	shader->flags[1] = VK_SHADER_STAGE_FRAGMENT_BIT;

	unsigned char vert_spv[3000] = { 0 };
	unsigned char frag_spv[3000] = { 0 };
	size_t vert_spv_size;
	size_t frag_spv_size;

	FILE* fp;
	fp = fopen("../../shader/spv/clearAttachment.vert.spv", "rb");
	fseek(fp, 0, SEEK_END);
	vert_spv_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	fread(vert_spv, sizeof(vert_spv), 1, fp);
	fclose(fp);


	fp = fopen("../../shader/spv/clearAttachment.frag.spv", "rb");
	fseek(fp, 0, SEEK_END);
	frag_spv_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	fread(frag_spv, sizeof(frag_spv), 1, fp);
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

	if (clearAttachment == NULL) {
		clearAttachment = malloc(sizeof(vkshader_t));
		Com_Memcpy(clearAttachment, shader, sizeof(vkshader_t));
	}
}

void VK_DestroyShader(vkshader_t* shader) {
	for (int i = 0; i < shader->size; ++i) {
		vkDestroyShaderModule(vk.device, shader->modules[i], NULL);
	}
	memset(shader, 0, sizeof(vkshader_t));
}

void VK_DestroyAllShader() {
	if (singleTexture != NULL) {
		VK_DestroyShader(singleTexture);
		free(singleTexture);
		singleTexture = NULL;
	}
	if (singleTextureClip != NULL) {
		VK_DestroyShader(singleTextureClip);
		free(singleTextureClip);
		singleTextureClip = NULL;
	}
	if (clearAttachment != NULL) {
		VK_DestroyShader(clearAttachment);
		free(clearAttachment);
		clearAttachment = NULL;
	}
}

void VK_CreateShaderModule(VkShaderModule *handle, const char *code, size_t size)
{
    VkShaderModuleCreateInfo createInfo = { 0 };
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = size;
    createInfo.pCode = (const uint32_t*)code;
    
    VK_CHECK(vkCreateShaderModule(vk.device, &createInfo, NULL, handle), "failed to create Shader Module!");
}
