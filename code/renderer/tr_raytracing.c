#include "tr_local.h"

/*
glConfig.driverType == VULKAN && r_vertexLight->value == 2
*/


void RB_CreateNewBottomAS(surfaceType_t *surface, shader_t* shader, vkbottomAS_t** bAS) {
	if (shader->isSky) return;
	tess.numVertexes = 0;
	tess.numIndexes = 0;

	if (*surface == SF_SKIP) return;
	if (*surface == SF_BAD) return;
	if (*surface == SF_FACE) RB_SurfaceFace((srfSurfaceFace_t*)surface);
	else if (*surface == SF_GRID) RB_SurfaceGrid((srfGridMesh_t*)surface);
	else if (*surface == SF_TRIANGLES) RB_SurfaceTriangles((srfTriangles_t*)surface);
	else if (*surface == SF_POLY) RB_SurfacePolychain((srfPoly_t*)surface);
	else if (*surface == SF_MD3) RB_SurfaceMesh((md3Surface_t*)surface);
	/*else if (*surface == SF_MD4) {
		RB_SurfaceAnim((md4Surface_t*)surface);
	}*/
	//else if (type == SF_FLARE) RB_SurfaceFlare((srfFlare_t*)s_worldData.surfaces[i].data);
	else return;

	/*R_DecomposeSort(drawSurf->sort, &entityNum, &shader, &fogNum, &dlighted);

	backEnd.currentEntity = &backEnd.refdef.entities[entityNum];
	*/
	int				j;
	tess.shader = shader;

	for (int stage = 0; stage < MAX_SHADER_STAGES; stage++)
	{

		shaderStage_t* pStage = shader->stages[stage];
		if (!pStage || pStage->bundle[0].isLightmap || !pStage->active) {
			continue;
		}
		
		ComputeColors(pStage);
		ComputeTexCoords(pStage);
		vk_d.bottomASList[vk_d.bottomASCount].geometries.sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
		vk_d.bottomASList[vk_d.bottomASCount].geometries.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
		vk_d.bottomASList[vk_d.bottomASCount].geometries.geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
		vk_d.bottomASList[vk_d.bottomASCount].geometries.geometry.triangles.vertexData = vk_d.geometry.xyz.buffer;
		vk_d.bottomASList[vk_d.bottomASCount].geometries.geometry.triangles.vertexOffset = vk_d.geometry.offsetXYZ * sizeof(float[8]);
		vk_d.bottomASList[vk_d.bottomASCount].geometries.geometry.triangles.vertexCount = tess.numVertexes;
		vk_d.bottomASList[vk_d.bottomASCount].geometries.geometry.triangles.vertexStride = 8 * sizeof(float);
		vk_d.bottomASList[vk_d.bottomASCount].geometries.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
		vk_d.bottomASList[vk_d.bottomASCount].geometries.geometry.triangles.indexData = vk_d.geometry.idx.buffer;
		vk_d.bottomASList[vk_d.bottomASCount].geometries.geometry.triangles.indexOffset = vk_d.geometry.offsetIDX * sizeof(uint32_t);
		vk_d.bottomASList[vk_d.bottomASCount].geometries.geometry.triangles.indexCount = tess.numIndexes;
		vk_d.bottomASList[vk_d.bottomASCount].geometries.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
		vk_d.bottomASList[vk_d.bottomASCount].geometries.geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
		vk_d.bottomASList[vk_d.bottomASCount].geometries.flags = VK_GEOMETRY_OPAQUE_BIT_NV;
		if (pStage->stateBits == (GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE)) {
			vk_d.bottomASList[vk_d.bottomASCount].geometries.flags = 0;
		}


		for (j = 0; j < tess.numIndexes; j++) {
			//VK_UploadBufferDataOffset(&geometry.idx, vk_d.offsetIdx * sizeof(uint32_t), numIndexes * sizeof(uint32_t), (void*)& indexes[0]);
			uint32_t idx = (uint32_t)tess.indexes[j];
			//idx += offsetXYZ;
			VK_UploadBufferDataOffset(&vk_d.geometry.idx, vk_d.geometry.offsetIDX * sizeof(uint32_t) + (j * sizeof(uint32_t)), sizeof(uint32_t), (void*)&idx);
		}
		vk_d.bottomASList[vk_d.bottomASCount].data.offsetIdx = vk_d.geometry.offsetIDX;
		vk_d.bottomASList[vk_d.bottomASCount].data.offsetXYZ = vk_d.geometry.offsetXYZ;
		vk_d.bottomASList[vk_d.bottomASCount].data.texIdx = (float)shader->stages[stage]->bundle[0].image[0]->index;
		vk_d.bottomASList[vk_d.bottomASCount].data.texIdx2 = 0;
		vk_d.bottomASList[vk_d.bottomASCount].data.blendfunc = (uint32_t)(pStage->stateBits);
		vk_d.bottomASList[vk_d.bottomASCount].data.isMirror = shader->sort == SS_PORTAL && strstr(shader->name, "mirror") != NULL; //strcmp(shader->name, "textures/common/mirror2") == 0;
		/*if (!strcmp(shader->name, "textures/common/mirror2")) {
			int aaaaa = 2;
		}*/

		if ((pStage->stateBits == (GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE)) || (shader->contentFlags & CONTENTS_TRANSLUCENT)) {
			vk_d.bottomASList[vk_d.bottomASCount].data.texIdx2 = 1;
		}
		if ((shader->surfaceFlags == SURF_ALPHASHADOW)) {
			vk_d.bottomASList[vk_d.bottomASCount].data.texIdx2 = 2;
		}
		//VK_UploadBufferDataOffset(&vk_d.instanceDataBuffer, vk_d.geometry.numSurfaces * sizeof(ASInstanceData), sizeof(ASInstanceData), (void*)&vk_d.bottomASList[vk_d.bottomASCount].data);

		qboolean cTex = (shader->stages[0]->bundle[0].tcGen != TCGEN_BAD || shader->stages[0]->bundle[0].numTexMods > 0);
		for (j = 0; j < tess.numVertexes; j++) {
			float p[8] = {
				p[0] = tess.xyz[j][0],
				p[1] = tess.xyz[j][1],
				p[2] = tess.xyz[j][2],
				p[3] = 0,
				p[4] = cTex ? tess.svars.texcoords[0][j][0] : tess.texCoords[j][0][0],
				p[5] = cTex ? tess.svars.texcoords[0][j][1] : tess.texCoords[j][0][1],
				p[6] = tess.texCoords[j][1][0],
				p[7] = tess.texCoords[j][1][1],
			};
			//cv->points[i][3 + j] = LittleFloat(verts[i].st[j]);
			VK_UploadBufferDataOffset(&vk_d.geometry.xyz, vk_d.geometry.offsetXYZ * 8 * sizeof(float) + (j * 8 * sizeof(float)), 8 * sizeof(float), (void*)&p);
		}
		//ri.Printf(PRINT_ALL, "Brightest lightmap value: %d\n", (int)(s->stages[0]->bundle[0].image[0]->index));
		//if (geometry.numSurfaces == 4727) {
		//	int as = 123;
		//	//isTransparent(pStage->stateBits);
		//	break;
		//}
		if (pStage->stateBits != 34 && pStage->stateBits != 256 && pStage->stateBits != 101) {
			int test = 1;
		}
		qboolean deform = /*surface == SF_MD3 ||*/ shader->numDeforms > 0;
		if (deform) VK_MakeBottomSingle(&vk_d.bottomASList[vk_d.bottomASCount], VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV);
		else VK_MakeBottomSingle(&vk_d.bottomASList[vk_d.bottomASCount], VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV);

		if (bAS != NULL) (*bAS) = &vk_d.bottomASList[vk_d.bottomASCount];
		vk_d.bottomASCount++;
		vk_d.geometry.numSurfaces += 1;
		/*if (tess.shader->isSky) {
			vk_d.geometry.offsetIDX += 55 * tess.numIndexes;
			vk_d.geometry.offsetXYZ += 55 *tess.numVertexes;
		}
		else {*/
		/*	if ((*surface == SF_MD3)) {
				vk_d.geometry.offsetIDX += tess.numIndexes;
				vk_d.geometry.offsetXYZ += ((md3Surface_t*)surface)->numFrames * tess.numVertexes;

			}
			else {*/

		vk_d.geometry.offsetIDX += shader->numDeforms > 0 ? 5 * tess.numIndexes : 5 * tess.numIndexes;
		vk_d.geometry.offsetXYZ += shader->numDeforms > 0 ? 5 * tess.numVertexes : 5 * tess.numVertexes;
		//}
		//}
		//break;

	}
	//if (geometry.numSurfaces == 4726) break; // control
	//if (geometry.numSurfaces > 4879) break; light
	//if (geometry.numSurfaces > 4846) break; // quake sphere 0 0 console/sphere2
	//if (geometry.numSurfaces >= 3639) break; // roof SURF_SKY SURF_NOIMPACT SURF_NOLIGHTMAP isSky

	tess.numVertexes = 0;
	tess.numIndexes = 0;
}

