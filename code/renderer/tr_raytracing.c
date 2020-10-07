#include "tr_local.h"
#include "../../shader/glsl/constants.h"

/*
glConfig.driverType == VULKAN && r_vertexLight->value == 2
*/

#define RTX_BOTTOM_AS_FLAG (VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV | VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV)
#define RTX_TOP_AS_FLAG (VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV)


void RB_UploadCluster(vkbuffer_t* buffer, uint32_t offsetIDX, int defaultC) {
	uint32_t* clusterData = calloc(tess.numIndexes/3, sizeof(uint32_t));
	for (int i = 0; i < (tess.numIndexes / 3); i++) {
		vec4_t pos = { 0,0,0,0 };
		for (int j = 0; j < 3; j++) {
			VectorAdd(pos, tess.xyz[tess.indexes[(i * 3) + j]], pos);
			VectorAdd(pos, tess.normal[tess.indexes[(i * 3) + j]], pos);
		}
		VectorScale(pos, 1.0f / 3.0f, pos);
		int c = -1;

		// the cluster calculation in Quake III is unreliable, therefore we need multiple fallbacks
		int cluster[3];
		cluster[0] = R_FindClusterForPos3(tess.xyz[tess.indexes[(i * 3) + 0]]);
		cluster[1] = R_FindClusterForPos3(tess.xyz[tess.indexes[(i * 3) + 1]]);
		cluster[2] = R_FindClusterForPos3(tess.xyz[tess.indexes[(i * 3) + 2]]);

		if(cluster[0] == -1) cluster[0] = R_FindClusterForPos(tess.xyz[tess.indexes[(i * 3) + 0]]);
		if (cluster[1] == -1) cluster[1] = R_FindClusterForPos(tess.xyz[tess.indexes[(i * 3) + 1]]);
		if (cluster[2] == -1) cluster[2] = R_FindClusterForPos(tess.xyz[tess.indexes[(i * 3) + 2]]);

		if (cluster[0] == -1) cluster[0] = R_FindClusterForPos2(tess.xyz[tess.indexes[(i * 3) + 0]]);
		if (cluster[1] == -1) cluster[1] = R_FindClusterForPos2(tess.xyz[tess.indexes[(i * 3) + 1]]);
		if (cluster[2] == -1) cluster[2] = R_FindClusterForPos2(tess.xyz[tess.indexes[(i * 3) + 2]]);

		// if each vertex is in a different cluster merge them to one where the whole triangle is inside
		c = RB_TryMergeCluster(cluster, defaultC);
		
		// if we still got no cluster try the center
		if (c == -1) c = R_FindClusterForPos2(pos);
		if (c == -1) c = R_FindClusterForPos(pos);
		if (c == -1) c = R_FindClusterForPos3(pos);
		if (c == -1) {
			// use default cluster as last resort
			c = defaultC;
		}
		//c = defaultC;
		clusterData[i] = c;
	}
	VK_UploadBufferDataOffset(buffer, offsetIDX * sizeof(uint32_t), (tess.numIndexes/3) * sizeof(uint32_t), (void*)clusterData);
	free(clusterData);
}

void RB_UploadIDX(vkbuffer_t* buffer, uint32_t offsetIDX, uint32_t offsetXYZ) {
	uint32_t* idxData = calloc(tess.numIndexes, sizeof(uint32_t));
	for (int j = 0; j < tess.numIndexes; j++) {
		idxData[j] = (uint32_t)(tess.indexes[j] + offsetXYZ);
	}
	VK_UploadBufferDataOffset(buffer, offsetIDX * sizeof(uint32_t), tess.numIndexes * sizeof(uint32_t), (void*)idxData);
	free(idxData);
}

void RB_UploadXYZ(vkbuffer_t* buffer, uint32_t offsetXYZ, int cluster) {
	uint32_t material = RB_GetMaterial();
	// encode two tex idx and its needColor and blend flags into one 4 byte uint
	uint32_t tex0 = (RB_GetNextTexEncoded(0)) | (RB_GetNextTexEncoded(1) << TEX_SHIFT_BITS);
	uint32_t tex1 = (RB_GetNextTexEncoded(2)) | (RB_GetNextTexEncoded(3) << TEX_SHIFT_BITS);
	VertexBuffer* vData = calloc(tess.numVertexes, sizeof(VertexBuffer));


	for (int j = 0; j < tess.numVertexes; j++) {
		vData[j].pos[0] = tess.xyz[j][0];
		vData[j].pos[1] = tess.xyz[j][1];
		vData[j].pos[2] = tess.xyz[j][2];
		vData[j].normal[0] = tess.normal[j][0];
		vData[j].normal[1] = tess.normal[j][1];
		vData[j].normal[2] = tess.normal[j][2];
		vData[j].normal[3] = 0;
		vData[j].material = material;
		vData[j].texIdx0 = tex0;
		vData[j].texIdx1 = tex1;
		//int c = R_FindClusterForPos(tess.xyz[j]);
		vData[j].cluster = cluster;// c != -1 ? c : cluster;
	}
	// if there are multiple stages we need to upload them all
	if (tess.shader->rtstages[0] != NULL && tess.shader->rtstages[0]->active) {
		ComputeTexCoords(tess.shader->rtstages[0]);
		ComputeColors(tess.shader->rtstages[0]);
		for (int j = 0; j < tess.numVertexes; j++) {
			vData[j].color0 = tess.svars.colors[j][0] | tess.svars.colors[j][1] << 8 | tess.svars.colors[j][2] << 16 | tess.svars.colors[j][3] << 24;
			vData[j].uv0[0] = tess.svars.texcoords[0][j][0];
			vData[j].uv0[1] = tess.svars.texcoords[0][j][1];
		}
	}
	if (tess.shader->rtstages[1] != NULL && tess.shader->rtstages[1]->active) {
		ComputeTexCoords(tess.shader->rtstages[1]);
		ComputeColors(tess.shader->rtstages[1]);
		for (int j = 0; j < tess.numVertexes; j++) {
			vData[j].color1 = tess.svars.colors[j][0] | tess.svars.colors[j][1] << 8 | tess.svars.colors[j][2] << 16 | tess.svars.colors[j][3] << 24;
			vData[j].uv1[0] = tess.svars.texcoords[0][j][0];
			vData[j].uv1[1] = tess.svars.texcoords[0][j][1];
		}
	}
	if (tess.shader->rtstages[2] != NULL && tess.shader->rtstages[2]->active) {
		ComputeTexCoords(tess.shader->rtstages[2]);
		ComputeColors(tess.shader->rtstages[2]);
		for (int j = 0; j < tess.numVertexes; j++) {
			vData[j].color2 = tess.svars.colors[j][0] | tess.svars.colors[j][1] << 8 | tess.svars.colors[j][2] << 16 | tess.svars.colors[j][3] << 24;
			vData[j].uv2[0] = tess.svars.texcoords[0][j][0];
			vData[j].uv2[1] = tess.svars.texcoords[0][j][1];
		}
	}
	if (tess.shader->rtstages[3] != NULL && tess.shader->rtstages[3]->active) {
		ComputeTexCoords(tess.shader->rtstages[3]);
		ComputeColors(tess.shader->rtstages[3]);
		for (int j = 0; j < tess.numVertexes; j++) {
			vData[j].color3 = tess.svars.colors[j][0] | tess.svars.colors[j][1] << 8 | tess.svars.colors[j][2] << 16 | tess.svars.colors[j][3] << 24;
			vData[j].uv3[0] = tess.svars.texcoords[0][j][0];
			vData[j].uv3[1] = tess.svars.texcoords[0][j][1];
		}
	}
	VK_UploadBufferDataOffset(buffer,
		offsetXYZ * sizeof(VertexBuffer),
		tess.numVertexes * sizeof(VertexBuffer), (void*)vData);
	free(vData);
}

