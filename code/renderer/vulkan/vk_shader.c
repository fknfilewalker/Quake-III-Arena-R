#include "../tr_local.h"

// SHADER HEADER
#include "../../../shader/header/clearAttachment.vert.h"
#include "../../../shader/header/clearAttachment.frag.h"

#include "../../../shader/header/fullscreenRect.vert.h"
#include "../../../shader/header/fullscreenRect.frag.h"

#include "../../../shader/header/singleTexture.vert.h"
#include "../../../shader/header/singleTextureClip.vert.h"
#include "../../../shader/header/singleTexture.frag.h"

#include "../../../shader/header/multiTexture.vert.h"
#include "../../../shader/header/multiTextureClip.vert.h"
#include "../../../shader/header/multiTextureAdd.frag.h"
#include "../../../shader/header/multiTextureMul.frag.h"

// RTX
#include "../../../shader/header/raygen.rgen.h"
#include "../../../shader/header/miss.rmiss.h"
#include "../../../shader/header/closesthit.rchit.h"

static vkshader_t *singleTexture;
static vkshader_t *singleTextureClip;
static vkshader_t *multiTextureMul;
static vkshader_t *multiTextureMulClip;
static vkshader_t *multiTextureAdd;
static vkshader_t *multiTextureAddClip;
static vkshader_t *clearAttachment;
static vkshader_t *fullscreenRect;
// rtx
static vkshader_t* rayTracing;

void VK_CreateShaderModule(VkShaderModule *handle, const char *code, size_t size);
void VK_LoadVertFragShadersFromFile(vkshader_t *shader, const char *vertexSPV, const char *fragmentSPV);
void VK_LoadVertFragShadersFromVariable(vkshader_t* shader, const char* vertexSPV, const uint32_t sizeVert, const char* fragmentSPV, const uint32_t sizeFrag);
void VK_LoadRayTracingShadersFromVariable(vkshader_t* shader, const char* rgenSPV, const uint32_t sizeRGEN,
	const char* rmissSPV, const uint32_t sizeRMISS,
	const char* rhitSPV, const uint32_t sizeRHIT);

void VK_SingleTextureShader(vkshader_t *shader){
    if (singleTexture == NULL) {
        singleTexture = malloc(sizeof(vkshader_t));
		//VK_LoadVertFragShadersFromFile(singleTexture, "../../shader/spv/singleTexture.vert.spv", "../../shader/spv/singleTexture.frag.spv");
		VK_LoadVertFragShadersFromVariable(singleTexture, &singleTextureVert, sizeof(singleTextureVert), &singleTextureFrag, sizeof(singleTextureFrag));
    }
    Com_Memcpy(shader, singleTexture, sizeof(vkshader_t));
}

void VK_SingleTextureClipShader(vkshader_t* shader) {
    if (singleTextureClip == NULL) {
        singleTextureClip = malloc(sizeof(vkshader_t));
		//VK_LoadVertFragShadersFromFile(singleTextureClip, "../../shader/spv/singleTextureClip.vert.spv", "../../shader/spv/singleTexture.frag.spv");
		VK_LoadVertFragShadersFromVariable(singleTextureClip, &singleTextureClipVert, sizeof(singleTextureClipVert), &singleTextureFrag, sizeof(singleTextureFrag));
    }
    Com_Memcpy(shader, singleTextureClip, sizeof(vkshader_t));
}

void VK_MultiTextureMulShader(vkshader_t *shader){
    if (multiTextureMul == NULL) {
        multiTextureMul = malloc(sizeof(vkshader_t));
		//VK_LoadVertFragShadersFromFile(multiTextureMul, "../../shader/spv/multiTexture.vert.spv", "../../shader/spv/multiTextureMul.frag.spv");
		VK_LoadVertFragShadersFromVariable(multiTextureMul, &multiTextureVert, sizeof(multiTextureVert), &multiTextureMulFrag, sizeof(multiTextureMulFrag));
    }
    Com_Memcpy(shader, multiTextureMul, sizeof(vkshader_t));
}

void VK_MultiTextureMulClipShader(vkshader_t* shader) {
    if (multiTextureMulClip == NULL) {
        multiTextureMulClip = malloc(sizeof(vkshader_t));
		//VK_LoadVertFragShadersFromFile(multiTextureMulClip, "../../shader/spv/multiTextureClip.vert.spv", "../../shader/spv/multiTextureMul.frag.spv");
		VK_LoadVertFragShadersFromVariable(multiTextureMulClip, &multiTextureClipVert, sizeof(multiTextureClipVert), &multiTextureMulFrag, sizeof(multiTextureMulFrag));
    }
    Com_Memcpy(shader, multiTextureMulClip, sizeof(vkshader_t));
}

