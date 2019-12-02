#include "tr_local.h"
#include "../../shader/glsl/constants.h"

/*
glConfig.driverType == VULKAN && r_vertexLight->value == 2
*/

#define ANIMATE_TEXTURE (tess.shader->stages[0]->bundle[0].numImageAnimations > 0)
#define UV_CHANGES		(tess.shader->stages[0] != NULL ? ((tess.shader->stages[0]->bundle[0].tcGen != TCGEN_BAD) && tess.shader->stages[0]->bundle[0].numTexMods > 0) : qfalse)
#define MODEL_DEFORM	(tess.shader->numDeforms > 0)
#define ANIMATE_MODEL	(backEnd.currentEntity->e.frame > 0 || backEnd.currentEntity->e.oldframe > 0)

#define RTX_DYNAMIC_AS		(MODEL_DEFORM || ANIMATE_MODEL)
#define RTX_DYNAMIC_AS_DATA (RTX_DYNAMIC_AS || UV_CHANGES)

static void RB_WriteIDX(uint32_t offset, qboolean dynamic) {
	for (int j = 0; j < tess.numIndexes; j++) {
		uint32_t idx = (uint32_t)tess.indexes[j];
		if(!dynamic) VK_UploadBufferDataOffset(&vk_d.geometry.idx_static, offset + (j * sizeof(uint32_t)), sizeof(uint32_t), (void*)&idx);
		else VK_UploadBufferDataOffset(&vk_d.geometry.idx_dynamic[vk.swapchain.currentImage], offset + (j * sizeof(uint32_t)), sizeof(uint32_t), (void*)&idx);
	}
}
static void RB_WriteXYZ(uint32_t offset, qboolean dynamic) {
	for (int j = 0; j < tess.numVertexes; j++) {
		float p[12] = {
			p[0] = tess.xyz[j][0],
			p[1] = tess.xyz[j][1],
			p[2] = tess.xyz[j][2],
			p[3] = 0,
			p[4] = UV_CHANGES ? tess.svars.texcoords[0][j][0] : tess.texCoords[j][0][0],
			p[5] = UV_CHANGES ? tess.svars.texcoords[0][j][1] : tess.texCoords[j][0][1],
			p[6] = tess.texCoords[j][1][0],
			p[7] = tess.texCoords[j][1][1],
			p[8] = (float)tess.svars.colors[j][0],
			p[9] = (float)tess.svars.colors[j][1],
			p[10] = (float)tess.svars.colors[j][2],
			p[11] = 0
		};
		if (!dynamic) VK_UploadBufferDataOffset(&vk_d.geometry.xyz_static, offset + (j * 12 * sizeof(float)), 12 * sizeof(float), (void*)&p);
		else VK_UploadBufferDataOffset(&vk_d.geometry.xyz_dynamic[vk.swapchain.currentImage], offset + (j * 12 * sizeof(float)), 12 * sizeof(float), (void*)&p);
	}
}

void RB_CreateStaticBottomAS(vkbottomAS_t** bAS) {
	int j;
	//if (MODEL_DEFORM) return;

	for (int stage = 0; stage < 1/*MAX_SHADER_STAGES*/; stage++)
	{
		shaderStage_t* pStage = tess.shader->stages[stage];
		if (!pStage || pStage->bundle[0].isLightmap || !pStage->active) continue;

		ComputeColors(pStage);
		if(UV_CHANGES) ComputeTexCoords(pStage);
		
		// set offsets and 
		uint32_t* idxOffset;
		uint32_t* xyzOffset;
		// save bas in static or dynamic list
		vkbottomAS_t* bASList;
		{
			idxOffset = &vk_d.geometry.idx_static_offset;
			xyzOffset = &vk_d.geometry.xyz_static_offset;
			bASList = &vk_d.bottomASList[vk_d.bottomASCount];
		}

		//define as geometry
		bASList->geometries.sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
		bASList->geometries.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
		bASList->geometries.geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
		bASList->geometries.geometry.triangles.vertexCount = tess.numVertexes;
		bASList->geometries.geometry.triangles.vertexStride = 12 * sizeof(float);
		bASList->geometries.geometry.triangles.indexCount = tess.numIndexes;
		bASList->geometries.geometry.triangles.vertexOffset = (*xyzOffset) * sizeof(float[12]);
		bASList->geometries.geometry.triangles.indexOffset = (*idxOffset) * sizeof(uint32_t);
		{
			bASList->geometries.geometry.triangles.vertexData = vk_d.geometry.xyz_static.buffer;
			bASList->geometries.geometry.triangles.indexData = vk_d.geometry.idx_static.buffer;
		}
		bASList->geometries.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
		bASList->geometries.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
		bASList->geometries.geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
		bASList->geometries.flags = 0;
		
		bASList->data.offsetIDX = (*idxOffset);
		bASList->data.offsetXYZ = (*xyzOffset);
		// write idx
		RB_WriteIDX(bASList->data.offsetIDX * sizeof(uint32_t), qfalse);
		// write xyz and other vertex attribs
		RB_WriteXYZ(bASList->data.offsetXYZ * 12 * sizeof(float), qfalse);

		VkCommandBuffer commandBuffer = { 0 };
		VK_BeginSingleTimeCommands(&commandBuffer);
		{
			VK_CreateBottomAS(commandBuffer, bASList, &vk_d.basBufferStatic, &vk_d.basBufferStaticOffset, VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV);
		}
		VK_EndSingleTimeCommands(&commandBuffer);

		vk_d.bottomASCount++;
		
		if(bAS != NULL) (*bAS) = bASList;
		//(*bAS)->dynamic = qfalse;
		(*idxOffset) += 3* tess.numIndexes;
		(*xyzOffset) += 3* tess.numVertexes;
	}
}
//
//void RB_CreateDynamicBottomAS(vkbottomAS_t* bAS) {
//	int j;
//
//	for (int stage = 0; stage < 1/*MAX_SHADER_STAGES*/; stage++)
//	{
//		shaderStage_t* pStage = tess.shader->stages[stage];
//		if (!pStage || pStage->bundle[0].isLightmap || !pStage->active) continue;
//
//		ComputeColors(pStage);
//		if (UV_CHANGES) ComputeTexCoords(pStage);
//
//		// set offsets and 
//		uint32_t* idxOffset;
//		uint32_t* xyzOffset;
//		// save bas in static or dynamic list
//		vkbottomAS_t* bASList;
//		
//		idxOffset = &vk_d.geometry.idx_dynamic_offset;
//		xyzOffset = &vk_d.geometry.xyz_dynamic_offset;
//		bASList = &vk_d.bottomASDynamicList[vk.swapchain.currentImage][vk_d.bottomASDynamicCount[vk.swapchain.currentImage]];
//		
//
//		//define as geometry
//		bASList->geometries.sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
//		bASList->geometries.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
//		bASList->geometries.geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
//		bASList->geometries.geometry.triangles.vertexCount = tess.numVertexes;
//		bASList->geometries.geometry.triangles.vertexStride = 12 * sizeof(float);
//		bASList->geometries.geometry.triangles.indexCount = tess.numIndexes;
//		bASList->geometries.geometry.triangles.vertexOffset = (*xyzOffset) * sizeof(float[12]);
//		bASList->geometries.geometry.triangles.indexOffset = (*idxOffset) * sizeof(uint32_t);
//		bASList->geometries.geometry.triangles.vertexData = vk_d.geometry.xyz_dynamic[vk.swapchain.currentImage].buffer;
//		bASList->geometries.geometry.triangles.indexData = vk_d.geometry.idx_dynamic[vk.swapchain.currentImage].buffer;
//		
//		bASList->geometries.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
//		bASList->geometries.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
//		bASList->geometries.geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
//		bASList->geometries.flags = 0;
//
//		bASList->data.offsetIDX = (*idxOffset);
//		bASList->data.offsetXYZ = (*xyzOffset);
//		// write idx
//		RB_WriteIDX(bASList->data.offsetIDX, qtrue);
//		// write xyz and other vertex attribs
//		RB_WriteXYZ(bASList->data.offsetXYZ, qtrue);
//
//		VkCommandBuffer commandBuffer = { 0 };
//		VK_BeginSingleTimeCommands(&commandBuffer);
//		{
//			VK_CreateBottomAS(commandBuffer, bASList, &vk_d.basBufferDynamic[vk.swapchain.currentImage], &vk_d.basBufferDynamicOffset, VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV);
//		}
//		VK_EndSingleTimeCommands(&commandBuffer);
//
//		vk_d.bottomASDynamicCount[vk.swapchain.currentImage]++;
//
//		//if (bAS != NULL) 
//		bASList->dynamic = qtrue;
//		bASList->data.dynamic = qtrue;
//		(bAS) = bASList;
//		(*idxOffset) += tess.numIndexes;
//		(*xyzOffset) += tess.numVertexes;
//	}
//}


