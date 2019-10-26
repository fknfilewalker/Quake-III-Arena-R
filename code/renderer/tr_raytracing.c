#include "tr_local.h"

/*
glConfig.driverType == VULKAN && r_vertexLight->value == 2
*/

static float	s_flipMatrix[16] = {
	// convert from our coordinate system (looking down X)
	// to OpenGL's coordinate system (looking down -Z)
	0, 0, -1, 0,
	-1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 0, 1
};

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
		vk_d.bottomASList[vk_d.bottomASCount].data.a = strcmp(shader->name, "textures/common/mirror2") == 0;
		if (!strcmp(shader->name, "textures/common/mirror2")) {
			int aaaaa = 2;
		}

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
				p[3] = (float)(pStage->stateBits),//->stages[0].bundle[0].image[0]->index, // texture id
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
		if(deform) VK_MakeBottomSingle(&vk_d.bottomASList[vk_d.bottomASCount], VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV);
		else VK_MakeBottomSingle(&vk_d.bottomASList[vk_d.bottomASCount], VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV);
		// setup topas instance data
		vk_d.bottomASList[vk_d.bottomASCount].geometryInstance.instanceCustomIndex = 0;
		vk_d.bottomASList[vk_d.bottomASCount].geometryInstance.mask = RTX_FIRST_PERSON_MIRROR_VISIBLE;
		vk_d.bottomASList[vk_d.bottomASCount].geometryInstance.instanceOffset = 0;
		vk_d.bottomASList[vk_d.bottomASCount].geometryInstance.accelerationStructureHandle = vk_d.bottomASList[vk_d.bottomASCount].handle;
		switch (shader->cullType) {
		case CT_FRONT_SIDED:
			vk_d.bottomASList[vk_d.bottomASCount].geometryInstance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FRONT_COUNTERCLOCKWISE_BIT_NV; break;
		case CT_BACK_SIDED:
			vk_d.bottomASList[vk_d.bottomASCount].geometryInstance.flags = 0; break;
		case CT_TWO_SIDED:
			vk_d.bottomASList[vk_d.bottomASCount].geometryInstance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV; break;
		}


		if(bAS != NULL) (*bAS) = &vk_d.bottomASList[vk_d.bottomASCount];
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