void VK_MultiTextureAddShader(vkshader_t *shader){
    if (multiTextureAdd == NULL) {
        multiTextureAdd = malloc(sizeof(vkshader_t));
		//VK_LoadVertFragShadersFromFile(multiTextureAdd, "../../shader/spv/multiTexture.vert.spv", "../../shader/spv/multiTextureAdd.frag.spv");
		VK_LoadVertFragShadersFromVariable(multiTextureAdd, &multiTextureVert, sizeof(multiTextureVert), &multiTextureAddFrag, sizeof(multiTextureAddFrag));
    }
    Com_Memcpy(shader, multiTextureAdd, sizeof(vkshader_t));
}

void VK_MultiTextureAddClipShader(vkshader_t* shader) {
    if (multiTextureAddClip == NULL) {
        multiTextureAddClip = malloc(sizeof(vkshader_t));
		//VK_LoadVertFragShadersFromFile(multiTextureAddClip, "../../shader/spv/multiTextureClip.vert.spv", "../../shader/spv/multiTextureAdd.frag.spv");
		VK_LoadVertFragShadersFromVariable(multiTextureAddClip, &multiTextureClipVert, sizeof(multiTextureClipVert), &multiTextureAddFrag, sizeof(multiTextureAddFrag));
    }
    Com_Memcpy(shader, multiTextureAddClip, sizeof(vkshader_t));
}

void VK_ClearAttachmentShader(vkshader_t* shader) {
    if (clearAttachment == NULL) {
        clearAttachment = malloc(sizeof(vkshader_t));
        //VK_LoadVertFragShadersFromFile(clearAttachment, "../../shader/spv/clearAttachment.vert.spv", "../../shader/spv/clearAttachment.frag.spv");
		VK_LoadVertFragShadersFromVariable(clearAttachment, &clearAttachmentVert, sizeof(clearAttachmentVert), &clearAttachmentFrag, sizeof(clearAttachmentFrag));
    }
    Com_Memcpy(shader, clearAttachment, sizeof(vkshader_t));
}

void VK_FullscreenRectShader(vkshader_t* shader) {
	if (fullscreenRect == NULL) {
		fullscreenRect = malloc(sizeof(vkshader_t));
		//VK_LoadVertFragShadersFromFile(fullscreenRect, "../../shader/spv/fullscreenRect.vert.spv", "../../shader/spv/fullscreenRect.frag.spv");
		VK_LoadVertFragShadersFromVariable(fullscreenRect, &fullscreenRectVert, sizeof(fullscreenRectVert), &fullscreenRectFrag, sizeof(fullscreenRectFrag));
	}
	Com_Memcpy(shader, fullscreenRect, sizeof(vkshader_t));
}

// rtx
void VK_RayTracingShader(vkshader_t* shader) {
	if (rayTracing == NULL) {
		rayTracing = malloc(sizeof(vkshader_t));
		VK_LoadRayTracingShadersFromVariable(rayTracing, &raygenRGen, sizeof(raygenRGen), &missRMiss, sizeof(missRMiss), &closesthitRCHit, sizeof(closesthitRCHit));
	}
	Com_Memcpy(shader, rayTracing, sizeof(vkshader_t));
}

void VK_DestroyShader(vkshader_t* shader) {
	for (int i = 0; i < shader->size; ++i) {
		vkDestroyShaderModule(vk.device, shader->modules[i], NULL);
	}
	memset(shader, 0, sizeof(vkshader_t));
}