void RB_CreateBottomAS(vkbottomAS_t** bAS, qboolean multi) {
	/*if (RTX_DYNAMIC_AS) RB_CreateDynamicBottomAS(bAS);
	else*/ 
	RB_CreateStaticBottomAS(bAS);
	if (RTX_DYNAMIC_AS && multi) {
		vk_d.bottomASList[vk_d.bottomASCount-1].dynamic = qtrue;
		RB_CreateStaticBottomAS(NULL);
		vk_d.bottomASList[vk_d.bottomASCount - 1].dynamic = qtrue;
		RB_CreateStaticBottomAS(NULL);
		vk_d.bottomASList[vk_d.bottomASCount - 1].dynamic = qtrue;
	}
}

static void RB_AddLightToLightList() {
	vec4_t pos = {0,0,0,0};

	for (int i = 0; i < tess.numVertexes; i++) {
		VectorAdd(pos, tess.xyz[i], pos);
	}
	VectorScale(pos, 1.0f / tess.numVertexes, pos);
	if (vk_d.lightCount >= RTX_MAX_LIGHTS) {
		ri.Error(ERR_FATAL, "Vulkan: Too many lights");
	}
	VectorCopy(pos, vk_d.lightList[vk_d.lightCount]);
	VK_UploadBufferDataOffset(&vk_d.uboLightList[vk.swapchain.currentImage], vk_d.lightCount * sizeof(vec4_t), 1 * sizeof(vec4_t), (void*)&vk_d.lightList[0]);
	vk_d.lightCount++;
	VK_UploadBufferDataOffset(&vk_d.uboLightList[vk.swapchain.currentImage], RTX_MAX_LIGHTS * sizeof(vec4_t), 1 * sizeof(uint32_t), (void*)&vk_d.lightCount);
}

