#include "../tr_local.h"

// SHADER HEADER
#include "../../../shader/header/clearAttachment.vert.h"
#include "../../../shader/header/clearAttachment.frag.h"

#include "../../../shader/header/fullscreenRect.vert.h"
#include "../../../shader/header/fullscreenRect.frag.h"

#include "../../../shader/header/texture.vert.h"
#include "../../../shader/header/texture.frag.h"

#include "../../../shader/header/rng.comp.h"

// RTX
#include "../../../shader/header/rt_raygen.rgen.h"
#include "../../../shader/header/rt_miss.rmiss.h"
#include "../../../shader/header/rt_closesthit.rchit.h"
#include "../../../shader/header/rt_anyhit.rahit.h"

#include "../../../shader/header/rt_shadow.rmiss.h"
#include "../../../shader/header/rt_shadow.rchit.h"
#include "../../../shader/header/rt_shadow.rahit.h"

static vkshader_t *texture;
static vkshader_t *clearAttachment;
static vkshader_t *fullscreenRect;
static vkshader_t* rngCompShader;
// rtx
static vkshader_t* rayTracing;
static vkshader_t* rayTracingAny;

void VK_CreateShaderModule(VkShaderModule *handle, const char *code, size_t size);
void VK_LoadVertFragShadersFromFile(vkshader_t *shader, const char *vertexSPV, const char *fragmentSPV);
void VK_LoadVertFragShadersFromVariable(vkshader_t* shader, const char* vertexSPV, const uint32_t sizeVert, const char* fragmentSPV, const uint32_t sizeFrag);
void VK_LoadCompShaderFromVariable(vkshader_t* shader, const char* compSPV, const uint32_t sizeComp);
void VK_LoadRayTracingShadersFromVariable(vkshader_t* shader, const char* rgenSPV, const uint32_t sizeRGEN,
	const char* rmissSPV, const uint32_t sizeRMISS,
	const char* rhitSPV, const uint32_t sizeRHIT);