void VK_DestroyAllShaders() {
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
    if (multiTextureMul != NULL) {
        VK_DestroyShader(multiTextureMul);
        free(multiTextureMul);
        multiTextureMul = NULL;
    }
    if (multiTextureMulClip != NULL) {
        VK_DestroyShader(multiTextureMulClip);
        free(multiTextureMulClip);
        multiTextureMulClip = NULL;
    }
    if (multiTextureAdd != NULL) {
        VK_DestroyShader(multiTextureAdd);
        free(multiTextureAdd);
        multiTextureAdd = NULL;
    }
    if (multiTextureAddClip != NULL) {
        VK_DestroyShader(multiTextureAddClip);
        free(multiTextureAddClip);
        multiTextureAddClip = NULL;
    }
	if (clearAttachment != NULL) {
		VK_DestroyShader(clearAttachment);
		free(clearAttachment);
		clearAttachment = NULL;
	}
	if (fullscreenRect != NULL) {
		VK_DestroyShader(fullscreenRect);
		free(fullscreenRect);
		fullscreenRect = NULL;
	}
	// RTX
	if (rayTracing != NULL) {
		VK_DestroyShader(rayTracing);
		free(rayTracing);
		rayTracing = NULL;
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

// Vertex and Fragment Shader
void VK_LoadVertFragShadersFromVariable(vkshader_t* shader, const char* vertexSPV, const uint32_t sizeVert, const char* fragmentSPV, const uint32_t sizeFrag) {
	shader->size = 2;
	shader->modules = malloc(shader->size * sizeof(VkShaderModule));
	shader->flags = malloc(shader->size * sizeof(VkShaderStageFlagBits));
	shader->shaderStageCreateInfos = calloc(shader->size, sizeof(VkPipelineShaderStageCreateInfo));

	shader->flags[0] = VK_SHADER_STAGE_VERTEX_BIT;
	shader->flags[1] = VK_SHADER_STAGE_FRAGMENT_BIT;

	VK_CreateShaderModule(&shader->modules[0], vertexSPV, sizeVert);
	VK_CreateShaderModule(&shader->modules[1], fragmentSPV, sizeFrag);

	shader->shaderStageCreateInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader->shaderStageCreateInfos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shader->shaderStageCreateInfos[0].module = shader->modules[0];
	shader->shaderStageCreateInfos[0].pName = "main";

	shader->shaderStageCreateInfos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader->shaderStageCreateInfos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shader->shaderStageCreateInfos[1].module = shader->modules[1];
	shader->shaderStageCreateInfos[1].pName = "main";
}

void VK_LoadVertFragShadersFromFile(vkshader_t *shader, const char *vertexSPV, const char *fragmentSPV){
    shader->size = 2;
    shader->modules = malloc(shader->size * sizeof(VkShaderModule));
    shader->flags = malloc(shader->size * sizeof(VkShaderStageFlagBits));
    shader->shaderStageCreateInfos = calloc(shader->size, sizeof(VkPipelineShaderStageCreateInfo));
    
    shader->flags[0] = VK_SHADER_STAGE_VERTEX_BIT;
    shader->flags[1] = VK_SHADER_STAGE_FRAGMENT_BIT;
    
    unsigned char vertSPV[2000] = {0};
    unsigned char fragSPV[2000] = {0};
    size_t vertSize;
    size_t fragSize;
    
    FILE *fp;
    fp = fopen(vertexSPV, "rb");
    if(!fp) ri.Error(ERR_FATAL, "Vulkan: could not open Shader %s", vertexSPV);
    
    fseek (fp, 0, SEEK_END);
	vertSize = ftell(fp);
    fseek (fp, 0, SEEK_SET);
    fread(vertSPV,sizeof(vertSPV),1,fp);
    fclose(fp);
    
    
    fp = fopen(fragmentSPV, "rb");
    if(!fp) ri.Error(ERR_FATAL, "Vulkan: could not open Shader %s", fragmentSPV);
    
    fseek (fp, 0, SEEK_END);
	fragSize = ftell(fp);
    fseek (fp, 0, SEEK_SET);
    fread(fragSPV,sizeof(fragSPV),1,fp);
    fclose(fp);
    
    VK_CreateShaderModule(&shader->modules[0], vertSPV, vertSize);
    VK_CreateShaderModule(&shader->modules[1], fragSPV, fragSize);
    
    shader->shaderStageCreateInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader->shaderStageCreateInfos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shader->shaderStageCreateInfos[0].module = shader->modules[0];
    shader->shaderStageCreateInfos[0].pName = "main";
    
    shader->shaderStageCreateInfos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader->shaderStageCreateInfos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shader->shaderStageCreateInfos[1].module = shader->modules[1];
    shader->shaderStageCreateInfos[1].pName = "main";
}

// RTX
void VK_LoadRayTracingShadersFromVariable(vkshader_t* shader, const char* rgenSPV, const uint32_t sizeRGEN, 
																const char* rmissSPV, const uint32_t sizeRMISS,
																const char* rhitSPV, const uint32_t sizeRHIT) {
	shader->size = 3;
	shader->modules = calloc(shader->size, sizeof(VkShaderModule));
	shader->flags = calloc(shader->size, sizeof(VkShaderStageFlagBits));
	shader->shaderStageCreateInfos = calloc(shader->size, sizeof(VkPipelineShaderStageCreateInfo));

	shader->flags[0] = VK_SHADER_STAGE_RAYGEN_BIT_NV;
	shader->flags[1] = VK_SHADER_STAGE_MISS_BIT_NV;
	shader->flags[2] = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;

	VK_CreateShaderModule(&shader->modules[0], rgenSPV, sizeRGEN);
	VK_CreateShaderModule(&shader->modules[1], rmissSPV, sizeRMISS);
	VK_CreateShaderModule(&shader->modules[2], rhitSPV, sizeRHIT);

	shader->shaderStageCreateInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader->shaderStageCreateInfos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
	shader->shaderStageCreateInfos[0].module = shader->modules[0];
	shader->shaderStageCreateInfos[0].pName = "main";

	shader->shaderStageCreateInfos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader->shaderStageCreateInfos[1].stage = VK_SHADER_STAGE_MISS_BIT_NV;
	shader->shaderStageCreateInfos[1].module = shader->modules[1];
	shader->shaderStageCreateInfos[1].pName = "main";

	shader->shaderStageCreateInfos[2].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader->shaderStageCreateInfos[2].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
	shader->shaderStageCreateInfos[2].module = shader->modules[2];
	shader->shaderStageCreateInfos[2].pName = "main";

}