static qboolean RB_MaterialException(vkbottomAS_t* bAS) {
	// -- lights --
	if (strstr(tess.shader->name, "base_light") || strstr(tess.shader->name, "gothic_light")) { // all lamp textures
		bAS->data.material = MATERIAL_KIND_REGULAR;
		bAS->data.material |= MATERIAL_FLAG_LIGHT;
		RB_AddLightToLightList();
	}
	else
	if (strstr(tess.shader->name, "flame")) { // all fire textures
		bAS->data.material = MATERIAL_KIND_REGULAR;
		bAS->data.material |= MATERIAL_FLAG_LIGHT;
		RB_AddLightToLightList();
	}
	//else
	//if (strstr(tess.shader->name, "beam")  /* || (strstr(tess.shader->name, "lamp") && strstr(tess.shader->name, "flare"))*/ ) { // light rect and cones (beam == cones, lamp = squares)
	//	bAS->data.material = MATERIAL_KIND_INVISIBLE;
	//	bAS->data.material |= MATERIAL_FLAG_LIGHT;
	//}
	else
	// -- glass --
	if (strstr(tess.shader->name, "glass") || strstr(tess.shader->name, "jacobs") || strstr(tess.shader->name, "green_sphere") || strstr(tess.shader->name, "yellow_sphere") || strstr(tess.shader->name, "red_sphere")) { // glass (jacobs = console glass, green sphere = life)
		bAS->data.material = MATERIAL_KIND_GLASS;
		//bAS->data.material |= MATERIAL_FLAG_LIGHT;
	} 
	else
	if (tess.shader->sort == SS_BLEND0) {
		int x = 2;
		//bAS->data.material == MATERIAL_KIND_GLASS;
	}
	//	else
	//if (strstr(tess.shader->name, "gratelamp/gratelamp") && !strstr(tess.shader->name, "flare") && !strstr(tess.shader->name, "_b")) {
	//	bAS->data.material == MATERIAL_KIND_INVISIBLE;
	//	//bAS->data.material |= MATERIAL_FLAG_LIGHT;
	//}
	//else
	//if (backEnd.currentEntity->e.reType == (RT_SPRITE) &&
	//	(strstr(tess.shader->name, "rocketExplosion") || strstr(tess.shader->name, "plasma1") || strstr(tess.shader->name, "grenadeExplosion") || strstr(tess.shader->name, "bfgExplosion"))) {
	//	//bAS->data.material |= MATERIAL_KIND_BULLET;
	//	bAS->data.material |= MATERIAL_FLAG_NEEDSCOLOR;
	//}
	//else if (backEnd.currentEntity->e.reType == RT_RAIL_CORE || backEnd.currentEntity->e.reType == RT_RAIL_RINGS || backEnd.currentEntity->e.reType == RT_LIGHTNING) {
	//	//bAS->data.material |= MATERIAL_KIND_BULLET;
	//	bAS->data.material |= MATERIAL_FLAG_NEEDSCOLOR;
	//}
	//else if (strstr(tess.shader->name, "railExplosion")) {
	//	bAS->data.material |= MATERIAL_FLAG_NEEDSCOLOR;
	//	bAS->data.material |= MATERIAL_FLAG_TRANSPARENT;
	//}
	//else if (tess.shader->sort == SS_DECAL) {
	//	//bAS->data.material |= MATERIAL_FLAG_NEEDSCOLOR;
	//	bAS->data.material |= MATERIAL_FLAG_BULLET_MARK;
	//}else if (strstr(tess.shader->name, "hologirl")) {
	//	bAS->data.material = MATERIAL_FLAG_SEE_THROUGH;
	//	//bAS->data.material |= MATERIAL_FLAG_NEEDSCOLOR;
	//}
	//else if (strstr(tess.shader->name, "gratelamp/gratelamp") && !strstr(tess.shader->name, "flare") && !strstr(tess.shader->name, "_b")) {
	//	bAS->data.material = MATERIAL_FLAG_SEE_THROUGH;
	//}
	//else if (strstr(tess.shader->name, "flare") || strstr(tess.shader->name, "textures/sfx/beam")) {
	//	bAS->data.material = MATERIAL_FLAG_LIGHT;
	//}
	//else if (strstr(tess.shader->name, "railgun")) bAS->data.material |= MATERIAL_FLAG_NEEDSCOLOR;
	else return qfalse;
	return qtrue;
}

void RB_UpdateInstanceDataBuffer(vkbottomAS_t* bAS) {
	// set texture id and calc texture animation
	int indexAnim = 0;
	if (tess.shader->stages[0]->bundle[0].numImageAnimations > 1) {
		indexAnim = (int)(tess.shaderTime * tess.shader->stages[0]->bundle[0].imageAnimationSpeed * FUNCTABLE_SIZE);
		indexAnim >>= FUNCTABLE_SIZE2;
		if (indexAnim < 0) {
			indexAnim = 0;	// may happen with shader time offsets
		}
		indexAnim %= tess.shader->stages[0]->bundle[0].numImageAnimations;	
	}
	if (bAS->data.texIdx != (uint32_t)tess.shader->stages[0]->bundle[0].image[indexAnim]->index) {
		bAS->data.texIdx = (uint32_t)tess.shader->stages[0]->bundle[0].image[indexAnim]->index;
		tess.shader->stages[0]->bundle[0].image[indexAnim]->frameUsed = tr.frameCount;
	}

	bAS->data.blendfunc = (uint32_t)(tess.shader->stages[0]->stateBits);
	bAS->data.opaque = tess.shader->sort;

	// set material
	if (!RB_MaterialException(bAS)) {

		bAS->data.material &= 0xfffffff0;
		switch (tess.shader->contentFlags & 0x0000007f) {
		case CONTENTS_SOLID: bAS->data.material |= MATERIAL_KIND_REGULAR; break;
		case CONTENTS_LAVA: bAS->data.material |= MATERIAL_KIND_LAVA; break;
		case CONTENTS_SLIME: bAS->data.material |= MATERIAL_KIND_SLIME; break;
		case CONTENTS_WATER: bAS->data.material |= MATERIAL_KIND_WATER; break;
		case CONTENTS_FOG: bAS->data.material |= MATERIAL_KIND_FOG; break;
		default: bAS->data.material |= MATERIAL_KIND_INVALID; break;
		}

		if (tess.shader->sort == SS_PORTAL && strstr(tess.shader->name, "mirror") != NULL) bAS->data.material |= MATERIAL_FLAG_MIRROR;
		//else if (tess.shader->sort == SS_PORTAL && strstr(tess.shader->name, "mirror") == NULL) bAS->data.material |= MATERIAL_FLAG_PORTAL;
		////if (tess.shader->sort <= SS_OPAQUE) bAS->data.material |= MATERIAL_FLAG_OPAQUE;
		//if (tess.shader->sort == SS_BLEND0 || tess.shader->sort == SS_BLEND1) bAS->data.material |= MATERIAL_FLAG_TRANSPARENT;
		//if ((tess.shader->contentFlags & CONTENTS_TRANSLUCENT) == CONTENTS_TRANSLUCENT) {
		//	bAS->data.material |= MATERIAL_FLAG_SEE_THROUGH;
		//}
		//if (tess.shader->sort <= SS_OPAQUE && tess.shader->contentFlags != CONTENTS_TRANSLUCENT /*!strstr(tess.shader->stages[0]->bundle->image[0]->imgName, "proto_grate4.tga")*/) bAS->data.material |= MATERIAL_FLAG_OPAQUE;
	}

	VK_UploadBufferDataOffset(&vk_d.instanceDataBuffer[vk.swapchain.currentImage], vk_d.bottomASTraceListCount * sizeof(ASInstanceData), sizeof(ASInstanceData), (void*)&bAS->data);
}