void RB_CreateEntityBottomAS(vkbottomAS_t** bAS, qboolean dynamic) {
	// set offsets and 
	uint32_t* idxOffset;
	uint32_t* xyzOffset;
	if (!dynamic) {
		idxOffset = &vk_d.geometry.idx_entity_static_offset;
		xyzOffset = &vk_d.geometry.xyz_entity_static_offset;
	}
	else {
		idxOffset = &vk_d.geometry.idx_entity_dynamic_offset;
		xyzOffset = &vk_d.geometry.xyz_entity_dynamic_offset;
	}
	// save bas in static or dynamic list
	vkbottomAS_t* bASList;
	if (!dynamic) bASList = &vk_d.bottomASList[vk_d.bottomASCount];
	else bASList = &vk_d.bottomASDynamicList[vk.swapchain.currentImage][vk_d.bottomASDynamicCount[vk.swapchain.currentImage]];
	//define AS geometry
	Com_Memset(&bASList->geometries, 0, sizeof(VkGeometryNV));
	bASList->geometries.sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
	bASList->geometries.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
	bASList->geometries.geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
	bASList->geometries.geometry.triangles.vertexOffset = (*xyzOffset) * sizeof(VertexBuffer);
	bASList->geometries.geometry.triangles.vertexCount = tess.numVertexes;
	bASList->geometries.geometry.triangles.vertexStride = sizeof(VertexBuffer);
	if (!dynamic) {
		bASList->geometries.geometry.triangles.vertexData = vk_d.geometry.xyz_entity_static.buffer;
		bASList->geometries.geometry.triangles.indexData = vk_d.geometry.idx_entity_static.buffer;
	}
	else {
		bASList->geometries.geometry.triangles.vertexData = vk_d.geometry.xyz_entity_dynamic[vk.swapchain.currentImage].buffer;
		bASList->geometries.geometry.triangles.indexData = vk_d.geometry.idx_entity_dynamic[vk.swapchain.currentImage].buffer;
	}
	bASList->geometries.geometry.triangles.indexOffset = (*idxOffset) * sizeof(uint32_t);
	bASList->geometries.geometry.triangles.indexCount = tess.numIndexes;
	bASList->geometries.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
	bASList->geometries.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
	bASList->geometries.geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
	bASList->geometries.flags = 0;

	if (strstr(tess.shader->name, "models/powerups/health/red")) {
		int x = 2;
		tess.shader->rtstages[0]->constantColor[0] = 140;
		tess.shader->rtstages[0]->constantColor[1] = 0;
		tess.shader->rtstages[0]->constantColor[2] = 0;
		tess.shader->rtstages[0]->constantColor[3] = 0;
		tess.shader->rtstages[0]->rgbGen = CGEN_CONST;
		tess.shader->rtstages[0]->bundle->image[0] = tr.whiteImage;
	}
	if (strstr(tess.shader->name, "models/powerups/health/yellow")) {
		int x = 2;
		tess.shader->rtstages[0]->constantColor[0] = 140;
		tess.shader->rtstages[0]->constantColor[1] = 140;
		tess.shader->rtstages[0]->constantColor[2] = 0;
		tess.shader->rtstages[0]->constantColor[3] = 0;
		tess.shader->rtstages[0]->rgbGen = CGEN_CONST;
		tess.shader->rtstages[0]->bundle->image[0] = tr.whiteImage;
	}

	bASList->data.offsetIDX = (*idxOffset);
	bASList->data.offsetXYZ = (*xyzOffset);
	if (!dynamic) {
		RB_UploadIDX(&vk_d.geometry.idx_entity_static, bASList->data.offsetIDX, 0);
		RB_UploadXYZ(&vk_d.geometry.xyz_entity_static, bASList->data.offsetXYZ, -1);
	}
	else {
		RB_UploadIDX(&vk_d.geometry.idx_entity_dynamic, bASList->data.offsetIDX, 0);
		RB_UploadXYZ(&vk_d.geometry.xyz_entity_dynamic, bASList->data.offsetXYZ, -1);
	}

	if (!dynamic) {
		VkCommandBuffer commandBuffer = { 0 };
		VK_BeginSingleTimeCommands(&commandBuffer);
		VK_CreateBottomAS(commandBuffer, bASList, &vk_d.basBufferEntityStatic, &vk_d.basBufferEntityStaticOffset, RTX_BOTTOM_AS_FLAG);
		VK_EndSingleTimeCommands(&commandBuffer);
	}
	else VK_CreateBottomAS(vk.swapchain.CurrentCommandBuffer(), bASList, &vk_d.basBufferEntityDynamic[vk.swapchain.currentImage], &vk_d.basBufferEntityDynamicOffset, RTX_BOTTOM_AS_FLAG);

	if (!dynamic) vk_d.bottomASCount++;
	else vk_d.bottomASDynamicCount[vk.swapchain.currentImage]++;
	(*idxOffset) += tess.numIndexes;
	(*xyzOffset) += tess.numVertexes;
	if (bAS != NULL) (*bAS) = bASList;
}

void RB_UpdateInstanceDataBuffer(vkbottomAS_t* bAS) {
	// set texture id and calc texture animation
	//int indexAnim = RB_GetNextTex(0);

	uint32_t tex0 = (RB_GetNextTexEncoded(0)) | (RB_GetNextTexEncoded(1) << TEX_SHIFT_BITS);
	uint32_t tex1 = (RB_GetNextTexEncoded(2)) | (RB_GetNextTexEncoded(3) << TEX_SHIFT_BITS);
	bAS->data.texIdx0 = tex0;
	bAS->data.texIdx1 = tex1;

	if (strstr(tess.shader->name, "orbb")) {
		int x = 2;
	}
	VK_UploadBufferDataOffset(&vk_d.instanceDataBuffer[vk.swapchain.currentImage], vk_d.bottomASTraceListCount * sizeof(ASInstanceData), sizeof(ASInstanceData), (void*)&bAS->data);
}

void RB_UpdateInstanceBuffer(vkbottomAS_t* bAS) {
	bAS->geometryInstance.instanceCustomIndex = 0;
	// set visibility for first and third person (eye and mirror)
	if ((backEnd.currentEntity->e.renderfx & RF_THIRD_PERSON)) bAS->geometryInstance.mask = RAY_MIRROR_OPAQUE_VISIBLE;
	else if ((backEnd.currentEntity->e.renderfx & RF_FIRST_PERSON)) bAS->geometryInstance.mask = RAY_FIRST_PERSON_OPAQUE_VISIBLE;
	else bAS->geometryInstance.mask = RAY_FIRST_PERSON_MIRROR_OPAQUE_VISIBLE;

	/*if (strstr(tess.shader->name, "glass")) {
		bAS->geometryInstance.mask = RAY_GLASS_VISIBLE;
	}*/

	if (tess.shader->sort <= SS_OPAQUE) {
		bAS->geometryInstance.flags = VK_GEOMETRY_INSTANCE_FORCE_OPAQUE_BIT_NV;
	}
	else {
		bAS->geometryInstance.flags = VK_GEOMETRY_INSTANCE_FORCE_NO_OPAQUE_BIT_NV;
	}
	if (strstr(tess.shader->name, "skel")) {
		int x = 2;
		bAS->geometryInstance.flags = VK_GEOMETRY_INSTANCE_FORCE_OPAQUE_BIT_NV;
	}
	if (strstr(tess.shader->name, "energy_grn1") || strstr(tess.shader->name, "teleportEffect") || strstr(tess.shader->name, "shotgun_laser") || strstr(tess.shader->name, "models/players/hunter/hunter_f")) {
		bAS->geometryInstance.flags = VK_GEOMETRY_INSTANCE_FORCE_NO_OPAQUE_BIT_NV;
		bAS->geometryInstance.instanceOffset = 1;
	}


	switch (tess.shader->cullType) {
		case CT_FRONT_SIDED:
			bAS->geometryInstance.flags |= VK_GEOMETRY_INSTANCE_TRIANGLE_FRONT_COUNTERCLOCKWISE_BIT_NV; break;
		case CT_BACK_SIDED:
			bAS->geometryInstance.flags |= 0; break;
		case CT_TWO_SIDED:
			bAS->geometryInstance.flags |= VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV; break;
	}
	//bAS->geometryInstance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV;

	bAS->geometryInstance.flags |= VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV;
	bAS->geometryInstance.accelerationStructureHandle = bAS->handle;

	VK_UploadBufferDataOffset(&vk_d.instanceBuffer[vk.swapchain.currentImage], vk_d.bottomASTraceListCount * sizeof(VkGeometryInstanceNV), sizeof(VkGeometryInstanceNV), (void*)&bAS->geometryInstance);
}