static void RB_UpdateRayTraceAS(drawSurf_t* drawSurfs, int numDrawSurfs) {
	shader_t* shader;
	int				fogNum;
	int				entityNum;
	int				dlighted;
	qboolean		depthRange;
	int				i,j;
	drawSurf_t* drawSurf;
	float			originalTime;

	vk_d.bottomASDynamicCount = 0;

	// save original time for entity shader offsets
	originalTime = backEnd.refdef.floatTime;

	backEnd.currentEntity = &tr.worldEntity;
	//backEnd.pc.c_surfaces += numDrawSurfs;

	// RTX
	vk_d.scratchBufferOffset = 0;
	vk_d.instanceBufferOffset = vk.swapchain.currentImage * (5000 * sizeof(VkGeometryInstanceNV));

	
	for (i = 0, drawSurf = drawSurfs; i < numDrawSurfs; i++, drawSurf++) {
		tess.numIndexes = 0;
		tess.numVertexes = 0;

		R_DecomposeSort(drawSurf->sort, &entityNum, &shader, &fogNum, &dlighted);
		tess.shader = shader;
		tess.shaderTime = backEnd.refdef.floatTime - tess.shader->timeOffset;
		if (tess.shader->clampTime && tess.shaderTime >= tess.shader->clampTime) {
			tess.shaderTime = tess.shader->clampTime;
		}

		if (shader->isSky) continue;

		if (!strcmp(tess.shader->name, "textures/gothic_block/gkcspinemove")) {
			int a = 2;
		}

		rb_surfaceTable[*drawSurf->surface](drawSurf->surface);

		if(drawSurf->bAS != NULL){
			//if (*drawSurf->surface != SF_GRID) continue;
			vkbottomAS_t* bAS;
			/*if (tess.shader->numDeforms > 0) {
				bAS = &drawSurf->bAS[vk.swapchain.currentImage];
			}
			else {
			}*/
				bAS = &drawSurf->bAS[0];

			if (bAS->offset != 66322432) {
				//continue;
			}
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
				Com_Memcpy(&bAS->geometryInstance.transform, &transform, sizeof(float[12]));
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
				Com_Memcpy(&bAS->geometryInstance.transform, &transform, sizeof(float[12]));
			}
			
			/*if (drawSurf->bAS->offset == 66322432) {
				bAS = drawSurf->bAS;
			}
			if (drawSurf->bAS->offset != 66322432 && bAS == drawSurf->bAS) {
				int x = 2;
			}*/

		
			
			qboolean anim = tess.shader->stages[0]->bundle[0].numImageAnimations > 0;
			qboolean cTex = tess.shader->stages[0] != NULL ? ((tess.shader->stages[0]->bundle[0].tcGen != TCGEN_BAD)&& tess.shader->stages[0]->bundle[0].numTexMods > 0) : qfalse;
			qboolean deform = tess.shader->numDeforms > 0;
			qboolean frames = qfalse; //(*drawSurf->surface == SF_MD3 && ((md3Surface_t*)drawSurf->surface)->numFrames > 1);
			qboolean sky = qfalse;//tess.shader->isSky;
			if (frames | deform | cTex) {
				if (!strcmp(tess.shader->name, "textures/gothic_block/gkcspinemove")) {
					int a = 2;
				}
				
				/*if (sky) {
					RB_ClipSkyPolygons(&tess);
					if (tess.shader->sky.outerbox[0] && tess.shader->sky.outerbox[0] != tr.defaultImage) {
						int x = 1;
					}
					R_BuildCloudData(&tess);

					RB_DeformTessGeometry();
					ComputeTexCoords(tess.shader->stages[0]);

					backEnd.skyRenderedThisView = qtrue;
				}
				else {*/
				if (bAS->offset == 66322432) {
					int x = 2l;
				}
					if (deform) RB_DeformTessGeometry();
					if (cTex) ComputeTexCoords(tess.shader->stages[0]);
				//}

				if (deform | sky) {
					for (j = 0; j < tess.numIndexes; j++) {
						//VK_UploadBufferDataOffset(&geometry.idx, vk_d.offsetIdx * sizeof(uint32_t), numIndexes * sizeof(uint32_t), (void*)& indexes[0]);
						uint32_t idx = (uint32_t)tess.indexes[j];
						//idx += offsetXYZ;
						VK_UploadBufferDataOffset(&vk_d.geometry.idx,bAS->geometries.geometry.triangles.indexOffset + (j * sizeof(uint32_t)), sizeof(uint32_t), (void*)&idx);
					}
				}
				if (frames) {

				}
				for (j = 0; j < tess.numVertexes; j++) {
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
				if (deform) {
					if (tess.numIndexes > 200) {
						int hh = 1;
					}

					qboolean newBottom = bAS->geometries.geometry.triangles.vertexCount < tess.numVertexes ||
						bAS->geometries.geometry.triangles.indexCount < tess.numIndexes;

					if (bAS->offset == 66322432) {
						int x = 2l;
						
					}
					if (bAS->offset == 66584576) {
						int x = 2l;
						//continue;
					}

					bAS->geometries.geometry.triangles.vertexCount = tess.numVertexes;
					bAS->geometries.geometry.triangles.indexCount = tess.numIndexes;
					//VK_UpdateBottomSingleDelete(vk.swapchain.CurrentCommandBuffer(), bAS, VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV);
					if (newBottom) VK_UpdateBottomSingleDelete(vk.swapchain.CurrentCommandBuffer(), bAS, VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV);
					else {
						VK_UpdateBottomSingle(vk.swapchain.CurrentCommandBuffer(), bAS, VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV);
					}
					//VK_UpdateBottomSingleDelete(&vk_d.bottomASList[idxBottomAS]);
				}
				
			}
			if (tess.shader->numDeforms) {
				
				//RB_DeformTessGeometry();

				/*vk_d.bottomASList[idxBottomAS].geometries.geometry.triangles.vertexCount = tess.numVertexes;
				vk_d.bottomASList[idxBottomAS].geometries.geometry.triangles.indexCount = tess.numIndexes;
				VK_MakeBottomSingleDelete(&vk_d.bottomASList[idxBottomAS]);*/
			}
		
			
			/*VkGeometryInstanceNV geometryInstance = { 0 };
			geometryInstance.instanceCustomIndex = 0;
			geometryInstance.mask = 0xff;
			geometryInstance.instanceOffset = 0;
			geometryInstance.flags = drawSurf->bAS->flags;*/
			//drawSurf->bAS->geometryInstance.accelerationStructureHandle = drawSurf->bAS->handle;

			
			if (anim) {
				int indexAnim = (int)(tess.shaderTime * tess.shader->stages[0]->bundle[0].imageAnimationSpeed * FUNCTABLE_SIZE);
				indexAnim >>= FUNCTABLE_SIZE2;

				if (indexAnim < 0) {
					indexAnim = 0;	// may happen with shader time offsets
				}
				indexAnim %= tess.shader->stages[0]->bundle[0].numImageAnimations;
				bAS->data.texIdx = (float)tess.shader->stages[0]->bundle[0].image[indexAnim]->index;
				tess.shader->stages[0]->bundle[0].image[indexAnim]->frameUsed = tr.frameCount;

			}
			else {
				tess.shader->stages[0]->bundle[0].image[0]->frameUsed = tr.frameCount;
			}
			// set visibility for first and third
			if ((backEnd.currentEntity->e.renderfx & RF_THIRD_PERSON)) {
				bAS->geometryInstance.mask = RTX_MIRROR_VISIBLE;
			}
			else if ((backEnd.currentEntity->e.renderfx & RF_FIRST_PERSON)) {
				bAS->geometryInstance.mask = RTX_FIRST_PERSON_VISIBLE;
			}
			else {
				bAS->geometryInstance.mask = RTX_FIRST_PERSON_MIRROR_VISIBLE;
			}
			VK_UploadBufferDataOffset(&vk_d.instanceBuffer, vk_d.instanceBufferOffset + vk_d.bottomASDynamicCount * sizeof(VkGeometryInstanceNV), sizeof(VkGeometryInstanceNV), (void*)&bAS->geometryInstance);
			
			bAS->data.texIdx = (float)shader->stages[0]->bundle[0].image[0]->index;
			
			VK_UploadBufferDataOffset(&vk_d.instanceDataBuffer[vk.swapchain.currentImage], vk_d.bottomASDynamicCount * sizeof(ASInstanceData), sizeof(ASInstanceData), (void*)&bAS->data);
			
			memcpy(&vk_d.bottomASListDynamic[vk_d.bottomASDynamicCount], bAS, sizeof(vkbottomAS_t));
			vk_d.bottomASDynamicCount++;
		}
		else {
		int x = 10;
		}
		/*else {
		int idx;
			RB_CreateNewBottomAS(drawSurf->surface, shader, &idx);
		}*/
	}

	// update as
	
	//vk_d.tasBufferOffset = vk_d.topAS[1].offset;
	//vk_d.tasBufferOffset = vk.swapchain.currentImage * (65536 * 5);
	//vk_d.tasBufferOffset = 0;//0 * (65536);
	VK_DestroyTopAccelerationStructure(&vk_d.topAS[vk.swapchain.currentImage]);

	VK_MakeTop(vk.swapchain.CurrentCommandBuffer(), &vk_d.topAS[vk.swapchain.currentImage], vk_d.bottomASListDynamic, vk_d.bottomASDynamicCount, VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_NV);
	//VK_UpdateTop(vk.swapchain.CurrentCommandBuffer(), &vk_d.topAS[vk.swapchain.currentImage], &vk_d.topAS, vk_d.bottomASListDynamic, vk_d.bottomASDynamicCount);
	
	
	backEnd.refdef.floatTime = originalTime;

	// clean render list
	tess.numIndexes = 0;
	tess.numVertexes = 0;
	tess.shader = NULL;
	tess.fogNum = 0;
	tess.dlightBits = 0;		// will be OR'd in by surface functions
	tess.xstages = NULL;
	tess.numPasses = 0;
	tess.currentStageIteratorFunc = NULL;

}