void RB_UpdateInstanceBuffer(vkbottomAS_t* bAS) {
	bAS->geometryInstance.instanceCustomIndex = 0;
	// set visibility for first and third person (eye and mirror)
	if ((backEnd.currentEntity->e.renderfx & RF_THIRD_PERSON)) bAS->geometryInstance.mask = RAY_MIRROR_OPAQUE_VISIBLE;
	else if ((backEnd.currentEntity->e.renderfx & RF_FIRST_PERSON)) bAS->geometryInstance.mask = RAY_FIRST_PERSON_OPAQUE_VISIBLE;
	else bAS->geometryInstance.mask = RAY_FIRST_PERSON_MIRROR_OPAQUE_VISIBLE;

	if (tess.shader->sort <= SS_OPAQUE) {
		bAS->geometries.flags = VK_GEOMETRY_OPAQUE_BIT_NV;
	}
	else {
		bAS->geometries.flags = 0;
	}

	/*if (bAS->data.material & MATERIAL_FLAG_PARTICLE || tess.shader->sort == SS_BLEND0 || tess.shader->sort == SS_BLEND1 || tess.shader->sort == SS_DECAL) {
		bAS->geometryInstance.instanceOffset = 1;
		if ((backEnd.currentEntity->e.renderfx & RF_THIRD_PERSON)) bAS->geometryInstance.mask = RAY_MIRROR_PARTICLE_VISIBLE;
		else if ((backEnd.currentEntity->e.renderfx & RF_FIRST_PERSON)) bAS->geometryInstance.mask = RAY_FIRST_PERSON_PARTICLE_VISIBLE;
		else bAS->geometryInstance.mask = RAY_FIRST_PERSON_MIRROR_PARTICLE_VISIBLE;
	}
	else {
		bAS->geometryInstance.instanceOffset = 0;
	}*/

	switch (tess.shader->cullType) {
		case CT_FRONT_SIDED:
			bAS->geometryInstance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FRONT_COUNTERCLOCKWISE_BIT_NV; break;
		case CT_BACK_SIDED:
			bAS->geometryInstance.flags = 0; break;
		case CT_TWO_SIDED:
			bAS->geometryInstance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV; break;
	}
	bAS->geometryInstance.accelerationStructureHandle = bAS->handle;

	VK_UploadBufferDataOffset(&vk_d.instanceBuffer[vk.swapchain.currentImage], vk_d.bottomASTraceListCount * sizeof(VkGeometryInstanceNV), sizeof(VkGeometryInstanceNV), (void*)&bAS->geometryInstance);
}