void RB_UpdateInstanceBuffer(vkbottomAS_t* bAS) {
	bAS->geometryInstance.instanceCustomIndex = 0;
	// set visibility for first and third person (eye and mirror)
	if ((backEnd.currentEntity->e.renderfx & RF_THIRD_PERSON)) bAS->geometryInstance.mask = RTX_MIRROR_VISIBLE;
	else if ((backEnd.currentEntity->e.renderfx & RF_FIRST_PERSON)) bAS->geometryInstance.mask = RTX_FIRST_PERSON_VISIBLE;
	else bAS->geometryInstance.mask = RTX_FIRST_PERSON_MIRROR_VISIBLE;

	bAS->geometryInstance.instanceOffset = 0;

	switch (tess.shader->cullType) {
	case CT_FRONT_SIDED:
		bAS->geometryInstance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FRONT_COUNTERCLOCKWISE_BIT_NV; break;
	case CT_BACK_SIDED:
		bAS->geometryInstance.flags = 0; break;
	case CT_TWO_SIDED:
		bAS->geometryInstance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV; break;
	}
	bAS->geometryInstance.accelerationStructureHandle = bAS->handle;

	VK_UploadBufferDataOffset(&vk_d.instanceBuffer, vk_d.instanceBufferOffset + vk_d.bottomASDynamicCount * sizeof(VkGeometryInstanceNV), sizeof(VkGeometryInstanceNV), (void*)&bAS->geometryInstance);
}