static void RB_TraceRays() {
	//VK_UpdateTop(&vk_d.staticBottomAS);
	//if (vk_d.accelerationStructures.init/* && vk_d.drawMirror*/) {
	float invView[16];
	float invProj[16];

	float	viewerMatrix[16];
	float	viewerMatrix2[16];
	vec3_t	origin;

	// proj
	const float* p = backEnd.viewParms.projectionMatrix;
	// update q3's proj matrix (opengl) to vulkan conventions: z - [0, 1] instead of [-1, 1] and invert y direction
	float zNear = r_znear->value;
	float zFar = backEnd.viewParms.zFar;
	float P10 = -zFar / (zFar - zNear);
	float P14 = -zFar * zNear / (zFar - zNear);
	float P5 = -p[5];

	float proj[16] = {
		p[0],  p[1],  p[2], p[3],
		p[4],  P5,    p[6], p[7],
		p[8],  p[9],  P10,  p[11],
		p[12], p[13], P14,  p[15]
	};
	Com_Memcpy(vk_d.projectionMatrix, proj, 64);

	// view
	VectorCopy(backEnd.viewParms. or .origin, origin);
	viewerMatrix[0] = backEnd.viewParms. or .axis[0][0];
	viewerMatrix[4] = backEnd.viewParms. or .axis[0][1];
	viewerMatrix[8] = backEnd.viewParms. or .axis[0][2];
	viewerMatrix[12] = -origin[0] * viewerMatrix[0] + -origin[1] * viewerMatrix[4] + -origin[2] * viewerMatrix[8];

	viewerMatrix[1] = backEnd.viewParms. or .axis[1][0];
	viewerMatrix[5] = backEnd.viewParms. or .axis[1][1];
	viewerMatrix[9] = backEnd.viewParms. or .axis[1][2];
	viewerMatrix[13] = -origin[0] * viewerMatrix[1] + -origin[1] * viewerMatrix[5] + -origin[2] * viewerMatrix[9];

	viewerMatrix[2] = backEnd.viewParms. or .axis[2][0];
	viewerMatrix[6] = backEnd.viewParms. or .axis[2][1];
	viewerMatrix[10] = backEnd.viewParms. or .axis[2][2];
	viewerMatrix[14] = -origin[0] * viewerMatrix[2] + -origin[1] * viewerMatrix[6] + -origin[2] * viewerMatrix[10];

	viewerMatrix[3] = 0;
	viewerMatrix[7] = 0;
	viewerMatrix[11] = 0;
	viewerMatrix[15] = 1;

	myGlMultMatrix(viewerMatrix, s_flipMatrix, viewerMatrix2);

	// inverse
	myGLInvertMatrix(&viewerMatrix2, &invView);
	myGLInvertMatrix(&vk_d.projectionMatrix, &invProj);

	/*VK_SetAccelerationStructure(&vk_d.accelerationStructures.descriptor[vk.swapchain.currentImage], 0, VK_SHADER_STAGE_RAYGEN_BIT_NV | VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV, &vk_d.topAS[vk.swapchain.currentImage].accelerationStructure);
	VK_SetStorageImage(&vk_d.accelerationStructures.descriptor[vk.swapchain.currentImage], 1, VK_SHADER_STAGE_RAYGEN_BIT_NV, vk_d.accelerationStructures.resultImage.view);
	VK_SetStorageBuffer(&vk_d.accelerationStructures.descriptor[vk.swapchain.currentImage], 2, VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV | VK_SHADER_STAGE_ANY_HIT_BIT_NV, vk_d.geometry.xyz.buffer);
	VK_SetStorageBuffer(&vk_d.accelerationStructures.descriptor[vk.swapchain.currentImage], 3, VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV | VK_SHADER_STAGE_ANY_HIT_BIT_NV, vk_d.geometry.idx.buffer);
	VK_SetStorageBuffer(&vk_d.accelerationStructures.descriptor[vk.swapchain.currentImage], 4, VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV | VK_SHADER_STAGE_ANY_HIT_BIT_NV, vk_d.instanceDataBuffer[vk.swapchain.currentImage].buffer);
	VK_UpdateDescriptorSet(&vk_d.accelerationStructures.descriptor[vk.swapchain.currentImage]);*/

	VK_BindRayTracingPipeline(&vk_d.accelerationStructures.pipeline);
	VK_Bind2RayTracingDescriptorSets(&vk_d.accelerationStructures.pipeline, &vk_d.accelerationStructures.descriptor[vk.swapchain.currentImage], &vk_d.imageDescriptor);

	float pos[3];
	pos[0] = backEnd.viewParms. or .origin[0];// -650;
	pos[1] = backEnd.viewParms. or .origin[1];//630;
	pos[2] = backEnd.viewParms. or .origin[2];//140;
	////tr.viewParms.or.origin;
	float direction[3];
	direction[0] = 1;// -650;
	direction[1] = 0;//630;
	direction[2] = 0;//140;
	////tr. or .modelMatrix
	//	//ri.Printf(PRINT_ALL, "%f %f %f\n", tr. or .modelMatrix[3],
	//		//tr. or .modelMatrix[7],
	//		//tr. or .modelMatrix[11]);
	VK_SetRayTracingPushConstant(&vk_d.accelerationStructures.pipeline, VK_SHADER_STAGE_RAYGEN_BIT_NV, 0 * sizeof(float), 16 * sizeof(float), &invView[0]);
	VK_SetRayTracingPushConstant(&vk_d.accelerationStructures.pipeline, VK_SHADER_STAGE_RAYGEN_BIT_NV, 16 * sizeof(float), 16 * sizeof(float), &invProj[0]);
	VK_SetRayTracingPushConstant(&vk_d.accelerationStructures.pipeline, VK_SHADER_STAGE_RAYGEN_BIT_NV, 32 * sizeof(float), sizeof(vec3_t), &origin);
	VK_SetRayTracingPushConstant(&vk_d.accelerationStructures.pipeline, VK_SHADER_STAGE_RAYGEN_BIT_NV, 36 * sizeof(float), sizeof(vec3_t), &backEnd.viewParms.portalPlane.normal);

	//VK_SetRayTracingPushConstant(&vk_d.accelerationStructures.pipeline, VK_SHADER_STAGE_RAYGEN_BIT_NV, 4 * sizeof(float), sizeof(vec3_t), &direction);
	VK_TraceRays(&vk_d.accelerationStructures.pipeline.shaderBindingTableBuffer);
	//VK_CopyImageToSwapchain(&vk_d.accelerationStructures.resultImage);
	//VK_TransitionImage(&vk_d.accelerationStructures.resultImage, )
	
	//vk_d.drawMirror = qfalse;
//}
//return;
}

void RB_RayTraceScene(drawSurf_t* drawSurfs, int numDrawSurfs) {
	vkCmdEndRenderPass(vk.swapchain.CurrentCommandBuffer());

	RB_UpdateRayTraceAS(drawSurfs, numDrawSurfs);
	RB_TraceRays();

	/*VkMemoryBarrier memoryBarrier = { 0 };
	memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV| VK_ACCESS_MEMORY_WRITE_BIT;
	memoryBarrier.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
	vkCmdPipelineBarrier(vk.swapchain.CurrentCommandBuffer(), VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, 0, 1, &memoryBarrier, 0, 0, 0, 0);*/

	VK_BeginRenderClear();
	VK_DrawFullscreenRect(&vk_d.accelerationStructures.resultImage);
}