void RB_AddBottomAS(vkbottomAS_t* bAS, qboolean dynamic, qboolean forceUpdate) {
	shaderCommands_t* input  = &tess;
	vkbottomAS_t* currentAS;

	if (input->numIndexes == 0) {
		return;
	}
	if (tess.shader == tr.shadowShader) return;
	
	// for debugging of sort order issues, stop rendering after a given sort value
	if (r_debugSort->integer && r_debugSort->integer < tess.shader->sort) return;
	if (tess.shader->stages[0] == NULL) return;

	// just stuff
	if (!strcmp(tess.shader->name, "textures/common/mirror2")) {
		int aaaaa = 2;
	}
	if (input->shader->sort == SS_PORTAL) {
		int x = 2l;
	}
	if (input->shader->sort == SS_STENCIL_SHADOW) {
		int x = 2l;
	}
	
	qboolean anim = tess.shader->stages[0]->bundle[0].numImageAnimations > 0;
	qboolean cTex = tess.shader->stages[0] != NULL ? ((tess.shader->stages[0]->bundle[0].tcGen != TCGEN_BAD) && tess.shader->stages[0]->bundle[0].numTexMods > 0) : qfalse;
	qboolean deform = tess.shader->numDeforms > 0;
	qboolean frames = backEnd.currentEntity->e.frame > 0 || backEnd.currentEntity->e.oldframe > 0;
	//if (!dynamic) {
	//	currentAS = bAS;
	//}
	//else {
	//	currentAS = &vk_d.bottomASDynamicList[vk.swapchain.currentImage][vk_d.bottomASDynamicCount[vk.swapchain.currentImage]];
	//	currentAS->geometries = bAS->geometries;
	//	currentAS->data = bAS->data;
	//	currentAS->geometryInstance = bAS->geometryInstance;
	//	currentAS->offset = vk_d.basBufferDynamicOffset;

	//	currentAS->geometries.geometry.triangles.vertexData = vk_d.geometry.xyz_dynamic[vk.swapchain.currentImage].buffer;
	//	currentAS->geometries.geometry.triangles.indexData = vk_d.geometry.idx_dynamic[vk.swapchain.currentImage].buffer;
	//	currentAS->geometries.geometry.triangles.indexOffset = vk_d.geometry.idx_dynamic_offset * sizeof(uint32_t);
	//	currentAS->geometries.geometry.triangles.vertexOffset = vk_d.geometry.xyz_dynamic_offset * sizeof(float[12]);
	//	currentAS->data.offsetIDX = vk_d.geometry.idx_dynamic_offset;
	//	currentAS->data.offsetXYZ = vk_d.geometry.xyz_dynamic_offset;
	//	currentAS->data.dynamic = qtrue;
	//	

	//	vk_d.bottomASDynamicCount[vk.swapchain.currentImage]++;
	//	//return;
	//}

	//// calculate new data if necessary
	//if (deform) RB_DeformTessGeometry();
	//if (cTex) {
	//	ComputeTexCoords(tess.shader->stages[0]);
	//	bAS->data.material |= MATERIAL_FLAG_NEEDSCOLOR;
	//}
	//ComputeColors(tess.shader->stages[0]);
	//
	//// update buffer (for dynamic we always need to update the dyn buffer)
	//if (dynamic || frames || deform || cTex) {
	//	RB_WriteIDX(currentAS->geometries.geometry.triangles.indexOffset, dynamic);
	//	RB_WriteXYZ(currentAS->geometries.geometry.triangles.vertexOffset, dynamic);

	//	if (!dynamic) {

	//		qboolean updateBottom = (currentAS->geometries.geometry.triangles.vertexCount >= tess.numVertexes &&
	//			currentAS->geometries.geometry.triangles.indexCount == tess.numIndexes);

	//		currentAS->geometries.geometry.triangles.vertexCount = tess.numVertexes;
	//		currentAS->geometries.geometry.triangles.indexCount = tess.numIndexes;

	//		if (updateBottom) VK_UpdateBottomAS(vk.swapchain.CurrentCommandBuffer(), currentAS, currentAS, &vk_d.basBufferStatic, VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV, NULL);
	//		else  VK_RecreateBottomAS(vk.swapchain.CurrentCommandBuffer(), currentAS, &vk_d.basBufferStatic, VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV);
	//	}
	//	else {
	//		currentAS->geometries.geometry.triangles.vertexCount = tess.numVertexes;
	//		currentAS->geometries.geometry.triangles.indexCount = tess.numIndexes;
	//		vk_d.geometry.idx_dynamic_offset += tess.numIndexes;
	//		vk_d.geometry.xyz_dynamic_offset += tess.numVertexes;

	//		VK_CreateBottomAS(vk.swapchain.CurrentCommandBuffer(), currentAS, &vk_d.basBufferDynamic[vk.swapchain.currentImage], &vk_d.basBufferDynamicOffset, VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV);
	//	}
	//}
	
	if (!dynamic) {
		currentAS = bAS;
	}
	else {
		if (deform || frames || forceUpdate) {
			currentAS = &vk_d.bottomASDynamicList[vk.swapchain.currentImage][vk_d.bottomASDynamicCount[vk.swapchain.currentImage]];
			currentAS->geometries = bAS->geometries;
			currentAS->data = bAS->data;
			currentAS->geometryInstance = bAS->geometryInstance;
			currentAS->offset = vk_d.basBufferDynamicOffset;

			currentAS->geometries.geometry.triangles.vertexData = vk_d.geometry.xyz_dynamic[vk.swapchain.currentImage].buffer;
			currentAS->geometries.geometry.triangles.indexData = vk_d.geometry.idx_dynamic[vk.swapchain.currentImage].buffer;
			currentAS->geometries.geometry.triangles.indexOffset = vk_d.geometry.idx_dynamic_offset * sizeof(uint32_t);
			currentAS->geometries.geometry.triangles.vertexOffset = vk_d.geometry.xyz_dynamic_offset * sizeof(float[12]);
			currentAS->data.offsetIDX = vk_d.geometry.idx_dynamic_offset;
			currentAS->data.offsetXYZ = vk_d.geometry.xyz_dynamic_offset;
		}
		else {
			currentAS = bAS;

		}
		currentAS->data.dynamic = qtrue;
	}


	if (deform) RB_DeformTessGeometry();
	if (cTex) {
		ComputeTexCoords(tess.shader->stages[0]);
		bAS->data.material |= MATERIAL_FLAG_NEEDSCOLOR;
	}
	ComputeColors(tess.shader->stages[0]);
	
	if (dynamic || frames || deform || cTex) {
		RB_WriteIDX(currentAS->geometries.geometry.triangles.indexOffset, dynamic);
		RB_WriteXYZ(currentAS->geometries.geometry.triangles.vertexOffset, dynamic);
	}
	if (!dynamic && (deform || frames)) {

		qboolean updateBottom = (currentAS->geometries.geometry.triangles.vertexCount >= tess.numVertexes &&
			currentAS->geometries.geometry.triangles.indexCount == tess.numIndexes);

		currentAS->geometries.geometry.triangles.vertexCount = tess.numVertexes;
		currentAS->geometries.geometry.triangles.indexCount = tess.numIndexes;

		if (updateBottom) VK_UpdateBottomAS(vk.swapchain.CurrentCommandBuffer(), currentAS, currentAS, &vk_d.basBufferStatic, VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV, NULL);
		else  VK_RecreateBottomAS(vk.swapchain.CurrentCommandBuffer(), currentAS, &vk_d.basBufferStatic, VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV);
	}
	if (dynamic) {
		if (currentAS != bAS) { // (deform || frames)
			qboolean updateBottom = (currentAS->geometries.geometry.triangles.vertexCount >= tess.numVertexes &&
				currentAS->geometries.geometry.triangles.indexCount == tess.numIndexes);

			currentAS->geometries.geometry.triangles.vertexCount = tess.numVertexes;
			currentAS->geometries.geometry.triangles.indexCount = tess.numIndexes;

			VK_UpdateBottomAS(vk.swapchain.CurrentCommandBuffer(), bAS, currentAS, &vk_d.basBufferDynamic[vk.swapchain.currentImage], VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV, &vk_d.basBufferDynamicOffset);
			vk_d.bottomASDynamicCount[vk.swapchain.currentImage]++;
			vk_d.geometry.idx_dynamic_offset += tess.numIndexes;
			vk_d.geometry.xyz_dynamic_offset += tess.numVertexes;
		}
	}
		
	RB_UpdateInstanceDataBuffer(currentAS);
	RB_UpdateInstanceBuffer(currentAS);
	// add bottom to trace as list
	memcpy(&vk_d.bottomASTraceList[vk_d.bottomASTraceListCount], currentAS, sizeof(vkbottomAS_t));
	vk_d.bottomASTraceListCount++;
}