void RB_UpdateInstanceDataBuffer(vkbottomAS_t* bAS) {
	if (tess.shader->stages[0]->bundle[0].numImageAnimations > 1) {
		int indexAnim = (int)(tess.shaderTime * tess.shader->stages[0]->bundle[0].imageAnimationSpeed * FUNCTABLE_SIZE);
		indexAnim >>= FUNCTABLE_SIZE2;

		if (indexAnim < 0) {
			indexAnim = 0;	// may happen with shader time offsets
		}
		indexAnim %= tess.shader->stages[0]->bundle[0].numImageAnimations;
		if (bAS->data.texIdx != (float)tess.shader->stages[0]->bundle[0].image[indexAnim]->index) {
			bAS->data.texIdx = (float)tess.shader->stages[0]->bundle[0].image[indexAnim]->index;
			tess.shader->stages[0]->bundle[0].image[indexAnim]->frameUsed = tr.frameCount;
		}
	}
	else {
		if (bAS->data.texIdx != (float)tess.shader->stages[0]->bundle[0].image[0]->index) {
			tess.shader->stages[0]->bundle[0].image[0]->frameUsed = tr.frameCount;
			bAS->data.texIdx = (float)tess.shader->stages[0]->bundle[0].image[0]->index;
		}
	}

	VK_UploadBufferDataOffset(&vk_d.instanceDataBuffer[vk.swapchain.currentImage], vk_d.bottomASDynamicCount * sizeof(ASInstanceData), sizeof(ASInstanceData), (void*)&bAS->data);
}