void VK_SingleTextureShader(vkshader_t *shader){
    if (texture == NULL) {
		texture = malloc(sizeof(vkshader_t));
		//VK_LoadVertFragShadersFromFile(singleTexture, "../../shader/spv/singleTexture.vert.spv", "../../shader/spv/singleTexture.frag.spv");
		VK_LoadVertFragShadersFromVariable(texture, &textureVert, sizeof(textureVert), &textureFrag, sizeof(textureFrag));
    }
    Com_Memcpy(shader, texture, sizeof(vkshader_t));
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

void VK_RngCompShader(vkshader_t* shader) {
	if (rngCompShader == NULL) {
		rngCompShader = malloc(sizeof(vkshader_t));
		VK_LoadCompShaderFromVariable(rngCompShader, &rngComp, sizeof(rngComp));
	}
	Com_Memcpy(shader, rngCompShader, sizeof(vkshader_t));
}

// rtx
void VK_RayTracingShader(vkshader_t* shader) {
	if (rayTracing == NULL) {
		rayTracing = malloc(sizeof(vkshader_t));
		VK_LoadRayTracingShadersFromVariable(rayTracing, &rt_raygenRGen, sizeof(rt_raygenRGen), &rt_missRMiss, sizeof(rt_missRMiss), &rt_closesthitRCHit, sizeof(rt_closesthitRCHit));
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
	if (texture != NULL) {
		VK_DestroyShader(texture);
		free(texture);
		texture = NULL;
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
	if (rayTracingAny != NULL) {
		VK_DestroyShader(rayTracingAny);
		free(rayTracingAny);
		rayTracingAny = NULL;
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

void VK_LoadCompShaderFromVariable(vkshader_t* shader, const char* compSPV, const uint32_t sizeComp) {
	shader->size = 1;
	shader->modules = malloc(shader->size * sizeof(VkShaderModule));
	shader->flags = malloc(shader->size * sizeof(VkShaderStageFlagBits));
	shader->shaderStageCreateInfos = calloc(shader->size, sizeof(VkPipelineShaderStageCreateInfo));

	shader->flags[0] = VK_SHADER_STAGE_COMPUTE_BIT;

	VK_CreateShaderModule(&shader->modules[0], compSPV, sizeComp);

	shader->shaderStageCreateInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader->shaderStageCreateInfos[0].stage = VK_SHADER_STAGE_COMPUTE_BIT;
	shader->shaderStageCreateInfos[0].module = shader->modules[0];
	shader->shaderStageCreateInfos[0].pName = "main";
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
void VK_LoadRayTracingShadersWithAnyFromVariable(vkshader_t* shader, const char* rgenSPV, const uint32_t sizeRGEN,
	const char* rmissSPV, const uint32_t sizeRMISS,
	const char* rhitSPV, const uint32_t sizeRHIT,
	const char* rahitSPV, const uint32_t sizeRAHIT,
	// shadow
	const char* rmissShadowSPV, const uint32_t sizeRMISSShadow,
	const char* rchitShadowSPV, const uint32_t sizeRCHITShadow,
	const char* rahitShadowSPV, const uint32_t sizeRAHITShadow) {
	shader->size = 7;
	shader->modules = calloc(shader->size, sizeof(VkShaderModule));
	shader->flags = calloc(shader->size, sizeof(VkShaderStageFlagBits));
	shader->shaderStageCreateInfos = calloc(shader->size, sizeof(VkPipelineShaderStageCreateInfo));

	shader->flags[0] = VK_SHADER_STAGE_RAYGEN_BIT_NV;
	shader->flags[1] = VK_SHADER_STAGE_MISS_BIT_NV;
	shader->flags[2] = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
	shader->flags[3] = VK_SHADER_STAGE_ANY_HIT_BIT_NV;
	// SHADOW
	shader->flags[4] = VK_SHADER_STAGE_MISS_BIT_NV;
	shader->flags[5] = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
	shader->flags[6] = VK_SHADER_STAGE_ANY_HIT_BIT_NV;

	VK_CreateShaderModule(&shader->modules[0], rgenSPV, sizeRGEN);
	VK_CreateShaderModule(&shader->modules[1], rmissSPV, sizeRMISS);
	VK_CreateShaderModule(&shader->modules[2], rhitSPV, sizeRHIT);
	VK_CreateShaderModule(&shader->modules[3], rahitSPV, sizeRAHIT);
	// SHADOW
	VK_CreateShaderModule(&shader->modules[4], rmissShadowSPV, sizeRMISSShadow);
	VK_CreateShaderModule(&shader->modules[5], rchitShadowSPV, sizeRCHITShadow);
	VK_CreateShaderModule(&shader->modules[6], rahitShadowSPV, sizeRAHITShadow);

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

	shader->shaderStageCreateInfos[3].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader->shaderStageCreateInfos[3].stage = VK_SHADER_STAGE_ANY_HIT_BIT_NV;
	shader->shaderStageCreateInfos[3].module = shader->modules[3];
	shader->shaderStageCreateInfos[3].pName = "main";

	shader->shaderStageCreateInfos[4].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader->shaderStageCreateInfos[4].stage = VK_SHADER_STAGE_MISS_BIT_NV;
	shader->shaderStageCreateInfos[4].module = shader->modules[4];
	shader->shaderStageCreateInfos[4].pName = "main";

	shader->shaderStageCreateInfos[5].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader->shaderStageCreateInfos[5].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
	shader->shaderStageCreateInfos[5].module = shader->modules[5];
	shader->shaderStageCreateInfos[5].pName = "main";

	shader->shaderStageCreateInfos[6].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader->shaderStageCreateInfos[6].stage = VK_SHADER_STAGE_ANY_HIT_BIT_NV;
	shader->shaderStageCreateInfos[6].module = shader->modules[6];
	shader->shaderStageCreateInfos[6].pName = "main";

}

void VK_RayTracingShaderWithAny(vkshader_t* shader) {
	if (rayTracingAny == NULL) {
		rayTracingAny = malloc(sizeof(vkshader_t));
		VK_LoadRayTracingShadersWithAnyFromVariable(rayTracingAny, &rt_raygenRGen, sizeof(rt_raygenRGen), &rt_missRMiss, sizeof(rt_missRMiss), &rt_closesthitRCHit, sizeof(rt_closesthitRCHit), &rt_anyhitRAHit, sizeof(rt_anyhitRAHit),
			&rt_shadowRMiss, sizeof(rt_shadowRMiss), &rt_shadowRCHit, sizeof(rt_shadowRCHit) ,&rt_shadowRAHit, sizeof(rt_shadowRAHit));
	}
	Com_Memcpy(shader, rayTracingAny, sizeof(vkshader_t));
}