static void RB_UpdateRayTraceAS(drawSurf_t* drawSurfs, int numDrawSurfs) {
	shader_t*		shader;
	int				fogNum;
	int				entityNum;
	int				dlighted;
	int				i;
	drawSurf_t*		drawSurf;
	float			originalTime;
	qboolean		dynamic, forceUpdate;

	// save original time for entity shader offsets
	originalTime = backEnd.refdef.floatTime;
	backEnd.currentEntity = &tr.worldEntity;
	
	for (i = 0, drawSurf = drawSurfs; i < numDrawSurfs; i++, drawSurf++) {
		R_DecomposeSort(drawSurf->sort, &entityNum, &shader, &fogNum, &dlighted);
		// skip stuff
		if (strstr(shader->name, "models/mapobjects/console/under") || strstr(shader->name, "textures/sfx/beam") || strstr(shader->name, "models/mapobjects/lamps/flare03")
			|| strstr(shader->name, "Shadow") || shader->isSky) {
			continue;
		}
		
		// SS_BLEND0 bullets, ball around energy, glow around armore shards, armor glow ,lights/fire
		// SS_DECAL bullet marks
		forceUpdate = qfalse;
		// just to clean backend state
		RB_BeginSurface(shader, fogNum);

		float tM[12];
		if (entityNum != ENTITYNUM_WORLD) {
			backEnd.currentEntity = &backEnd.refdef.entities[entityNum];
			backEnd.refdef.floatTime = originalTime - backEnd.currentEntity->e.shaderTime;
			tess.shaderTime = backEnd.refdef.floatTime - tess.shader->timeOffset;

			R_RotateForEntity(backEnd.currentEntity, &backEnd.viewParms, &backEnd. or );
			
			tM[0] = backEnd.currentEntity->e.axis[0][0]; tM[1] = backEnd.currentEntity->e.axis[1][0]; tM[2] = backEnd.currentEntity->e.axis[2][0]; tM[3] = backEnd.currentEntity->e.origin[0];
			tM[4] = backEnd.currentEntity->e.axis[0][1]; tM[5] = backEnd.currentEntity->e.axis[1][1]; tM[6] = backEnd.currentEntity->e.axis[2][1]; tM[7] = backEnd.currentEntity->e.origin[1];
			tM[8] = backEnd.currentEntity->e.axis[0][2]; tM[9] = backEnd.currentEntity->e.axis[1][2]; tM[10] = backEnd.currentEntity->e.axis[2][2]; tM[11] = backEnd.currentEntity->e.origin[2];
			dynamic = qtrue;

			if (backEnd.currentEntity->e.reType & (RT_SPRITE | RT_BEAM | RT_LIGHTNING | RT_RAIL_CORE | RT_RAIL_RINGS)) {
				tM[0] = 1; tM[1] = 0; tM[2] = 0; tM[3] = 0;
				tM[4] = 0; tM[5] = 1; tM[6] = 0; tM[7] = 0;
				tM[8] = 0; tM[9] = 0; tM[10] = 1; tM[11] = 0;
			}
		}
		else {
			backEnd.currentEntity = &tr.worldEntity;
			backEnd.refdef.floatTime = originalTime;
			backEnd. or = backEnd.viewParms.world;
			tess.shaderTime = backEnd.refdef.floatTime - tess.shader->timeOffset;

			tM[0] = 1; tM[1] = 0; tM[2] = 0; tM[3] = 0;
			tM[4] = 0; tM[5] = 1; tM[6] = 0; tM[7] = 0;
			tM[8] = 0; tM[9] = 0; tM[10] = 1; tM[11] = 0;

			dynamic = qfalse;
		}
		if (drawSurf->bAS == NULL && drawSurf->surface == SF_ENTITY) {
			int x = 2;
		}
		if (backEnd.refdef.num_dlights > 0) {
			int x = 2;
		}
		//"models/mapobjects/gratelamp/gratelamp_b.tga" ...}, ...} ...}, ...}	textureBundle_t[2]


		// add the triangles for this surface
		rb_surfaceTable[*drawSurf->surface](drawSurf->surface);
		
		if (drawSurf->bAS != NULL && drawSurf->bAS->dynamic) {
			//continue;
			//VK_DestroyBottomAccelerationStructure(drawSurf->bAS);
			//RB_CreateDynamicBottomAS(drawSurf->bAS);
			//continue;
		}
		if (drawSurf->bAS == NULL) {
			continue;
		}
		//if (shader->sort == SS_DECAL) {
		//	continue;
		//	vec3_t a;
		//	vec3_t b;
		//	VectorSubtract(tess.xyz[0], tess.xyz[1], a);
		//	VectorSubtract(tess.xyz[0], tess.xyz[2], b);

		//	vec3_t normal;
		//	CrossProduct(a, b, normal);
		//	VectorNormalizeFast(normal);
		//	for (int u = 0; u < tess.numVertexes; u++) {
		//		tess.xyz[u][0] += 0.5f * normal[0];
		//		tess.xyz[u][1] += 0.5f * normal[1];
		//		tess.xyz[u][2] -= 0.5f * normal[2];
		//	}
		//	if (drawSurf->bAS == NULL && tess.numIndexes == 6 && tess.numVertexes == 4) {//backEnd.currentEntity->e.reType == RT_SPRITE) {RT_BEAM
		//		drawSurf->bAS = &vk_d.bottomASList[0];
		//		dynamic = qtrue;
		//		forceUpdate = qtrue;
		//		if (shader->stages[0]->stateBits == 65/*strstr(shader->name, "bullet_mrk") || strstr(shader->name, "burn_med_mrk")*/) drawSurf->bAS->data.material = MATERIAL_FLAG_BULLET_MARK | MATERIAL_FLAG_NEEDSCOLOR;
		//		else drawSurf->bAS->data.material = MATERIAL_FLAG_NEEDSCOLOR | MATERIAL_FLAG_PARTICLE;//MATERIAL_FLAG_BULLET_MARK;
		//	}
		//}
		//if (backEnd.currentEntity->e.reType & (RT_SPRITE)) {
		//	int x;
		//}

		//// blood ray projectile etc
		//if (drawSurf->bAS == NULL && tess.numIndexes == 6 && tess.numVertexes == 4){//backEnd.currentEntity->e.reType == RT_SPRITE) {RT_BEAM
		//	drawSurf->bAS = &vk_d.bottomASList[0];
		//	dynamic = qtrue;
		//	forceUpdate = qtrue;
		//	drawSurf->bAS->data.material |= (MATERIAL_FLAG_NEEDSCOLOR | MATERIAL_FLAG_PARTICLE);
		//}
		//
		//if (i > 30) //continue;
		//if (strstr(shader->name, "wire")) {
		//	//continue;
		//}
		//// everything else
		///*if (drawSurf->bAS == NULL && tess.shader->stages[0] != NULL) {
		//	dynamic = qtrue;
		//	forceUpdate = qtrue;
		//	RB_CreateBottomAS(&drawSurf->bAS, dynamic);
		//}*/
		
		if (drawSurf->bAS == NULL) {
			continue;
		}
		
		
		if (drawSurf->bAS->dynamic) {
			Com_Memcpy(&drawSurf->bAS[vk.swapchain.currentImage].geometryInstance.transform, &tM, sizeof(float[12]));
			RB_AddBottomAS(&drawSurf->bAS[vk.swapchain.currentImage], dynamic, forceUpdate);
		}
		else {
			Com_Memcpy(&drawSurf->bAS->geometryInstance.transform, &tM, sizeof(float[12]));
			RB_AddBottomAS(drawSurf->bAS, dynamic, forceUpdate);
		}
	}
	backEnd.refdef.floatTime = originalTime;

	VK_DestroyTopAccelerationStructure(&vk_d.topAS[vk.swapchain.currentImage]);
	VK_MakeTopAS(vk.swapchain.CurrentCommandBuffer(), &vk_d.topAS[vk.swapchain.currentImage], &vk_d.topASBuffer[vk.swapchain.currentImage], vk_d.bottomASTraceList, vk_d.bottomASTraceListCount, vk_d.instanceBuffer[vk.swapchain.currentImage], VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_NV);
	
	tess.numIndexes = 0;
	tess.numVertexes = 0;
}