void RB_AddBottomAS(vkbottomAS_t* bAS) {
	shaderCommands_t* input  = &tess;

	if (input->numIndexes == 0) {
		return;
	}
	if (input->indexes[SHADER_MAX_INDEXES - 1] != 0) {
		ri.Error(ERR_DROP, "RB_EndSurface() - SHADER_MAX_INDEXES hit");
	}
	if (input->xyz[SHADER_MAX_VERTEXES - 1][0] != 0) {
		ri.Error(ERR_DROP, "RB_EndSurface() - SHADER_MAX_VERTEXES hit");
	}
	if (tess.shader == tr.shadowShader) {
		return;
	}
	// for debugging of sort order issues, stop rendering after a given sort value
	if (r_debugSort->integer && r_debugSort->integer < tess.shader->sort) {
		return;
	}

	if (bAS != NULL) {
		if (!strcmp(tess.shader->name, "textures/common/mirror2")) {
			int aaaaa = 2;
		}
		if (input->shader->sort == SS_PORTAL) {
			int x = 2l;
		}
		if (input->shader->sort == SS_OPAQUE) {
			int x = 2l;
		}
		qboolean anim = tess.shader->stages[0]->bundle[0].numImageAnimations > 0;
		qboolean cTex = tess.shader->stages[0] != NULL ? ((tess.shader->stages[0]->bundle[0].tcGen != TCGEN_BAD) && tess.shader->stages[0]->bundle[0].numTexMods > 0) : qfalse;
		qboolean deform = tess.shader->numDeforms > 0 /*|| (*drawSurf->surface == SF_MD3 && ((md3Surface_t*)drawSurf->surface)->numFrames > 1)*/;
		qboolean frames = backEnd.currentEntity->e.frame != backEnd.currentEntity->e.oldframe;//(*drawSurf->surface == SF_MD3 && ((md3Surface_t*)drawSurf->surface)->numFrames > 1);
		qboolean sky = qfalse;//tess.shader->isSky;
		if (frames | deform | cTex) {

			if (deform) RB_DeformTessGeometry();
			if (cTex) ComputeTexCoords(tess.shader->stages[0]);

			if (frames | deform | sky) {
				for (int j = 0; j < tess.numIndexes; j++) {
					//VK_UploadBufferDataOffset(&geometry.idx, vk_d.offsetIdx * sizeof(uint32_t), numIndexes * sizeof(uint32_t), (void*)& indexes[0]);
					uint32_t idx = (uint32_t)tess.indexes[j];
					//idx += offsetXYZ;
					VK_UploadBufferDataOffset(&vk_d.geometry.idx, bAS->geometries.geometry.triangles.indexOffset + (j * sizeof(uint32_t)), sizeof(uint32_t), (void*)&idx);
				}
			}
			for (int j = 0; j < tess.numVertexes; j++) {
				float p[12] = {
					p[0] = tess.xyz[j][0],
					p[1] = tess.xyz[j][1],
					p[2] = tess.xyz[j][2],
					p[3] = 0,
					p[4] = cTex ? tess.svars.texcoords[0][j][0] : tess.texCoords[j][0][0],
					p[5] = cTex ? tess.svars.texcoords[0][j][1] : tess.texCoords[j][0][1],
					p[6] = tess.texCoords[j][1][0],
					p[7] = tess.texCoords[j][1][1]
				};
				VK_UploadBufferDataOffset(&vk_d.geometry.xyz, bAS->geometries.geometry.triangles.vertexOffset + (j * sizeof(float[8])), 8 * sizeof(float), (void*)&p);
			}
			if (deform || frames) {

				qboolean updateBottom = (bAS->geometries.geometry.triangles.vertexCount >= tess.numVertexes &&
					bAS->geometries.geometry.triangles.indexCount == tess.numIndexes);

				bAS->geometries.geometry.triangles.vertexCount = tess.numVertexes;
				bAS->geometries.geometry.triangles.indexCount = tess.numIndexes;

				if (updateBottom) VK_UpdateBottomSingle(vk.swapchain.CurrentCommandBuffer(), bAS, VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV);
				else  VK_UpdateBottomSingleDelete(vk.swapchain.CurrentCommandBuffer(), bAS, VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV);
			}

		}
		/*VkGeometryInstanceNV geometryInstance = { 0 };
		geometryInstance.instanceCustomIndex = 0;
		geometryInstance.mask = 0xff;
		geometryInstance.instanceOffset = 0;
		geometryInstance.flags = drawSurf->bAS->flags;*/
		//drawSurf->bAS->geometryInstance.accelerationStructureHandle = drawSurf->bAS->handle;


		
		RB_UpdateInstanceBuffer(bAS);
		RB_UpdateInstanceDataBuffer(bAS);
		
		// add bottom to trace as list
		memcpy(&vk_d.bottomASListDynamic[vk_d.bottomASDynamicCount], bAS, sizeof(vkbottomAS_t));
		vk_d.bottomASDynamicCount++;
	}
}