uint32_t preInstanceIDs[41500];
static void RB_UpdateRayTraceAS(drawSurf_t* drawSurfs, int numDrawSurfs) {
	shader_t*		shader;
	int				fogNum;
	int				entityNum;
	int				dlighted;
	int				i;
	drawSurf_t*		drawSurf;
	float			originalTime;
	qboolean		dynamic, forceUpdate;

	
	Com_Memset(vk_d.prevToCurrInstance, -1, 300 * sizeof(vk_d.prevToCurrInstance[0]));

	// save original time for entity shader offsets
	originalTime = backEnd.refdef.floatTime;
	backEnd.currentEntity = &tr.worldEntity;
	
	// add static world
	vk_d.bottomASWorldStatic.data.currInstanceID = vk_d.bottomASTraceListCount;
	vk_d.bottomASWorldStatic.data.prevInstanceID = vk_d.bottomASTraceListCount;
	vk_d.prevToCurrInstance[vk_d.bottomASTraceListCount] = vk_d.bottomASTraceListCount;
	VK_UploadBufferDataOffset(&vk_d.instanceDataBuffer[vk.swapchain.currentImage], vk_d.bottomASTraceListCount * sizeof(ASInstanceData), sizeof(ASInstanceData), (void*)&vk_d.bottomASWorldStatic.data);
	VK_UploadBufferDataOffset(&vk_d.instanceBuffer[vk.swapchain.currentImage], vk_d.bottomASTraceListCount * sizeof(VkGeometryInstanceNV), sizeof(VkGeometryInstanceNV), (void*)&vk_d.bottomASWorldStatic.geometryInstance);
	Com_Memcpy(&vk_d.bottomASTraceList[vk_d.bottomASTraceListCount], &vk_d.bottomASWorldStatic, sizeof(vkbottomAS_t));
	vk_d.bottomASTraceListCount++;
	// add static world trans
	/*vk_d.bottomASWorldStaticTrans.data.currInstanceID = vk_d.bottomASTraceListCount;
	vk_d.bottomASWorldStaticTrans.data.prevInstanceID = vk_d.bottomASTraceListCount;
	vk_d.prevToCurrInstance[vk_d.bottomASTraceListCount] = vk_d.bottomASTraceListCount;

	VK_UploadBufferDataOffset(&vk_d.instanceDataBuffer[vk.swapchain.currentImage], vk_d.bottomASTraceListCount * sizeof(ASInstanceData), sizeof(ASInstanceData), (void*)&vk_d.bottomASWorldStaticTrans.data);
	VK_UploadBufferDataOffset(&vk_d.instanceBuffer[vk.swapchain.currentImage], vk_d.bottomASTraceListCount * sizeof(VkGeometryInstanceNV), sizeof(VkGeometryInstanceNV), (void*)&vk_d.bottomASWorldStaticTrans.geometryInstance);
	memcpy(&vk_d.bottomASTraceList[vk_d.bottomASTraceListCount], &vk_d.bottomASWorldStaticTrans, sizeof(vkbottomAS_t));
	vk_d.bottomASTraceListCount++;*/
	// add world with dynamic data
	vk_d.bottomASWorldDynamicData.data.currInstanceID = vk_d.bottomASTraceListCount;
	vk_d.bottomASWorldDynamicData.data.prevInstanceID = vk_d.bottomASTraceListCount;
	vk_d.prevToCurrInstance[vk_d.bottomASTraceListCount] = vk_d.bottomASTraceListCount;
	VK_UploadBufferDataOffset(&vk_d.instanceDataBuffer[vk.swapchain.currentImage], vk_d.bottomASTraceListCount * sizeof(ASInstanceData), sizeof(ASInstanceData), (void*)&vk_d.bottomASWorldDynamicData.data);
	VK_UploadBufferDataOffset(&vk_d.instanceBuffer[vk.swapchain.currentImage], vk_d.bottomASTraceListCount * sizeof(VkGeometryInstanceNV), sizeof(VkGeometryInstanceNV), (void*)&vk_d.bottomASWorldDynamicData.geometryInstance);
	Com_Memcpy(&vk_d.bottomASTraceList[vk_d.bottomASTraceListCount], &vk_d.bottomASWorldDynamicData, sizeof(vkbottomAS_t));
	vk_d.bottomASTraceListCount++;
	// add world with dynamic data trans
	vk_d.bottomASWorldDynamicDataTrans.data.currInstanceID = vk_d.bottomASTraceListCount;
	vk_d.bottomASWorldDynamicDataTrans.data.prevInstanceID = vk_d.bottomASTraceListCount;
	vk_d.prevToCurrInstance[vk_d.bottomASTraceListCount] = vk_d.bottomASTraceListCount;
	VK_UploadBufferDataOffset(&vk_d.instanceDataBuffer[vk.swapchain.currentImage], vk_d.bottomASTraceListCount * sizeof(ASInstanceData), sizeof(ASInstanceData), (void*)&vk_d.bottomASWorldDynamicDataTrans.data);
	VK_UploadBufferDataOffset(&vk_d.instanceBuffer[vk.swapchain.currentImage], vk_d.bottomASTraceListCount * sizeof(VkGeometryInstanceNV), sizeof(VkGeometryInstanceNV), (void*)&vk_d.bottomASWorldDynamicDataTrans.geometryInstance);
	Com_Memcpy(&vk_d.bottomASTraceList[vk_d.bottomASTraceListCount], &vk_d.bottomASWorldDynamicDataTrans, sizeof(vkbottomAS_t));
	vk_d.bottomASTraceListCount++;
	// update world with dynamic data
	for (int i = 0; i < vk_d.updateDataOffsetXYZCount; i++) {
		shader_t* shader = vk_d.updateDataOffsetXYZ[i].shader;
		msurface_t* surf = vk_d.updateDataOffsetXYZ[i].surf;
		uint32_t offset = vk_d.updateDataOffsetXYZ[i].offsetXYZ;


		tess.numVertexes = 0;
		tess.numIndexes = 0;
		tess.shader = shader;
		backEnd.refdef.floatTime = originalTime;
		tess.shaderTime = backEnd.refdef.floatTime - tess.shader->timeOffset;
		rb_surfaceTable[*surf->data](surf->data);

		if (vk_d.updateDataOffsetXYZ[i].numXYZ != tess.numVertexes)
		{
			int x = 2;
		}

		RB_UploadXYZ(&vk_d.geometry.xyz_world_dynamic_data[vk.swapchain.currentImage], offset, vk_d.updateDataOffsetXYZ[i].cluster);
		tess.numVertexes = 0;
		tess.numIndexes = 0;
			
	}
	backEnd.refdef.floatTime = originalTime;

	// update world with dynamic as
	for (int i = 0; i < vk_d.updateASOffsetXYZCount; i++) {
		shader_t* shader = vk_d.updateASOffsetXYZ[i].shader;
		msurface_t* surf = vk_d.updateASOffsetXYZ[i].surf;
		uint32_t offsetIDX = vk_d.updateASOffsetXYZ[i].offsetIDX;
		uint32_t offsetXYZ = vk_d.updateASOffsetXYZ[i].offsetXYZ;
		uint32_t countXYZ = vk_d.updateASOffsetXYZ[i].countXYZ;


		tess.numVertexes = 0;
		tess.numIndexes = 0;
		tess.shader = shader;
		backEnd.refdef.floatTime = originalTime;
		tess.shaderTime = backEnd.refdef.floatTime - tess.shader->timeOffset;
		rb_surfaceTable[*surf->data](surf->data);
		RB_DeformTessGeometry();

		if (vk_d.updateASOffsetXYZ[i].numXYZ != tess.numVertexes)
		{
			int x = 2;
		}

		RB_UploadIDX(&vk_d.geometry.idx_world_dynamic_as[vk.swapchain.currentImage], offsetIDX, countXYZ);
		RB_UploadXYZ(&vk_d.geometry.xyz_world_dynamic_as[vk.swapchain.currentImage], offsetXYZ, vk_d.updateASOffsetXYZ[i].cluster);
		tess.numVertexes = 0;
		tess.numIndexes = 0;

	}
	vk_d.bottomASWorldDynamicAS[vk.swapchain.currentImage].data.currInstanceID = vk_d.bottomASTraceListCount;
	vk_d.bottomASWorldDynamicAS[vk.swapchain.currentImage].data.prevInstanceID = vk_d.bottomASTraceListCount;
	vk_d.prevToCurrInstance[vk_d.bottomASTraceListCount] = vk_d.bottomASTraceListCount;
	VK_UploadBufferDataOffset(&vk_d.instanceDataBuffer[vk.swapchain.currentImage], vk_d.bottomASTraceListCount * sizeof(ASInstanceData), sizeof(ASInstanceData), (void*)&vk_d.bottomASWorldDynamicAS[vk.swapchain.currentImage].data);
	VK_UploadBufferDataOffset(&vk_d.instanceBuffer[vk.swapchain.currentImage], vk_d.bottomASTraceListCount * sizeof(VkGeometryInstanceNV), sizeof(VkGeometryInstanceNV), (void*)&vk_d.bottomASWorldDynamicAS[vk.swapchain.currentImage].geometryInstance);
	VK_UpdateBottomAS(vk.swapchain.CurrentCommandBuffer(), &vk_d.bottomASWorldDynamicAS[vk.swapchain.currentImage],
		&vk_d.bottomASWorldDynamicAS[vk.swapchain.currentImage],
		&vk_d.basBufferWorldDynamicAS, NULL, VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV);
	backEnd.refdef.floatTime = originalTime;
	Com_Memcpy(&vk_d.bottomASTraceList[vk_d.bottomASTraceListCount], &vk_d.bottomASWorldDynamicAS[vk.swapchain.currentImage], sizeof(vkbottomAS_t));
	vk_d.bottomASTraceListCount++;
	// trans
	vk_d.bottomASWorldDynamicASTrans[vk.swapchain.currentImage].data.currInstanceID = vk_d.bottomASTraceListCount;
	vk_d.bottomASWorldDynamicASTrans[vk.swapchain.currentImage].data.prevInstanceID = vk_d.bottomASTraceListCount;
	vk_d.prevToCurrInstance[vk_d.bottomASTraceListCount] = vk_d.bottomASTraceListCount;
	VK_UploadBufferDataOffset(&vk_d.instanceDataBuffer[vk.swapchain.currentImage], vk_d.bottomASTraceListCount * sizeof(ASInstanceData), sizeof(ASInstanceData), (void*)&vk_d.bottomASWorldDynamicASTrans[vk.swapchain.currentImage].data);
	VK_UploadBufferDataOffset(&vk_d.instanceBuffer[vk.swapchain.currentImage], vk_d.bottomASTraceListCount * sizeof(VkGeometryInstanceNV), sizeof(VkGeometryInstanceNV), (void*)&vk_d.bottomASWorldDynamicASTrans[vk.swapchain.currentImage].geometryInstance);
	VK_UpdateBottomAS(vk.swapchain.CurrentCommandBuffer(), &vk_d.bottomASWorldDynamicASTrans[vk.swapchain.currentImage],
		&vk_d.bottomASWorldDynamicASTrans[vk.swapchain.currentImage],
		&vk_d.basBufferWorldDynamicAS, NULL, VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV);
	backEnd.refdef.floatTime = originalTime;
	Com_Memcpy(&vk_d.bottomASTraceList[vk_d.bottomASTraceListCount], &vk_d.bottomASWorldDynamicASTrans[vk.swapchain.currentImage], sizeof(vkbottomAS_t));
	vk_d.bottomASTraceListCount++;

	for (i = 0, drawSurf = drawSurfs; i < numDrawSurfs; i++, drawSurf++) {

		R_DecomposeSort(drawSurf->sort, &entityNum, &shader, &fogNum, &dlighted);
		// skip stuff
		if (strstr(shader->name, "skel")) {
			int x = 2;
		}
		if (strstr(shader->name, "models/mapobjects/console/under") || strstr(shader->name, "textures/sfx/beam") || strstr(shader->name, "models/mapobjects/lamps/flare03")
			|| strstr(shader->name, "Shadow") || shader->isSky ||
			(shader->contentFlags & CONTENTS_TRANSLUCENT) == CONTENTS_TRANSLUCENT || shader->sort > SS_OPAQUE) {
			if(!strstr(shader->name, "green_sphere") && !strstr(shader->name, "yellow_sphere") && !strstr(shader->name, "red_sphere") && !strstr(shader->name, "energy_grn1")
				&& !strstr(shader->name, "teleportEffect") && !strstr(shader->name, "bloodExplosion") && !strstr(shader->name, "skel")) continue;
		}

		if (strstr(shader->name, "green_sphere") || strstr(shader->name, "yellow_sphere") || strstr(shader->name, "red_sphere") || strstr(shader->name, "plasma_glass")) {
			shader->rtstages[0]->active = qfalse;
		}
	
		if (strstr(shader->name, "bloodExplosion")) {// bloodExplotion
			int i = 2;
			//rb_surfaceTable[*drawSurf->surface](drawSurf->surface);
			//continue;
		}
		if (shader->rtstages[0] == NULL || drawSurf->bAS == NULL) if (!strstr(shader->name, "bloodExplosion"))  continue;
		if (drawSurf->bAS == NULL) {

		}

		// SS_BLEND0 bullets, ball around energy, glow around armore shards, armor glow ,lights/fire
		// SS_DECAL bullet marks
		forceUpdate = qfalse;
		// just to clean backend state
		RB_BeginSurface(shader, fogNum);

		float tM[12];
		if (entityNum != ENTITYNUM_WORLD) {
			if (tess.shader->numDeforms > 0) {
				int x = 2;
			}

			backEnd.currentEntity = &backEnd.refdef.entities[entityNum];
			backEnd.refdef.floatTime = originalTime - backEnd.currentEntity->e.shaderTime;
			tess.shaderTime = backEnd.refdef.floatTime - tess.shader->timeOffset;

			R_RotateForEntity(backEnd.currentEntity, &backEnd.viewParms, &backEnd. or );
			
			tM[0] = backEnd.currentEntity->e.axis[0][0]; tM[1] = backEnd.currentEntity->e.axis[1][0]; tM[2] = backEnd.currentEntity->e.axis[2][0]; tM[3] = backEnd.currentEntity->e.origin[0];
			tM[4] = backEnd.currentEntity->e.axis[0][1]; tM[5] = backEnd.currentEntity->e.axis[1][1]; tM[6] = backEnd.currentEntity->e.axis[2][1]; tM[7] = backEnd.currentEntity->e.origin[1];
			tM[8] = backEnd.currentEntity->e.axis[0][2]; tM[9] = backEnd.currentEntity->e.axis[1][2]; tM[10] = backEnd.currentEntity->e.axis[2][2]; tM[11] = backEnd.currentEntity->e.origin[2];
			dynamic = qtrue;

			int cluster = -1; 
			if (drawSurf->bAS == NULL || !drawSurf->bAS->isWorldSurface) {
				//if(cluster == -1)cluster = R_FindClusterForPos((vec3_t) { backEnd.currentEntity->e.origin[0], backEnd.currentEntity->e.origin[1], backEnd.currentEntity->e.origin[2] });
				//if (cluster == -1)cluster = R_FindClusterForPos2((vec3_t) { backEnd.currentEntity->e.origin[0], backEnd.currentEntity->e.origin[1], backEnd.currentEntity->e.origin[2] });
				if (cluster == -1)cluster = R_FindClusterForPos3((vec3_t) { backEnd.currentEntity->e.origin[0], backEnd.currentEntity->e.origin[1], backEnd.currentEntity->e.origin[2] });
				if (cluster == -1) {
					int x = 2;
				}
			}
			else {
				cluster = drawSurf->bAS->c;//R_GetClusterFromSurface(drawSurf->surface);
			}
			if (backEnd.currentEntity->e.origin[0] == 0 && backEnd.currentEntity->e.origin[1] == 0 && backEnd.currentEntity->e.origin[2] == 0) {
				int x=2;
			}

			if (backEnd.currentEntity->e.reType & (RT_SPRITE | RT_BEAM | RT_LIGHTNING | RT_RAIL_CORE | RT_RAIL_RINGS)) {
				tM[0] = 1; tM[1] = 0; tM[2] = 0; tM[3] = 0;
				tM[4] = 0; tM[5] = 1; tM[6] = 0; tM[7] = 0;
				tM[8] = 0; tM[9] = 0; tM[10] = 1; tM[11] = 0;
			}
			if (strstr(shader->name, "bloodExplosion")) {// bloodExplotion
				int i = 2;
			}
			
			//if (strstr(tess.shader->name, "portal"))
			//if (strstr(shader->name, "powerups/armor/energy_grn1")) continue; //0x00000298be8ba1a8 {name=0x00000298be8ba1a8 "models/powerups/armor/energy_grn1" lightmapIndex=-1 index=...}
			if (drawSurf->bAS == NULL) {
				rb_surfaceTable[*drawSurf->surface](drawSurf->surface);
				RB_DeformTessGeometry();
				RB_CreateEntityBottomAS(&drawSurf->bAS, qtrue);
				drawSurf->bAS->data.type = BAS_ENTITY_DYNAMIC;
				drawSurf->bAS->data.cluster = cluster;
				drawSurf->bAS->geometryInstance.instanceOffset = 1;

				Com_Memcpy(&drawSurf->bAS->geometryInstance.transform, &tM, sizeof(float[12]));
				Com_Memcpy(&drawSurf->bAS->data.modelmat, &tM, sizeof(float[12]));

				RB_UpdateInstanceDataBuffer(drawSurf->bAS);
				RB_UpdateInstanceBuffer(drawSurf->bAS);

				Com_Memcpy(&vk_d.bottomASTraceList[vk_d.bottomASTraceListCount], drawSurf->bAS, sizeof(vkbottomAS_t));
				vk_d.bottomASTraceListCount++;
			}else if (RB_ASDataDynamic(tess.shader) && !RB_ASDynamic(tess.shader)) {
				//continue;
				drawSurf->bAS->data.type = BAS_ENTITY_DYNAMIC;
				drawSurf->bAS->data.cluster = cluster;

				rb_surfaceTable[*drawSurf->surface](drawSurf->surface);

				drawSurf->bAS->data.offsetIDX = vk_d.geometry.idx_entity_dynamic_offset;
				drawSurf->bAS->data.offsetXYZ = vk_d.geometry.xyz_entity_dynamic_offset;
				RB_UploadIDX(&vk_d.geometry.idx_entity_dynamic[vk.swapchain.currentImage], drawSurf->bAS->data.offsetIDX, 0);
				RB_UploadXYZ(&vk_d.geometry.xyz_entity_dynamic[vk.swapchain.currentImage], drawSurf->bAS->data.offsetXYZ, cluster);

				vk_d.geometry.idx_entity_dynamic_offset += tess.numIndexes;
				vk_d.geometry.xyz_entity_dynamic_offset += tess.numVertexes;

				Com_Memcpy(&drawSurf->bAS->geometryInstance.transform, &tM, sizeof(float[12]));
				Com_Memcpy(&drawSurf->bAS->data.modelmat, &tM, sizeof(float[12]));
				
				drawSurf->bAS->data.currInstanceID = vk_d.bottomASTraceListCount;
				drawSurf->bAS->data.prevInstanceID = preInstanceIDs[backEnd.currentEntity->e.id + drawSurf->bAS->surfcount];
				vk_d.prevToCurrInstance[drawSurf->bAS->data.prevInstanceID] = vk_d.bottomASTraceListCount;

				preInstanceIDs[backEnd.currentEntity->e.id + drawSurf->bAS->surfcount] = vk_d.bottomASTraceListCount;
				RB_UpdateInstanceDataBuffer(drawSurf->bAS);
				RB_UpdateInstanceBuffer(drawSurf->bAS);
				// add bottom to trace as list
				Com_Memcpy(&vk_d.bottomASTraceList[vk_d.bottomASTraceListCount], drawSurf->bAS, sizeof(vkbottomAS_t));
				vk_d.bottomASTraceListCount++;
			}
			else if (RB_ASDynamic(tess.shader)) {
				//continue;
				// ?leak only in debug build?
				//if(backEnd.currentEntity->e.id == 10) continue;
				vkbottomAS_t *newAS = &vk_d.bottomASDynamicList[vk.swapchain.currentImage][vk_d.bottomASDynamicCount[vk.swapchain.currentImage]];
				vk_d.bottomASDynamicCount[vk.swapchain.currentImage]++;
				Com_Memcpy(newAS, drawSurf->bAS, sizeof(vkbottomAS_t));
				newAS->data.type = BAS_ENTITY_DYNAMIC;
				newAS->data.cluster = cluster;

				rb_surfaceTable[*drawSurf->surface](drawSurf->surface);
				RB_DeformTessGeometry();

				newAS->data.offsetIDX = vk_d.geometry.idx_entity_dynamic_offset;
				newAS->data.offsetXYZ = vk_d.geometry.xyz_entity_dynamic_offset;
				RB_UploadIDX(&vk_d.geometry.idx_entity_dynamic[vk.swapchain.currentImage], newAS->data.offsetIDX, 0);
				RB_UploadXYZ(&vk_d.geometry.xyz_entity_dynamic[vk.swapchain.currentImage], newAS->data.offsetXYZ, cluster);

				newAS->geometries.geometry.triangles.indexOffset = newAS->data.offsetIDX * sizeof(uint32_t);
				newAS->geometries.geometry.triangles.vertexOffset = newAS->data.offsetXYZ * sizeof(VertexBuffer);
				newAS->geometries.geometry.triangles.indexData = vk_d.geometry.idx_entity_dynamic[vk.swapchain.currentImage].buffer;
				newAS->geometries.geometry.triangles.vertexData = vk_d.geometry.xyz_entity_dynamic[vk.swapchain.currentImage].buffer;

				vk_d.geometry.idx_entity_dynamic_offset += tess.numIndexes;
				vk_d.geometry.xyz_entity_dynamic_offset += tess.numVertexes;

				Com_Memcpy(&newAS->geometryInstance.transform, &tM, sizeof(float[12]));
				Com_Memcpy(&newAS->data.modelmat, &tM, sizeof(float[12]));

				VK_UpdateBottomAS(vk.swapchain.CurrentCommandBuffer(), drawSurf->bAS, newAS, &vk_d.basBufferEntityDynamic[vk.swapchain.currentImage], &vk_d.basBufferEntityDynamicOffset, RTX_BOTTOM_AS_FLAG);
				
				newAS->data.currInstanceID = vk_d.bottomASTraceListCount;
				newAS->data.prevInstanceID = preInstanceIDs[backEnd.currentEntity->e.id + drawSurf->bAS->surfcount];
				vk_d.prevToCurrInstance[newAS->data.prevInstanceID] = vk_d.bottomASTraceListCount;
				preInstanceIDs[backEnd.currentEntity->e.id + drawSurf->bAS->surfcount] = vk_d.bottomASTraceListCount;
				RB_UpdateInstanceDataBuffer(newAS);
				RB_UpdateInstanceBuffer(newAS);
				// add bottom to trace as list
				Com_Memcpy(&vk_d.bottomASTraceList[vk_d.bottomASTraceListCount], newAS, sizeof(vkbottomAS_t));
				vk_d.bottomASTraceListCount++;
			}
			else {
				//if ((backEnd.currentEntity->e.renderfx & RF_THIRD_PERSON)) continue;
				drawSurf->bAS->data.type = BAS_ENTITY_STATIC;
				drawSurf->bAS->data.cluster = cluster;

				if ((backEnd.currentEntity->e.renderfx & RF_FIRST_PERSON)) {
					drawSurf->bAS->data.isPlayer = qtrue;
				}
				else drawSurf->bAS->data.isPlayer = qfalse;

				Com_Memcpy(&drawSurf->bAS->geometryInstance.transform, &tM, sizeof(float[12]));	
				Com_Memcpy(&drawSurf->bAS->data.modelmat, &tM, sizeof(float[12]));
		
				drawSurf->bAS->data.currInstanceID = vk_d.bottomASTraceListCount;
				drawSurf->bAS->data.prevInstanceID = preInstanceIDs[backEnd.currentEntity->e.id + drawSurf->bAS->surfcount];
				vk_d.prevToCurrInstance[drawSurf->bAS->data.prevInstanceID] = vk_d.bottomASTraceListCount;
				preInstanceIDs[backEnd.currentEntity->e.id + drawSurf->bAS->surfcount] = vk_d.bottomASTraceListCount;
				RB_UpdateInstanceDataBuffer(drawSurf->bAS);
				RB_UpdateInstanceBuffer(drawSurf->bAS);
				// add bottom to trace as list
				Com_Memcpy(&vk_d.bottomASTraceList[vk_d.bottomASTraceListCount], drawSurf->bAS, sizeof(vkbottomAS_t));
				vk_d.bottomASTraceListCount++;
			}
			
			continue;
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

		if (strstr(shader->name, "bloodExplosion")) {// bloodExplotion
			int i = 2;
			//rb_surfaceTable[*drawSurf->surface](drawSurf->surface);
			//continue;
		}

		if (drawSurf->bAS == NULL && tess.numIndexes == 6 && tess.numVertexes == 4) {
			int i = 2;
		}
		if (strstr(shader->name, "blood")) {
			int i = 2;
		}
		if (drawSurf->bAS == NULL && drawSurf->surface == SF_ENTITY) {
			int x = 2;
		}
		if (backEnd.refdef.num_dlights > 0) {
			int x = 2;
		}
		//"models/mapobjects/gratelamp/gratelamp_b.tga" ...}, ...} ...}, ...}	textureBundle_t[2]

		continue;
		// add the triangles for this surface
		rb_surfaceTable[*drawSurf->surface](drawSurf->surface);
		
		if (drawSurf->bAS == NULL) {
			continue;
		}

	}
	backEnd.refdef.floatTime = originalTime;
	VK_UploadBufferDataOffset(&vk_d.prevToCurrInstanceBuffer[vk.swapchain.currentImage], 0, sizeof(vk_d.prevToCurrInstance), (void*)vk_d.prevToCurrInstance);

	VK_DestroyTopAccelerationStructure(&vk_d.topAS[vk.swapchain.currentImage]);
	VK_MakeTopAS(vk.swapchain.CurrentCommandBuffer(), &vk_d.topAS[vk.swapchain.currentImage], &vk_d.topASBuffer[vk.swapchain.currentImage], vk_d.bottomASTraceList, vk_d.bottomASTraceListCount, vk_d.instanceBuffer[vk.swapchain.currentImage], RTX_TOP_AS_FLAG);


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
	Com_Memcpy(projMatrix, &result, sizeof(result));
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
	GlobalUbo *ubo = &vk_d.uboGlobal[vk.swapchain.currentImage];
	ubo->frameIndex = tr.frameCount;//vk.swapchain.currentFrame;
	VectorCopy(origin, ubo->camPos);

	ubo->currentCluster = vk_d.currentCluster;
	ubo->clusterBytes = vk_d.clusterBytes;
	ubo->numClusters = vk_d.numClusters;

	ubo->showIntermediateResults = pt_showIntermediateResults->integer;
	ubo->cullLights = rt_cullLights->integer;
	ubo->numRandomDL = rt_numRandomDL->integer;
	ubo->numRandomIL = rt_numRandomIL->integer;
	ubo->numBounces = rt_numBounces->integer;
	ubo->randSampleLight = rt_softshadows->integer;
	ubo->denoiser = rt_denoiser->integer;
	ubo->taa = rt_taa->integer;
	ubo->debugLights = rt_debug_lights->integer;
	ubo->brightness = rt_brightness->value;
	ubo->tonemappingReinhard = rt_tonemapping_reinhard->value;

	ubo->width = vk.swapchain.extent.width;
	ubo->height = vk.swapchain.extent.height;
	ubo->mipmapLevel = vk_d.mipmapLevel;

	ubo->illumination = rt_illumination->integer;
	ubo->aperture = rt_aperture->value;
	ubo->focalLength = rt_focalLength->value;
	ubo->dof = rt_dof->integer;
	ubo->randomPixelOffset = rt_antialiasing->integer;
	ubo->accumulate = rt_accumulate->integer;

	ubo->ambient[0] = 0.06;
	ubo->ambient[1] = 0.06;
	ubo->ambient[2] = 0.03;

	if (strstr(tr.world->name, "q3dm1")) {
		ubo->ambient[0] = 0.06;
		ubo->ambient[1] = 0.05;
		ubo->ambient[2] = 0.05;
	}else if (strstr(tr.world->name, "q3dm7")) {
		ubo->ambient[0] = 0.07;
		ubo->ambient[1] = 0.07;
		ubo->ambient[2] = 0.03;
	}
	

	ubo->maxSamples = rt_maxSamples->integer;
	if (rt_accumulate->integer || rt_pause->integer) {
		Cvar_Set("cl_paused", "1");
		Cvar_Set("sv_paused", "1");
		int prevNumSamples = vk_d.uboGlobal[(vk.swapchain.currentImage + (vk.swapchain.imageCount - 1)) % vk.swapchain.imageCount].numSamples;
		if (rt_accumulate->integer && (ubo->maxSamples == 0 || prevNumSamples < ubo->maxSamples)) ubo->numSamples = prevNumSamples + 1;
		else if (!rt_accumulate->integer) ubo->numSamples = 0;
		else ubo->numSamples = prevNumSamples;
	}
	else {
		Cvar_Set("cl_paused", "0");
		Cvar_Set("sv_paused", "0");
		ubo->numSamples = 0;
	}

	float viewMatrix[16];
	// viewMatrix (needs flip)
	RB_BuildViewMatrix(&viewMatrix, &origin, &backEnd.viewParms. or .axis);
	// flip view matrix for vulkan
	myGlMultMatrix(&viewMatrix, &s_flipMatrix, &ubo->viewMat);
	// inverse view matrix
	myGLInvertMatrix(&ubo->viewMat, &ubo->inverseViewMat);
	// projection matrix
	RB_BuildProjMatrix(&ubo->projMat, backEnd.viewParms.projectionMatrix, backEnd.viewParms.zFar);
	// inverse proj matrix
	myGLInvertMatrix(&ubo->projMat, &ubo->inverseProjMat);
	
	// view portal
	vec3_t	originPortal;	// portal position
	//VectorCopy(vk_d.portalViewParms.pvsOrigin, originPortal);
	VectorCopy(vk_d.portalViewParms. or .origin, originPortal);
	if (vk_d.portalInView) {
		float	viewMatrixPortal[16];
		float	viewMatrixFlippedPortal[16];
		float	projMatrixPortal[16];

		// portal inv view mat
		RB_BuildViewMatrix(&viewMatrixPortal, &originPortal, &vk_d.portalViewParms. or .axis);
		myGlMultMatrix(&viewMatrixPortal, &s_flipMatrix, &viewMatrixFlippedPortal);
		myGLInvertMatrix(&viewMatrixFlippedPortal, &ubo->inverseViewMatPortal);
		// portal inv proj mat
		RB_BuildProjMatrix(&projMatrixPortal, vk_d.portalViewParms.projectionMatrix, vk_d.portalViewParms.zFar);
		myGLInvertMatrix(&projMatrixPortal, &ubo->inverseProjMatPortal);
	}
	ubo->hasPortal = vk_d.portalInView;
	// mvp
	myGlMultMatrix(&ubo->viewMat[0], ubo->projMat, vk_d.mvp);
	VK_UploadBufferDataOffset(&vk_d.uboBuffer[vk.swapchain.currentImage], 0, sizeof(GlobalUbo), (void*)ubo);

	VK_SetAccelerationStructure(&vk_d.rtxDescriptor[vk.swapchain.currentImage], BINDING_OFFSET_AS, VK_SHADER_STAGE_RAYGEN_BIT_NV, &vk_d.topAS[vk.swapchain.currentImage].accelerationStructure);
	VK_UpdateDescriptorSet(&vk_d.rtxDescriptor[vk.swapchain.currentImage]);
	
	// comp
	/*VkImageSubresourceRange subresource_range = { \
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, \
			.baseMipLevel = 0, \
			.levelCount = 1, \
			.baseArrayLayer = 0, \
			.layerCount = 1 \
	};
	vkCmdClearColorImage(cmd_buf, qvk.images[current_sample_pos_image],
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_grd_smpl_pos, 1, &subresource_range);*/

	VK_ClearImage(&vk_d.asvgf[vk.swapchain.currentImage].gradSamplePos, (VkClearColorValue) { .uint32 = { 0, 0, 0, 0 } }, VK_IMAGE_LAYOUT_GENERAL);// VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	VK_ClearImage(&vk_d.asvgf[vk.swapchain.currentImage].debug, (VkClearColorValue) { .uint32 = { 0, 0, 0, 0 } }, VK_IMAGE_LAYOUT_GENERAL);// VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	VK_ClearImage(&vk_d.gBuffer[vk.swapchain.currentImage].reflection, (VkClearColorValue) { .uint32 = { 0, 0, 0, 0 } }, VK_IMAGE_LAYOUT_GENERAL);// VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	BARRIER_COMPUTE(vk.swapchain.CurrentCommandBuffer(), vk_d.asvgf[vk.swapchain.currentImage].gradSamplePos.handle);
	BARRIER_COMPUTE(vk.swapchain.CurrentCommandBuffer(), vk_d.asvgf[vk.swapchain.currentImage].debug.handle);

	// ASVGF RNG
	PROFILER_SET_MARKER(vk.swapchain.CurrentCommandBuffer(), PROFILER_ASVGF_RNG_BEGIN);
	VK_BindComputePipeline(&vk_d.accelerationStructures.asvgfRngPipeline);
	VK_BindCompute2DescriptorSets(&vk_d.accelerationStructures.asvgfRngPipeline, &vk_d.computeDescriptor[vk.swapchain.currentImage], &vk_d.imageDescriptor);
	VK_Dispatch((vk.swapchain.extent.width + 31) / 32, (vk.swapchain.extent.height + 31) / 32, 1);
	PROFILER_SET_MARKER(vk.swapchain.CurrentCommandBuffer(), PROFILER_ASVGF_RNG_END);

	BARRIER_COMPUTE(vk.swapchain.CurrentCommandBuffer(), vk_d.asvgf[vk.swapchain.currentImage].rngSeed.handle);

	// ASVGF FORWARD
	PROFILER_SET_MARKER(vk.swapchain.CurrentCommandBuffer(), PROFILER_ASVGF_FORWARD_BEGIN);
	VK_BindComputePipeline(&vk_d.accelerationStructures.asvgfFwdPipeline);
	VK_BindCompute2DescriptorSets(&vk_d.accelerationStructures.asvgfFwdPipeline, &vk_d.computeDescriptor[vk.swapchain.currentImage], &vk_d.imageDescriptor);
	VK_Dispatch((vk.swapchain.extent.width / GRAD_DWN + 31) / 32, (vk.swapchain.extent.height / GRAD_DWN + 31) / 32, 1);
	PROFILER_SET_MARKER(vk.swapchain.CurrentCommandBuffer(), PROFILER_ASVGF_FORWARD_END);

	BARRIER_COMPUTE(vk.swapchain.CurrentCommandBuffer(), vk_d.asvgf[vk.swapchain.currentImage].rngSeed.handle);
	BARRIER_COMPUTE(vk.swapchain.CurrentCommandBuffer(), vk_d.asvgf[vk.swapchain.currentImage].positionFwd.handle);
	// primary rays
	PROFILER_SET_MARKER(vk.swapchain.CurrentCommandBuffer(), PROFILER_PRIMARY_RAYS_BEGIN);
	VK_BindRayTracingPipeline(&vk_d.primaryRaysPipeline);
	VK_Bind2RayTracingDescriptorSets(&vk_d.primaryRaysPipeline, &vk_d.rtxDescriptor[vk.swapchain.currentImage], &vk_d.imageDescriptor);
	VK_TraceRays(&vk_d.primaryRaysPipeline);
	PROFILER_SET_MARKER(vk.swapchain.CurrentCommandBuffer(), PROFILER_PRIMARY_RAYS_END);

	BARRIER_COMPUTE(vk.swapchain.CurrentCommandBuffer(), vk_d.gBuffer[vk.swapchain.currentImage].albedo.handle);
	BARRIER_COMPUTE(vk.swapchain.CurrentCommandBuffer(), vk_d.gBuffer[vk.swapchain.currentImage].position.handle);
	BARRIER_COMPUTE(vk.swapchain.CurrentCommandBuffer(), vk_d.gBuffer[vk.swapchain.currentImage].normals.handle);
	BARRIER_COMPUTE(vk.swapchain.CurrentCommandBuffer(), vk_d.gBuffer[vk.swapchain.currentImage].viewDir.handle);
	BARRIER_COMPUTE(vk.swapchain.CurrentCommandBuffer(), vk_d.gBuffer[vk.swapchain.currentImage].objectInfo.handle);

	// reflections/refractions
	PROFILER_SET_MARKER(vk.swapchain.CurrentCommandBuffer(), PROFILER_REFLECTION_REFRACTION_BEGIN);
	VK_BindRayTracingPipeline(&vk_d.reflectRaysPipeline);
	VK_Bind2RayTracingDescriptorSets(&vk_d.reflectRaysPipeline, &vk_d.rtxDescriptor[vk.swapchain.currentImage], &vk_d.imageDescriptor);
	VK_TraceRays(&vk_d.reflectRaysPipeline);
	PROFILER_SET_MARKER(vk.swapchain.CurrentCommandBuffer(), PROFILER_REFLECTION_REFRACTION_END);

	BARRIER_COMPUTE(vk.swapchain.CurrentCommandBuffer(), vk_d.gBuffer[vk.swapchain.currentImage].reflection.handle);
	
	// direct Ill
	PROFILER_SET_MARKER(vk.swapchain.CurrentCommandBuffer(), PROFILER_DIRECT_ILLUMINATION_BEGIN);
	VK_BindRayTracingPipeline(&vk_d.directIlluminationPipeline);
	VK_Bind2RayTracingDescriptorSets(&vk_d.directIlluminationPipeline, &vk_d.rtxDescriptor[vk.swapchain.currentImage], &vk_d.imageDescriptor);
	VK_TraceRays(&vk_d.directIlluminationPipeline);
	PROFILER_SET_MARKER(vk.swapchain.CurrentCommandBuffer(), PROFILER_DIRECT_ILLUMINATION_END);

	// indirect Ill
	if (ubo->numBounces > 0) {
		/*PROFILER_SET_MARKER(vk.swapchain.CurrentCommandBuffer(), PROFILER_INDIRECT_ILLUMINATION_BEGIN);
		VK_BindRayTracingPipeline(&vk_d.indirectIlluminationPipeline);
		VK_Bind2RayTracingDescriptorSets(&vk_d.indirectIlluminationPipeline, &vk_d.rtxDescriptor[vk.swapchain.currentImage], &vk_d.imageDescriptor);
		VK_TraceRays(&vk_d.indirectIlluminationPipeline);
		PROFILER_SET_MARKER(vk.swapchain.CurrentCommandBuffer(), PROFILER_INDIRECT_ILLUMINATION_END);*/
	}

	if (rt_denoiser->integer) {
		BARRIER_COMPUTE(vk.swapchain.CurrentCommandBuffer(), vk_d.gBuffer[vk.swapchain.currentImage].directIllumination.handle);

		PROFILER_SET_MARKER(vk.swapchain.CurrentCommandBuffer(), PROFILER_ASVGF_GRADIENT_BEGIN);
		VK_BindComputePipeline(&vk_d.accelerationStructures.asvgfGradImgPipeline);
		VK_BindCompute2DescriptorSets(&vk_d.accelerationStructures.asvgfGradImgPipeline, &vk_d.computeDescriptor[vk.swapchain.currentImage], &vk_d.imageDescriptor);
		VK_Dispatch((vk.swapchain.extent.width / GRAD_DWN + 31) / 32, (vk.swapchain.extent.height / GRAD_DWN + 31) / 32, 1);
		PROFILER_SET_MARKER(vk.swapchain.CurrentCommandBuffer(), PROFILER_ASVGF_GRADIENT_END);

		BARRIER_COMPUTE(vk.swapchain.CurrentCommandBuffer(), vk_d.asvgf[vk.swapchain.currentImage].gradA.handle);

		PROFILER_SET_MARKER(vk.swapchain.CurrentCommandBuffer(), PROFILER_ASVGF_GRADIENT_ATROUS_BEGIN);
		VK_BindComputePipeline(&vk_d.accelerationStructures.asvgfGradAtrousPipeline);
		VK_BindCompute2DescriptorSets(&vk_d.accelerationStructures.asvgfGradAtrousPipeline, &vk_d.computeDescriptor[vk.swapchain.currentImage], &vk_d.imageDescriptor);
		const int num_atrous_iterations_gradient = 5;
		for (uint32_t i = 0; i < num_atrous_iterations_gradient; i++) {
			VK_SetComputePushConstant(&vk_d.accelerationStructures.asvgfGradAtrousPipeline, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &i);
			VK_Dispatch((vk.swapchain.extent.width / GRAD_DWN + 31) / 32, (vk.swapchain.extent.height / GRAD_DWN + 31) / 32, 1);
			BARRIER_COMPUTE(vk.swapchain.CurrentCommandBuffer(), vk_d.asvgf[vk.swapchain.currentImage].gradA.handle);
			BARRIER_COMPUTE(vk.swapchain.CurrentCommandBuffer(), vk_d.asvgf[vk.swapchain.currentImage].gradB.handle);
		}
		PROFILER_SET_MARKER(vk.swapchain.CurrentCommandBuffer(), PROFILER_ASVGF_GRADIENT_ATROUS_END);

		// ASVGF TEMPORAL
		PROFILER_SET_MARKER(vk.swapchain.CurrentCommandBuffer(), PROFILER_ASVGF_TEMPORAL_BEGIN);
		VK_BindComputePipeline(&vk_d.accelerationStructures.asvgfTemporalPipeline);
		VK_BindCompute2DescriptorSets(&vk_d.accelerationStructures.asvgfTemporalPipeline, &vk_d.computeDescriptor[vk.swapchain.currentImage], &vk_d.imageDescriptor);
		VK_Dispatch((vk.swapchain.extent.width + 31) / 32, (vk.swapchain.extent.height + 31) / 32, 1);
		PROFILER_SET_MARKER(vk.swapchain.CurrentCommandBuffer(), PROFILER_ASVGF_TEMPORAL_END);

		BARRIER_COMPUTE(vk.swapchain.CurrentCommandBuffer(), vk_d.asvgf[vk.swapchain.currentImage].atrousA.handle);
		BARRIER_COMPUTE(vk.swapchain.CurrentCommandBuffer(), vk_d.asvgf[vk.swapchain.currentImage].atrousB.handle);
		BARRIER_COMPUTE(vk.swapchain.CurrentCommandBuffer(), vk_d.asvgf[vk.swapchain.currentImage].histColor.handle);
		BARRIER_COMPUTE(vk.swapchain.CurrentCommandBuffer(), vk_d.asvgf[vk.swapchain.currentImage].histMoments.handle);

		// ASVGF ATROUS
		PROFILER_SET_MARKER(vk.swapchain.CurrentCommandBuffer(), PROFILER_ASVGF_ATROUS_BEGIN);
		VK_BindComputePipeline(&vk_d.accelerationStructures.asvgfAtrousPipeline);
		VK_BindCompute2DescriptorSets(&vk_d.accelerationStructures.asvgfAtrousPipeline, &vk_d.computeDescriptor[vk.swapchain.currentImage], &vk_d.imageDescriptor);
		const int num_atrous_iterations = 5;
		for (uint32_t i = 0; i < 5; i++) {
			VK_SetComputePushConstant(&vk_d.accelerationStructures.asvgfAtrousPipeline, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &i);
			VK_Dispatch((vk.swapchain.extent.width + 31) / 32, (vk.swapchain.extent.height + 31) / 32, 1);
			BARRIER_COMPUTE(vk.swapchain.CurrentCommandBuffer(), vk_d.asvgf[vk.swapchain.currentImage].atrousA.handle);
			BARRIER_COMPUTE(vk.swapchain.CurrentCommandBuffer(), vk_d.asvgf[vk.swapchain.currentImage].atrousB.handle);
			BARRIER_COMPUTE(vk.swapchain.CurrentCommandBuffer(), vk_d.asvgf[vk.swapchain.currentImage].histColor.handle);
		}
		PROFILER_SET_MARKER(vk.swapchain.CurrentCommandBuffer(), PROFILER_ASVGF_ATROUS_END);


	}
	// compositing
	VK_BindComputePipeline(&vk_d.accelerationStructures.compositingPipeline);
	VK_BindCompute2DescriptorSets(&vk_d.accelerationStructures.compositingPipeline, &vk_d.computeDescriptor[vk.swapchain.currentImage], &vk_d.imageDescriptor);
	VK_Dispatch((vk.swapchain.extent.width + 31) / 32, (vk.swapchain.extent.height + 31) / 32, 1);
	BARRIER_COMPUTE(vk.swapchain.CurrentCommandBuffer(), vk_d.asvgf[vk.swapchain.currentImage].color.handle);
	
	

	BARRIER_COMPUTE(vk.swapchain.CurrentCommandBuffer(), vk_d.asvgf[vk.swapchain.currentImage].color.handle);
	// ASVGF TAA
	if (rt_taa->integer == 1) {
		PROFILER_SET_MARKER(vk.swapchain.CurrentCommandBuffer(), PROFILER_ASVGF_TAA_BEGIN);
		VK_BindComputePipeline(&vk_d.accelerationStructures.asvgfTaaPipeline);
		VK_BindCompute2DescriptorSets(&vk_d.accelerationStructures.asvgfTaaPipeline, &vk_d.computeDescriptor[vk.swapchain.currentImage], &vk_d.imageDescriptor);
		VK_Dispatch((vk.swapchain.extent.width + 31) / 32, (vk.swapchain.extent.height + 31) / 32, 1);
		PROFILER_SET_MARKER(vk.swapchain.CurrentCommandBuffer(), PROFILER_ASVGF_TAA_END);
	}

	BARRIER_COMPUTE(vk.swapchain.CurrentCommandBuffer(), vk_d.gBuffer[vk.swapchain.currentImage].viewDir.handle);
	if (rt_tonemapping_reinhard->value == 1) {
		BARRIER_COMPUTE(vk.swapchain.CurrentCommandBuffer(), vk_d.asvgf[vk.swapchain.currentImage].taa.handle);
		for (uint32_t i = 0; i < vk_d.mipmapLevel; i++) {
			VK_BindComputePipeline(&vk_d.accelerationStructures.maxmipmapPipeline);
			VK_BindCompute2DescriptorSets(&vk_d.accelerationStructures.maxmipmapPipeline, &vk_d.computeDescriptor[vk.swapchain.currentImage], &vk_d.imageDescriptor);
			VK_SetComputePushConstant(&vk_d.accelerationStructures.maxmipmapPipeline, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &i);
			//if(i == 0) VK_Dispatch((vk.swapchain.extent.width), (vk.swapchain.extent.height ), 1);
			//else
			VK_Dispatch((vk.swapchain.extent.width) / (2 * (i + 1)), (vk.swapchain.extent.height) / (2 * (i + 1)), 1);
			BARRIER_COMPUTE(vk.swapchain.CurrentCommandBuffer(), vk_d.gBuffer[vk.swapchain.currentImage].maxmipmap.handle);
		}

		//VK_BindComputePipeline(&vk_d.accelerationStructures.maxmipmapPipeline);
		//VK_BindCompute2DescriptorSets(&vk_d.accelerationStructures.maxmipmapPipeline, &vk_d.computeDescriptor[vk.swapchain.currentImage], &vk_d.imageDescriptor);
		//int a = 0;
		//VK_SetComputePushConstant(&vk_d.accelerationStructures.maxmipmapPipeline, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &a);
		//VK_Dispatch(vk.swapchain.extent.width, vk.swapchain.extent.height, 1);
		BARRIER_COMPUTE(vk.swapchain.CurrentCommandBuffer(), vk_d.asvgf[vk.swapchain.currentImage].taa.handle);
		VK_BindComputePipeline(&vk_d.accelerationStructures.tonemappingPipeline);
		VK_BindCompute2DescriptorSets(&vk_d.accelerationStructures.tonemappingPipeline, &vk_d.computeDescriptor[vk.swapchain.currentImage], &vk_d.imageDescriptor);
		int a = vk_d.mipmapLevel - 1;
		VK_SetComputePushConstant(&vk_d.accelerationStructures.tonemappingPipeline, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &a);
		VK_Dispatch((vk.swapchain.extent.width), (vk.swapchain.extent.height), 1);
	}

}

static void
get_aabb_corner(vec3_t mins, vec3_t maxs, int corner_idx, float *corner)
{
	corner[0] = (corner_idx & 1) ? maxs[0] : mins[0];
	corner[1] = (corner_idx & 2) ? maxs[1] : mins[1];
	corner[2] = (corner_idx & 4) ? maxs[2] : mins[2];
	corner[3] = 0;
}

static void drawCluster() {

	VK_Bind(tr.whiteImage);
	tr_api.State(GLS_POLYMODE_LINE | GLS_DEPTHMASK_TRUE);
	vk_d.viewport.minDepth = 0;
	vk_d.viewport.maxDepth = 0;
	vk_d.state.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
	vk_d.state.polygonMode = VK_POLYGON_MODE_LINE;
	vk_d.state.cullMode = VK_CULL_MODE_NONE;
	vk_d.state.dsBlend.depthTestEnable = VK_FALSE;
	tr_api.State(GLS_SRCBLEND_ONE | GLS_DSTBLEND_ZERO | GLS_DEPTHTEST_DISABLE);

	vec4_t* xyz = malloc(4 * 8 * tr.world->numClusters * sizeof(vec4_t));
	color4ub_t* colors = malloc(4 * 8 * tr.world->numClusters * sizeof(color4ub_t));
	uint32_t* indexes = malloc(4 * 24 * tr.world->numClusters * sizeof(uint32_t));

	int vertexCount = 0;
	int idxCount = 0;
	for (int i = 0; i < tr.world->numClusters; i++) {
		vec4_t points[8];
		color4ub_t color = { 0,255,0,0 };
		if (i == 1333) color[0] = 255;
		//if (i != 1333) continue;
		uint32_t offset = vertexCount;
		for (uint32_t idx = 0; idx < 8; idx++) {
			get_aabb_corner(vk_d.clusterList[i].mins, vk_d.clusterList[i].maxs, idx, &points[idx]);
			Com_Memcpy(&colors[vertexCount + idx], &color, sizeof(color4ub_t));
		}
		Com_Memcpy(&xyz[vertexCount], &points[0], 8 * sizeof(vec4_t));
		vertexCount += 8;

		indexes[idxCount] = offset + 0; idxCount++; indexes[idxCount] = offset + 1; idxCount++;
		indexes[idxCount] = offset + 0; idxCount++; indexes[idxCount] = offset + 2; idxCount++;
		indexes[idxCount] = offset + 0; idxCount++; indexes[idxCount] = offset + 4; idxCount++;

		indexes[idxCount] = offset + 3; idxCount++; indexes[idxCount] = offset + 1; idxCount++;
		indexes[idxCount] = offset + 3; idxCount++; indexes[idxCount] = offset + 2; idxCount++;
		indexes[idxCount] = offset + 3; idxCount++; indexes[idxCount] = offset + 7; idxCount++;

		indexes[idxCount] = offset + 5; idxCount++; indexes[idxCount] = offset + 1; idxCount++;
		indexes[idxCount] = offset + 5; idxCount++; indexes[idxCount] = offset + 4; idxCount++;
		indexes[idxCount] = offset + 5; idxCount++; indexes[idxCount] = offset + 7; idxCount++;

		indexes[idxCount] = offset + 6; idxCount++; indexes[idxCount] = offset + 2; idxCount++;
		indexes[idxCount] = offset + 6; idxCount++; indexes[idxCount] = offset + 4; idxCount++;
		indexes[idxCount] = offset + 6; idxCount++; indexes[idxCount] = offset + 7; idxCount++;
	}

	VK_UploadBufferDataOffset(&vk_d.colorbuffer, vk_d.offset * sizeof(color4ub_t), vertexCount * sizeof(color4ub_t), (void*)&colors[0]);
	VK_UploadBufferDataOffset(&vk_d.vertexbuffer, vk_d.offset * sizeof(vec4_t), vertexCount * sizeof(vec4_t), (void*)&xyz[0]);
	tr_api.R_DrawElements(idxCount, indexes);

	free(xyz);
	free(colors);
	free(indexes);

	vk_d.viewport.minDepth = 0;
	vk_d.viewport.maxDepth = 1;
	vk_d.state.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	vk_d.offset += vertexCount;
	vk_d.offsetIdx += idxCount;
	
}

void RB_RayTraceScene(drawSurf_t* drawSurfs, int numDrawSurfs) {
	VkMemoryBarrier memoryBarrier = { 0 };
	vkCmdEndRenderPass(vk.swapchain.CurrentCommandBuffer());

	// destroy all dynamic as for this frame
	for (int i = 0; i < vk_d.bottomASDynamicCount[vk.swapchain.currentImage]; i++) {
		VK_DestroyBottomAccelerationStructure(&(vk_d.bottomASDynamicList[vk.swapchain.currentImage][i]));
	}
	vk_d.bottomASDynamicCount[vk.swapchain.currentImage] = 0;
	// reset offsets
	vk_d.basBufferEntityDynamicOffset = 0;
	vk_d.geometry.idx_entity_dynamic_offset = 0;
	vk_d.geometry.xyz_entity_dynamic_offset = 0;
	vk_d.scratchBufferOffset = 0;
	// reset trace list for this frame
	vk_d.bottomASTraceListCount = 0; 
	// reset performance marker query
	VK_ResetPerformanceQueryPool(vk.swapchain.CurrentCommandBuffer());

	PROFILER_SET_MARKER(vk.swapchain.CurrentCommandBuffer(), PROFILER_BUILD_AS_BEGIN);
	RB_UpdateRayTraceAS(drawSurfs, numDrawSurfs);
	PROFILER_SET_MARKER(vk.swapchain.CurrentCommandBuffer(), PROFILER_BUILD_AS_END);

	RB_TraceRays();

	memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV| VK_ACCESS_MEMORY_WRITE_BIT;
	memoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
	vkCmdPipelineBarrier(vk.swapchain.CurrentCommandBuffer(), VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, 0, 1, &memoryBarrier, 0, 0, 0, 0);

	vk_d.portalInView = qfalse;
	vk_d.mirrorInView = qfalse;

	// draw rt results to swap chain
	VK_BeginRenderClear();

	// create descriptor
	vkimage_t* drawImage;
	/*if(rt_denoiser->integer) drawImage = &vk_d.asvgf[vk.swapchain.currentImage].taa;
	else */
	//vkimage_t* drawImage = &vk_d.accelerationStructures.resultImage[vk.swapchain.currentImage];
	//vkimage_t* drawImage = &vk_d.accelerationStructures.resultImage[vk.swapchain.currentImage];
	//vkimage_t* drawImage = &vk_d.gBuffer[vk.swapchain.currentImage].objectInfo;
	//vkimage_t* drawImage = &vk_d.gBuffer[vk.swapchain.currentImage].color;
	//vkimage_t* drawImage = &vk_d.gBuffer[vk.swapchain.currentImage].indirectIllumination;
	//vkimage_t* drawImage = &vk_d.asvgf[vk.swapchain.currentImage].debug;
	//vkimage_t* drawImage = &vk_d.asvgf[vk.swapchain.currentImage].atrousA;
	if(rt_tonemapping_reinhard->value == 1) drawImage = &vk_d.gBuffer[vk.swapchain.currentImage].result;
	else drawImage = &vk_d.asvgf[vk.swapchain.currentImage].taa;
	//vkimage_t* drawImage = &vk_d.gBuffer[vk.swapchain.currentImage].transparent;
	//vkimage_t* drawImage = &vk_d.asvgf[vk.swapchain.currentImage].color;
	//vkimage_t* drawImage = &vk_d.asvgf[vk.swapchain.currentImage].gradSamplePos;
	{
		if (drawImage->descriptor_set.set == NULL) {
			VK_AddSampler(&drawImage->descriptor_set, 0, VK_SHADER_STAGE_FRAGMENT_BIT);
			drawImage->descriptor_set.data[0].descImageInfo->imageLayout = VK_IMAGE_LAYOUT_GENERAL;
			VK_SetSampler(&drawImage->descriptor_set, 0, VK_SHADER_STAGE_FRAGMENT_BIT, drawImage->sampler, drawImage->view);
			VK_FinishDescriptor(&drawImage->descriptor_set);
		}	
	}
	VK_DrawFullscreenRect(drawImage);

	if (r_showcluster->integer) drawCluster();
	
	if (rt_printPerformanceStatistic->integer > 0) {
		VK_PerformanceQueryPoolResults();
		double build, rng, fwd, prim, refref, direct, gradient, grad_atrous, temporal, atrous, taa;
		VK_TimeBetweenMarkers(&build, PROFILER_BUILD_AS_BEGIN, PROFILER_BUILD_AS_END);
		VK_TimeBetweenMarkers(&rng, PROFILER_ASVGF_RNG_BEGIN, PROFILER_ASVGF_RNG_END);
		VK_TimeBetweenMarkers(&fwd, PROFILER_ASVGF_FORWARD_BEGIN, PROFILER_ASVGF_FORWARD_END);
		VK_TimeBetweenMarkers(&prim, PROFILER_PRIMARY_RAYS_BEGIN, PROFILER_PRIMARY_RAYS_END);
		VK_TimeBetweenMarkers(&refref, PROFILER_REFLECTION_REFRACTION_BEGIN, PROFILER_REFLECTION_REFRACTION_END);
		VK_TimeBetweenMarkers(&direct, PROFILER_DIRECT_ILLUMINATION_BEGIN, PROFILER_DIRECT_ILLUMINATION_END);
		VK_TimeBetweenMarkers(&gradient, PROFILER_ASVGF_GRADIENT_BEGIN, PROFILER_ASVGF_GRADIENT_END);
		VK_TimeBetweenMarkers(&grad_atrous, PROFILER_ASVGF_GRADIENT_ATROUS_BEGIN, PROFILER_ASVGF_GRADIENT_ATROUS_END);
		VK_TimeBetweenMarkers(&temporal, PROFILER_ASVGF_TEMPORAL_BEGIN, PROFILER_ASVGF_TEMPORAL_END);
		VK_TimeBetweenMarkers(&atrous, PROFILER_ASVGF_ATROUS_BEGIN, PROFILER_ASVGF_ATROUS_END);
		VK_TimeBetweenMarkers(&taa, PROFILER_ASVGF_TAA_BEGIN, PROFILER_ASVGF_TAA_END);
		ri.Printf(PRINT_ALL, "Build/Update AS           %f ms\n", build);
		ri.Printf(PRINT_ALL, "ASVGF RNG                 %f ms\n", rng);
		ri.Printf(PRINT_ALL, "ASVGF Forward             %f ms\n", fwd);
		ri.Printf(PRINT_ALL, "Primary Ray Stage         %f ms\n", prim);
		ri.Printf(PRINT_ALL, "Reflect/Refract Stage     %f ms\n", refref);
		ri.Printf(PRINT_ALL, "Direct Illumination Stage %f ms\n", direct);
		ri.Printf(PRINT_ALL, "ASVGF Gradient            %f ms\n", gradient);
		ri.Printf(PRINT_ALL, "ASVGF Gradient Atrous     %f ms\n", grad_atrous);
		ri.Printf(PRINT_ALL, "ASVGF Temporal            %f ms\n", temporal);
		ri.Printf(PRINT_ALL, "ASVGF Atrous              %f ms\n", atrous);
		ri.Printf(PRINT_ALL, "ASVGF Taa                 %f ms\n", taa);
		ri.Printf(PRINT_ALL, "Accumulated Images        %d\n", vk_d.uboGlobal[vk.swapchain.currentImage].numSamples);
		ri.Printf(PRINT_ALL, "ASVGF SUM                 %f ms\n", rng + fwd + gradient + grad_atrous + temporal + atrous + taa);
		ri.Cvar_Set("rt_printPerfStats", 0);
	}

}