static void RB_BuildViewMatrix(float *viewMatrix, float *origin, vec3_t *axis) {
	viewMatrix[0] = axis[0][0];
	viewMatrix[4] = axis[0][1];
	viewMatrix[8] = axis[0][2];
	viewMatrix[12] = -origin[0] * viewMatrix[0] + -origin[1] * viewMatrix[4] + -origin[2] * viewMatrix[8];

	viewMatrix[1] = axis[1][0];
	viewMatrix[5] = axis[1][1];
	viewMatrix[9] = axis[1][2];
	viewMatrix[13] = -origin[0] * viewMatrix[1] + -origin[1] * viewMatrix[5] + -origin[2] * viewMatrix[9];

	viewMatrix[2] = axis[2][0];
	viewMatrix[6] = axis[2][1];
	viewMatrix[10] = axis[2][2];
	viewMatrix[14] = -origin[0] * viewMatrix[2] + -origin[1] * viewMatrix[6] + -origin[2] * viewMatrix[10];

	viewMatrix[3] = 0;
	viewMatrix[7] = 0;
	viewMatrix[11] = 0;
	viewMatrix[15] = 1;
}

static void RB_BuildProjMatrix(float* projMatrix, float* p, float zFar) {
	// update q3's proj matrix (opengl) to vulkan conventions: z - [0, 1] instead of [-1, 1] and invert y direction
	float zNear = r_znear->value;
	float P10 = -zFar / (zFar - zNear);
	float P14 = -zFar * zNear / (zFar - zNear);
	float P5 = -p[5];

	float result[16] = {
		p[0],  p[1],  p[2], p[3],
		p[4],  P5,    p[6], p[7],
		p[8],  p[9],  P10,  p[11],
		p[12], p[13], P14,  p[15]
	};
	memcpy(projMatrix, &result, sizeof(result));
}