static void RB_UpdateRayTraceAS(drawSurf_t* drawSurfs, int numDrawSurfs) {
	shader_t*		shader;
	int				fogNum;
	int				entityNum;
	int				dlighted;
	int				i;
	drawSurf_t*		drawSurf;
	float			originalTime;

	vk_d.bottomASDynamicCount = 0;
	vk_d.scratchBufferOffset = 0;
	vk_d.instanceBufferOffset = vk.swapchain.currentImage * (5000 * sizeof(VkGeometryInstanceNV));
	// save original time for entity shader offsets
	originalTime = backEnd.refdef.floatTime;
	backEnd.currentEntity = &tr.worldEntity;
	
	for (i = 0, drawSurf = drawSurfs; i < numDrawSurfs; i++, drawSurf++) {
		R_DecomposeSort(drawSurf->sort, &entityNum, &shader, &fogNum, &dlighted);
		// just to clean backend state
		RB_BeginSurface(shader, fogNum);

		if (entityNum != ENTITYNUM_WORLD) {
			backEnd.currentEntity = &backEnd.refdef.entities[entityNum];
			backEnd.refdef.floatTime = originalTime - backEnd.currentEntity->e.shaderTime;
			tess.shaderTime = backEnd.refdef.floatTime - tess.shader->timeOffset;

			R_RotateForEntity(backEnd.currentEntity, &backEnd.viewParms, &backEnd. or );
			float transform[12] = {
				backEnd.currentEntity->e.axis[0][0], backEnd.currentEntity->e.axis[1][0], backEnd.currentEntity->e.axis[2][0], backEnd.currentEntity->e.origin[0],
				backEnd.currentEntity->e.axis[0][1], backEnd.currentEntity->e.axis[1][1], backEnd.currentEntity->e.axis[2][1], backEnd.currentEntity->e.origin[1],
				backEnd.currentEntity->e.axis[0][2], backEnd.currentEntity->e.axis[1][2], backEnd.currentEntity->e.axis[2][2], backEnd.currentEntity->e.origin[2]
			};
			if (drawSurf->bAS != NULL) Com_Memcpy(&drawSurf->bAS->geometryInstance.transform, &transform, sizeof(float[12]));
		}
		else {
			backEnd.currentEntity = &tr.worldEntity;
			backEnd.refdef.floatTime = originalTime;
			backEnd. or = backEnd.viewParms.world;
			tess.shaderTime = backEnd.refdef.floatTime - tess.shader->timeOffset;

			float transform[12] = {
				1,0,0,0,
				0,1,0,0,
				0,0,1,0
			};
			if(drawSurf->bAS != NULL) Com_Memcpy(&drawSurf->bAS->geometryInstance.transform, &transform, sizeof(float[12]));
		}

		// add the triangles for this surface
		rb_surfaceTable[*drawSurf->surface](drawSurf->surface);
		RB_AddBottomAS(drawSurf->bAS);
	}
	backEnd.refdef.floatTime = originalTime;

	VK_DestroyTopAccelerationStructure(&vk_d.topAS[vk.swapchain.currentImage]);
	VK_MakeTop(vk.swapchain.CurrentCommandBuffer(), &vk_d.topAS[vk.swapchain.currentImage], vk_d.bottomASListDynamic, vk_d.bottomASDynamicCount, VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_NV);
	
	tess.numIndexes = 0;
	tess.numVertexes = 0;
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

	float	invViewMatrix[16];
	float	invProjMatrix[16];

	float	viewMatrix[16];
	float	viewMatrixFlipped[16];
	vec3_t	origin;	// player position
	VectorCopy(backEnd.viewParms. or .origin, origin);

	// projection matrix calculation
	const float* p = backEnd.viewParms.projectionMatrix;
	// update q3's proj matrix (opengl) to vulkan conventions: z - [0, 1] instead of [-1, 1] and invert y direction
	float zNear = r_znear->value;
	float zFar = backEnd.viewParms.zFar;
	float P10 = -zFar / (zFar - zNear);
	float P14 = -zFar * zNear / (zFar - zNear);
	float P5 = -p[5];

	float projMatrix[16] = {
		p[0],  p[1],  p[2], p[3],
		p[4],  P5,    p[6], p[7],
		p[8],  p[9],  P10,  p[11],
		p[12], p[13], P14,  p[15]
	};

	// view
	viewMatrix[0] = backEnd.viewParms. or .axis[0][0];
	viewMatrix[4] = backEnd.viewParms. or .axis[0][1];
	viewMatrix[8] = backEnd.viewParms. or .axis[0][2];
	viewMatrix[12] = -origin[0] * viewMatrix[0] + -origin[1] * viewMatrix[4] + -origin[2] * viewMatrix[8];

	viewMatrix[1] = backEnd.viewParms. or .axis[1][0];
	viewMatrix[5] = backEnd.viewParms. or .axis[1][1];
	viewMatrix[9] = backEnd.viewParms. or .axis[1][2];
	viewMatrix[13] = -origin[0] * viewMatrix[1] + -origin[1] * viewMatrix[5] + -origin[2] * viewMatrix[9];

	viewMatrix[2] = backEnd.viewParms. or .axis[2][0];
	viewMatrix[6] = backEnd.viewParms. or .axis[2][1];
	viewMatrix[10] = backEnd.viewParms. or .axis[2][2];
	viewMatrix[14] = -origin[0] * viewMatrix[2] + -origin[1] * viewMatrix[6] + -origin[2] * viewMatrix[10];

	viewMatrix[3] = 0;
	viewMatrix[7] = 0;
	viewMatrix[11] = 0;
	viewMatrix[15] = 1;

	// flip matrix for vulkan
	myGlMultMatrix(viewMatrix, s_flipMatrix, viewMatrixFlipped);

	// inverse view matrix
	myGLInvertMatrix(&viewMatrixFlipped, &invViewMatrix);
	// inverse proj matrix
	myGLInvertMatrix(&projMatrix, &invProjMatrix);
	// mvp
	myGlMultMatrix(&viewMatrixFlipped[0], projMatrix, vk_d.mvp);

	// bind rt pipeline
	VK_BindRayTracingPipeline(&vk_d.accelerationStructures.pipeline);
	// bind descriptor (rt data and texture array)
	VK_Bind2RayTracingDescriptorSets(&vk_d.accelerationStructures.pipeline, &vk_d.accelerationStructures.descriptor[vk.swapchain.currentImage], &vk_d.imageDescriptor);

	// push constants
	VK_SetRayTracingPushConstant(&vk_d.accelerationStructures.pipeline, VK_SHADER_STAGE_RAYGEN_BIT_NV, 0 * sizeof(float), 16 * sizeof(float), &invViewMatrix[0]);
	VK_SetRayTracingPushConstant(&vk_d.accelerationStructures.pipeline, VK_SHADER_STAGE_RAYGEN_BIT_NV, 16 * sizeof(float), 16 * sizeof(float), &invProjMatrix[0]);
	VK_SetRayTracingPushConstant(&vk_d.accelerationStructures.pipeline, VK_SHADER_STAGE_RAYGEN_BIT_NV, 32 * sizeof(float), sizeof(vec3_t), &origin);
	VK_SetRayTracingPushConstant(&vk_d.accelerationStructures.pipeline, VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV, 40 * sizeof(float), 16 * sizeof(float), &vk_d.mvp[0]);

	VK_TraceRays(&vk_d.accelerationStructures.pipeline.shaderBindingTableBuffer);
}

void RB_RayTraceScene(drawSurf_t* drawSurfs, int numDrawSurfs) {
	vkCmdEndRenderPass(vk.swapchain.CurrentCommandBuffer());

	RB_UpdateRayTraceAS(drawSurfs, numDrawSurfs);
	RB_TraceRays();

	VkMemoryBarrier memoryBarrier = { 0 };
	memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV| VK_ACCESS_MEMORY_WRITE_BIT;
	memoryBarrier.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
	vkCmdPipelineBarrier(vk.swapchain.CurrentCommandBuffer(), VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, 0, 1, &memoryBarrier, 0, 0, 0, 0);

	// draw rt results to swap chain
	VK_BeginRenderClear();
	VK_DrawFullscreenRect(&vk_d.accelerationStructures.resultImage);
}