static void RB_TraceRays() {
	static float	s_flipMatrix[16] = {
		// convert from our coordinate system (looking down X)
		// to OpenGL's coordinate system (looking down -Z)
		0, 0, -1, 0,
		-1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 0, 1
	};

	vec3_t	origin;	// player position
	VectorCopy(backEnd.viewParms. or .origin, origin);
	float	viewMatrix[16];
	float	viewMatrixFlipped[16];
	float	invViewMatrix[16];
	float	invProjMatrix[16];

	// projection matrix
	float projMatrix[16];
	RB_BuildProjMatrix(&projMatrix, backEnd.viewParms.projectionMatrix, backEnd.viewParms.zFar);
	// viewMatrix
	RB_BuildViewMatrix(&viewMatrix[0], &origin, &backEnd.viewParms. or .axis);
	// flip matrix for vulkan
	myGlMultMatrix(viewMatrix, s_flipMatrix, viewMatrixFlipped);
	// inverse view matrix
	myGLInvertMatrix(&viewMatrixFlipped, &invViewMatrix);
	// inverse proj matrix
	myGLInvertMatrix(&projMatrix, &invProjMatrix);
	
	// view portal
	vec3_t	originPortal;	// portal position
	//VectorCopy(vk_d.portalViewParms.pvsOrigin, originPortal);
	VectorCopy(vk_d.portalViewParms. or .origin, originPortal);
	if (vk_d.portalInView) {
		float	invViewMatrixPortal[16];
		float	viewMatrixPortal[16];
		float	viewMatrixFlippedPortal[16];
		float	projMatrixPortal[16];
		float	invProjMatrixPortal[16];

		RB_BuildProjMatrix(&projMatrixPortal, vk_d.portalViewParms.projectionMatrix, vk_d.portalViewParms.zFar);
		myGLInvertMatrix(&projMatrixPortal, &invProjMatrixPortal);

		RB_BuildViewMatrix(&viewMatrixPortal[0], &originPortal, &vk_d.portalViewParms. or .axis);
		myGlMultMatrix(viewMatrixPortal, s_flipMatrix, viewMatrixFlippedPortal);
		myGLInvertMatrix(&viewMatrixFlippedPortal, &invViewMatrixPortal);
		VK_UploadBufferDataOffset(&vk_d.uboBuffer[vk.swapchain.currentImage], 16 * sizeof(float), 16 * sizeof(float), (void*)&invViewMatrixPortal[0]);
		VK_UploadBufferDataOffset(&vk_d.uboBuffer[vk.swapchain.currentImage], 48 * sizeof(float), 16 * sizeof(float), (void*)&invProjMatrixPortal[0]);
	}

	// mvp
	myGlMultMatrix(&viewMatrixFlipped[0], projMatrix, vk_d.mvp);

	VK_UploadBufferDataOffset(&vk_d.uboBuffer[vk.swapchain.currentImage], 0, 16 * sizeof(float), (void*)&invViewMatrix[0]);
	VK_UploadBufferDataOffset(&vk_d.uboBuffer[vk.swapchain.currentImage], 32 * sizeof(float), 16 * sizeof(float), (void*)&invProjMatrix[0]);
	VK_UploadBufferDataOffset(&vk_d.uboBuffer[vk.swapchain.currentImage], 64 * sizeof(float), 16 * sizeof(float), (void*)&viewMatrixFlipped[0]);
	VK_UploadBufferDataOffset(&vk_d.uboBuffer[vk.swapchain.currentImage], 80 * sizeof(float), 16 * sizeof(float), (void*)&projMatrix[0]);
	VK_UploadBufferDataOffset(&vk_d.uboBuffer[vk.swapchain.currentImage], 96 * sizeof(float), 1 * sizeof(qboolean), (void*)&vk_d.portalInView);

	// bind rt pipeline
	VK_BindRayTracingPipeline(&vk_d.accelerationStructures.pipeline);
	// bind descriptor (rt data and texture array)
	VK_Bind2RayTracingDescriptorSets(&vk_d.accelerationStructures.pipeline, &vk_d.accelerationStructures.descriptor[vk.swapchain.currentImage], &vk_d.imageDescriptor);

	// push constants
	//VK_SetRayTracingPushConstant(&vk_d.accelerationStructures.pipeline, VK_SHADER_STAGE_RAYGEN_BIT_NV, 0 * sizeof(float), 16 * sizeof(float), &invViewMatrix[0]);
	//VK_SetRayTracingPushConstant(&vk_d.accelerationStructures.pipeline, VK_SHADER_STAGE_RAYGEN_BIT_NV, 16 * sizeof(float), 16 * sizeof(float), &invProjMatrix[0]);
	VK_SetRayTracingPushConstant(&vk_d.accelerationStructures.pipeline, VK_SHADER_STAGE_RAYGEN_BIT_NV, 0 * sizeof(float), sizeof(vec3_t), &origin);
	VK_SetRayTracingPushConstant(&vk_d.accelerationStructures.pipeline, VK_SHADER_STAGE_RAYGEN_BIT_NV, 4 * sizeof(float), sizeof(vec3_t), &originPortal);
	VK_SetRayTracingPushConstant(&vk_d.accelerationStructures.pipeline, VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV, 40 * sizeof(float), 16 * sizeof(float), &vk_d.mvp[0]);

	VK_TraceRays(&vk_d.accelerationStructures.pipeline.shaderBindingTableBuffer);
}

void RB_RayTraceScene(drawSurf_t* drawSurfs, int numDrawSurfs) {
	VkMemoryBarrier memoryBarrier = { 0 };
	vkCmdEndRenderPass(vk.swapchain.CurrentCommandBuffer());
	//VK_BeginFramebuffer(&vk_d.accelerationStructures.resultFramebuffer);
	//renderSky(drawSurfs, numDrawSurfs);
	//VK_EndFramebuffer(&vk_d.accelerationStructures.resultFramebuffer);
	//VK_CopySwapchainToImage(&vk_d.accelerationStructures.resultImage);

	for (int i = 0; i < vk_d.bottomASDynamicCount[vk.swapchain.currentImage]; i++) {
		VK_DestroyBottomAccelerationStructure(&vk_d.bottomASDynamicList[vk.swapchain.currentImage][i]);
	}
	vk_d.basBufferDynamicOffset = 0;
	vk_d.bottomASDynamicCount[vk.swapchain.currentImage] = 0;
	vk_d.geometry.xyz_dynamic_offset = 0;
	vk_d.geometry.idx_dynamic_offset = 0;

	vk_d.bottomASTraceListCount = 0;
	vk_d.scratchBufferOffset = 0;
	vk_d.lightCount = 0;

	vk_d.asUpdateTime = Sys_Milliseconds();
	RB_UpdateRayTraceAS(drawSurfs, numDrawSurfs);
	vk_d.asUpdateTime = Sys_Milliseconds() - vk_d.asUpdateTime;

	RB_TraceRays();

	memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV| VK_ACCESS_MEMORY_WRITE_BIT;
	memoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
	vkCmdPipelineBarrier(vk.swapchain.CurrentCommandBuffer(), VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, 0, 1, &memoryBarrier, 0, 0, 0, 0);

	/*memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	memoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
	memoryBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	vkCmdPipelineBarrier(vk.swapchain.CurrentCommandBuffer(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, 0, 1, &memoryBarrier, 0, 0, 0, 0);*/


	vk_d.portalInView = qfalse;
	vk_d.mirrorInView = qfalse;

	// draw pt results to swap chain
	VK_BeginRenderClear();
	VK_DrawFullscreenRect(&vk_d.accelerationStructures.resultImage[vk.swapchain.currentImage]);
	//VK_DrawFullscreenRect(&vk_d.accelerationStructures.resultFramebuffer.image);
}

