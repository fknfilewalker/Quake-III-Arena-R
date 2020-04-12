#include "tr_local.h"
#include "../../shader/glsl/constants.h"

/*
glConfig.driverType == VULKAN && r_vertexLight->value == 2
*/

#define RTX_BOTTOM_AS_FLAG (VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV | VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV)
#define RTX_TOP_AS_FLAG (VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV)

qboolean RB_ClusterVisIdent(byte* aVis, byte* bVis) {
	if(!memcmp(aVis, bVis, vk_d.clusterBytes * sizeof(byte))) return qtrue;
	/*for (int b = 0; b < vk_d.clusterBytes; b++) {
		if (aVis[b] != bVis[b]) return qfalse;
	}*/
	return qfalse;
}

int RB_CheckClusterExist(byte* cVis) {
	for (int i = vk_d.numFixedCluster; i < vk_d.numClusters; i++) {
		byte* allVis = (vk_d.vis + i * vk_d.clusterBytes);
		if (RB_ClusterVisIdent(allVis, cVis)) return i;
	}
	return -1;
}

int RB_TryMergeCluster(int cluster[3], int defaultC) {
	if ((cluster[0] == -1 && cluster[1] == -1) || (cluster[1] == -1 && cluster[2] == -1) || (cluster[2] == -1 && cluster[0] == -1)) return -1;
	//if ((cluster[0] == -1 && cluster[1] == -1) && cluster[2] == defaultC) return -1;
	//if ((cluster[1] == -1 && cluster[2] == -1) && cluster[0] == defaultC) return -1;
	//if ((cluster[2] == -1 && cluster[0] == -1) && cluster[1] == defaultC) return -1;
	//if ((cluster[0] == -1 && cluster[1] == -1) || (cluster[1] == -1 && cluster[2] == -1) || (cluster[2] == -1 && cluster[0] == -1)) return -1;

	if (cluster[0] != cluster[1] || cluster[1] != cluster[2] || cluster[0] != cluster[2]) {
		vec3_t mins = { 99999, 99999, 99999 };
		vec3_t maxs = { -99999, -99999, -99999 };
		if (cluster[0] != -1) {
			mins[0] = mins[0] < vk_d.clusterList[cluster[0]].mins[0] ? mins[0] : vk_d.clusterList[cluster[0]].mins[0];
			mins[1] = mins[1] < vk_d.clusterList[cluster[0]].mins[1] ? mins[1] : vk_d.clusterList[cluster[0]].mins[1];
			mins[2] = mins[2] < vk_d.clusterList[cluster[0]].mins[2] ? mins[2] : vk_d.clusterList[cluster[0]].mins[2];
			maxs[0] = maxs[0] > vk_d.clusterList[cluster[0]].maxs[0] ? maxs[0] : vk_d.clusterList[cluster[0]].maxs[0];
			maxs[1] = maxs[1] > vk_d.clusterList[cluster[0]].maxs[1] ? maxs[1] : vk_d.clusterList[cluster[0]].maxs[1];
			maxs[2] = maxs[2] > vk_d.clusterList[cluster[0]].maxs[2] ? maxs[2] : vk_d.clusterList[cluster[0]].maxs[2];
		}
		if (cluster[1] != -1) {
			mins[0] = mins[0] < vk_d.clusterList[cluster[1]].mins[0] ? mins[0] : vk_d.clusterList[cluster[1]].mins[0];
			mins[1] = mins[1] < vk_d.clusterList[cluster[1]].mins[1] ? mins[1] : vk_d.clusterList[cluster[1]].mins[1];
			mins[2] = mins[2] < vk_d.clusterList[cluster[1]].mins[2] ? mins[2] : vk_d.clusterList[cluster[1]].mins[2];
			maxs[0] = maxs[0] > vk_d.clusterList[cluster[1]].maxs[0] ? maxs[0] : vk_d.clusterList[cluster[1]].maxs[0];
			maxs[1] = maxs[1] > vk_d.clusterList[cluster[1]].maxs[1] ? maxs[1] : vk_d.clusterList[cluster[1]].maxs[1];
			maxs[2] = maxs[2] > vk_d.clusterList[cluster[1]].maxs[2] ? maxs[2] : vk_d.clusterList[cluster[1]].maxs[2];
		}
		if (cluster[2] != -1) {
			mins[0] = mins[0] < vk_d.clusterList[cluster[2]].mins[0] ? mins[0] : vk_d.clusterList[cluster[2]].mins[0];
			mins[1] = mins[1] < vk_d.clusterList[cluster[2]].mins[1] ? mins[1] : vk_d.clusterList[cluster[2]].mins[1];
			mins[2] = mins[2] < vk_d.clusterList[cluster[2]].mins[2] ? mins[2] : vk_d.clusterList[cluster[2]].mins[2];
			maxs[0] = maxs[0] > vk_d.clusterList[cluster[2]].maxs[0] ? maxs[0] : vk_d.clusterList[cluster[2]].maxs[0];
			maxs[1] = maxs[1] > vk_d.clusterList[cluster[2]].maxs[1] ? maxs[1] : vk_d.clusterList[cluster[2]].maxs[1];
			maxs[2] = maxs[2] > vk_d.clusterList[cluster[2]].maxs[2] ? maxs[2] : vk_d.clusterList[cluster[2]].maxs[2];
		}
		if (defaultC != -1) {
			mins[0] = mins[0] < vk_d.clusterList[defaultC].mins[0] ? mins[0] : vk_d.clusterList[defaultC].mins[0];
			mins[1] = mins[1] < vk_d.clusterList[defaultC].mins[1] ? mins[1] : vk_d.clusterList[defaultC].mins[1];
			mins[2] = mins[2] < vk_d.clusterList[defaultC].mins[2] ? mins[2] : vk_d.clusterList[defaultC].mins[2];
			maxs[0] = maxs[0] > vk_d.clusterList[defaultC].maxs[0] ? maxs[0] : vk_d.clusterList[defaultC].maxs[0];
			maxs[1] = maxs[1] > vk_d.clusterList[defaultC].maxs[1] ? maxs[1] : vk_d.clusterList[defaultC].maxs[1];
			maxs[2] = maxs[2] > vk_d.clusterList[defaultC].maxs[2] ? maxs[2] : vk_d.clusterList[defaultC].maxs[2];
		}


		byte* vis = calloc(1, sizeof(byte) * vk_d.clusterBytes);
		for (int i = 0; i < vk_d.numFixedCluster; i++) {
			if ((vk_d.clusterList[i].mins[0] >= mins[0] && vk_d.clusterList[i].mins[1] >= mins[1] && vk_d.clusterList[i].mins[2] >= mins[2]) &&
				(vk_d.clusterList[i].maxs[0] <= maxs[0] && vk_d.clusterList[i].maxs[1] <= maxs[1] && vk_d.clusterList[i].maxs[2] <= maxs[2])) {

				byte* allVis = (vk_d.vis + i * vk_d.clusterBytes);
				for (int b = 0; b < vk_d.clusterBytes; b++) {

					vis[b] = vis[b] | allVis[b];
				}
				//const byte* clusterVis = vk_d.vis + cluster * s_worldData.clusterBytes;
				int x = 2;
			}
		}

		int c = RB_CheckClusterExist(vis);
		if (c == -1) {
			byte* allVis = (vk_d.vis + vk_d.numClusters * vk_d.clusterBytes);
			for (int b = 0; b < vk_d.clusterBytes; b++) {
				allVis[b] = vis[b];
			}
			c = vk_d.numClusters;
			vk_d.numClusters++;
		}
		free(vis);
		
		return c;

		//else c = 0;
	}
	return -1;
}
int RB_GetCluster() {
	vec3_t mins = { 99999, 99999, 99999 };
	vec3_t maxs = { -99999, -99999, -99999 };

	for (int i = 0; i < (tess.numVertexes); i++) {
		int cluster = R_FindClusterForPos3(tess.xyz[i]);
		if(cluster == -1) cluster = R_FindClusterForPos(tess.xyz[i]);
		if (cluster == -1) cluster = R_FindClusterForPos2(tess.xyz[i]);

		if (cluster != -1) {
			mins[0] = mins[0] < vk_d.clusterList[cluster].mins[0] ? mins[0] : vk_d.clusterList[cluster].mins[0];
			mins[1] = mins[1] < vk_d.clusterList[cluster].mins[1] ? mins[1] : vk_d.clusterList[cluster].mins[1];
			mins[2] = mins[2] < vk_d.clusterList[cluster].mins[2] ? mins[2] : vk_d.clusterList[cluster].mins[2];
			maxs[0] = maxs[0] > vk_d.clusterList[cluster].maxs[0] ? maxs[0] : vk_d.clusterList[cluster].maxs[0];
			maxs[1] = maxs[1] > vk_d.clusterList[cluster].maxs[1] ? maxs[1] : vk_d.clusterList[cluster].maxs[1];
			maxs[2] = maxs[2] > vk_d.clusterList[cluster].maxs[2] ? maxs[2] : vk_d.clusterList[cluster].maxs[2];
		}
	}

	byte* vis = calloc(1, sizeof(byte) * vk_d.clusterBytes);
	for (int i = 0; i < vk_d.numFixedCluster; i++) {
		if ((vk_d.clusterList[i].mins[0] >= mins[0] && vk_d.clusterList[i].mins[1] >= mins[1] && vk_d.clusterList[i].mins[2] >= mins[2]) &&
			(vk_d.clusterList[i].maxs[0] <= maxs[0] && vk_d.clusterList[i].maxs[1] <= maxs[1] && vk_d.clusterList[i].maxs[2] <= maxs[2])) {

			byte* allVis = (vk_d.vis + i * vk_d.clusterBytes);
			for (int b = 0; b < vk_d.clusterBytes; b++) {

				vis[b] = vis[b] | allVis[b];
			}
			//const byte* clusterVis = vk_d.vis + cluster * s_worldData.clusterBytes;
			int x = 2;
		}

	}

	byte* allVis = (vk_d.vis + vk_d.numClusters * vk_d.clusterBytes);
	for (int b = 0; b < vk_d.clusterBytes; b++) {
		allVis[b] = vis[b];
	}

	free(vis);
	int c = vk_d.numClusters;
	vk_d.numClusters++;
	return c;
}

void RB_UploadCluster(vkbuffer_t* buffer, uint32_t offsetIDX, int defaultC) {
	uint32_t* clusterData = calloc(tess.numIndexes/3, sizeof(uint32_t));
	for (int i = 0; i < (tess.numIndexes / 3); i++) {
		vec4_t pos = { 0,0,0,0 };
		for (int j = 0; j < 3; j++) {
			VectorAdd(pos, tess.xyz[tess.indexes[(i * 3) + j]], pos);
			VectorAdd(pos, tess.normal[tess.indexes[(i * 3) + j]], pos);
		}
		VectorScale(pos, 1.0f / 3.0f, pos);
		int c = -1;//R_FindClusterForPos(pos);

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

		c = RB_TryMergeCluster(cluster, defaultC);

		/*if (c == -1) {
			cluster[0] = R_FindClusterForPos3(tess.xyz[tess.indexes[(i * 3) + 0]]);
			cluster[1] = R_FindClusterForPos3(tess.xyz[tess.indexes[(i * 3) + 1]]);
			cluster[2] = R_FindClusterForPos3(tess.xyz[tess.indexes[(i * 3) + 2]]);

			if (cluster[0] == -1) cluster[0] = R_FindClusterForPos(tess.xyz[tess.indexes[(i * 3) + 0]]);
			if (cluster[1] == -1) cluster[1] = R_FindClusterForPos(tess.xyz[tess.indexes[(i * 3) + 1]]);
			if (cluster[2] == -1) cluster[2] = R_FindClusterForPos(tess.xyz[tess.indexes[(i * 3) + 2]]);

			if (cluster[0] == -1) cluster[0] = R_FindClusterForPos3(tess.xyz[tess.indexes[(i * 3) + 0]]);
			if (cluster[1] == -1) cluster[1] = R_FindClusterForPos3(tess.xyz[tess.indexes[(i * 3) + 1]]);
			if (cluster[2] == -1) cluster[2] = R_FindClusterForPos3(tess.xyz[tess.indexes[(i * 3) + 2]]);
			c = RB_TryMergeCluster(cluster, defaultC);
		}*/
		
		//if (c == -1 && (clister0 == -1 && clister1 == -1)) c = clister2;
		//if (c == -1 && (clister1 == -1 && clister2 == -1)) c = clister0;
		//if (c == -1 && (clister2 == -1 && clister0 == -1)) c = clister1;
		
		if (c == -1) c = R_FindClusterForPos2(pos);
		if (c == -1) c = R_FindClusterForPos(pos);
		if (c == -1) c = R_FindClusterForPos3(pos);
		if (c == -1) {
			c = defaultC;
		}
		//c = 218;
		//c = R_ClosestCluster(pos);
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

void RB_CreateEntityBottomAS(vkbottomAS_t** bAS) {
	// set offsets and 
	uint32_t* idxOffset = &vk_d.geometry.idx_entity_static_offset;
	uint32_t* xyzOffset = &vk_d.geometry.xyz_entity_static_offset;
	// save bas in static or dynamic list
	vkbottomAS_t* bASList = &vk_d.bottomASList[vk_d.bottomASCount];
	//define AS geometry
	Com_Memset(&bASList->geometries, 0, sizeof(VkGeometryNV));
	bASList->geometries.sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
	bASList->geometries.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
	bASList->geometries.geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
	bASList->geometries.geometry.triangles.vertexData = vk_d.geometry.xyz_entity_static.buffer;
	bASList->geometries.geometry.triangles.vertexOffset = (*xyzOffset) * sizeof(VertexBuffer);
	bASList->geometries.geometry.triangles.vertexCount = tess.numVertexes;
	bASList->geometries.geometry.triangles.vertexStride = sizeof(VertexBuffer);
	bASList->geometries.geometry.triangles.indexData = vk_d.geometry.idx_entity_static.buffer;
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
	// write idx
	RB_UploadIDX(&vk_d.geometry.idx_entity_static, bASList->data.offsetIDX, 0);
	// write xyz and other vertex attribs
	RB_UploadXYZ(&vk_d.geometry.xyz_entity_static, bASList->data.offsetXYZ, -1);

	VkCommandBuffer commandBuffer = { 0 };
	VK_BeginSingleTimeCommands(&commandBuffer);
	VK_CreateBottomAS(commandBuffer, bASList, &vk_d.basBufferEntityStatic, &vk_d.basBufferEntityStaticOffset, RTX_BOTTOM_AS_FLAG);
	VK_EndSingleTimeCommands(&commandBuffer);

	vk_d.bottomASCount++;
	(*idxOffset) += tess.numIndexes;
	(*xyzOffset) += tess.numVertexes;
	if (bAS != NULL) (*bAS) = bASList;
}

//
//static qboolean RB_MaterialException(vkbottomAS_t* bAS) {
//	// -- lights --
//	if (strstr(tess.shader->name, "base_light") || strstr(tess.shader->name, "gothic_light")) { // all lamp textures
//		bAS->data.material = MATERIAL_KIND_REGULAR;
//		bAS->data.material |= MATERIAL_FLAG_LIGHT;
//		//RB_AddLightToLightList();
//	}
//	else
//	if (strstr(tess.shader->name, "flame")) { // all fire textures
//		bAS->data.material = MATERIAL_KIND_REGULAR;
//		bAS->data.material |= MATERIAL_FLAG_LIGHT;
//		//RB_AddLightToLightList();
//	}
//	//else
//	//if (strstr(tess.shader->name, "beam")  /* || (strstr(tess.shader->name, "lamp") && strstr(tess.shader->name, "flare"))*/ ) { // light rect and cones (beam == cones, lamp = squares)
//	//	bAS->data.material = MATERIAL_KIND_INVISIBLE;
//	//	bAS->data.material |= MATERIAL_FLAG_LIGHT;
//	//}
//	else
//	// -- glass --
//	if (strstr(tess.shader->name, "glass") || strstr(tess.shader->name, "jacobs") || strstr(tess.shader->name, "green_sphere") || strstr(tess.shader->name, "yellow_sphere") || strstr(tess.shader->name, "red_sphere")) { // glass (jacobs = console glass, green sphere = life)
//		bAS->data.material = MATERIAL_KIND_GLASS;
//		//bAS->data.material |= MATERIAL_FLAG_LIGHT;
//	} 
//	else
//	if (tess.shader->sort == SS_BLEND0) {
//		int x = 2;
//		//bAS->data.material == MATERIAL_KIND_GLASS;
//	}
//	//	else
//	//if (strstr(tess.shader->name, "gratelamp/gratelamp") && !strstr(tess.shader->name, "flare") && !strstr(tess.shader->name, "_b")) {
//	//	bAS->data.material == MATERIAL_KIND_INVISIBLE;
//	//	//bAS->data.material |= MATERIAL_FLAG_LIGHT;
//	//}
//	//else
//	//if (backEnd.currentEntity->e.reType == (RT_SPRITE) &&
//	//	(strstr(tess.shader->name, "rocketExplosion") || strstr(tess.shader->name, "plasma1") || strstr(tess.shader->name, "grenadeExplosion") || strstr(tess.shader->name, "bfgExplosion"))) {
//	//	//bAS->data.material |= MATERIAL_KIND_BULLET;
//	//	bAS->data.material |= MATERIAL_FLAG_NEEDSCOLOR;
//	//}
//	//else if (backEnd.currentEntity->e.reType == RT_RAIL_CORE || backEnd.currentEntity->e.reType == RT_RAIL_RINGS || backEnd.currentEntity->e.reType == RT_LIGHTNING) {
//	//	//bAS->data.material |= MATERIAL_KIND_BULLET;
//	//	bAS->data.material |= MATERIAL_FLAG_NEEDSCOLOR;
//	//}
//	//else if (strstr(tess.shader->name, "railExplosion")) {
//	//	bAS->data.material |= MATERIAL_FLAG_NEEDSCOLOR;
//	//	bAS->data.material |= MATERIAL_FLAG_TRANSPARENT;
//	//}
//	//else if (tess.shader->sort == SS_DECAL) {
//	//	//bAS->data.material |= MATERIAL_FLAG_NEEDSCOLOR;
//	//	bAS->data.material |= MATERIAL_FLAG_BULLET_MARK;
//	//}else if (strstr(tess.shader->name, "hologirl")) {
//	//	bAS->data.material = MATERIAL_FLAG_SEE_THROUGH;
//	//	//bAS->data.material |= MATERIAL_FLAG_NEEDSCOLOR;
//	//}
//	//else if (strstr(tess.shader->name, "gratelamp/gratelamp") && !strstr(tess.shader->name, "flare") && !strstr(tess.shader->name, "_b")) {
//	//	bAS->data.material = MATERIAL_FLAG_SEE_THROUGH;
//	//}
//	//else if (strstr(tess.shader->name, "flare") || strstr(tess.shader->name, "textures/sfx/beam")) {
//	//	bAS->data.material = MATERIAL_FLAG_LIGHT;
//	//}
//	//else if (strstr(tess.shader->name, "railgun")) bAS->data.material |= MATERIAL_FLAG_NEEDSCOLOR;
//	else return qfalse;
//	return qtrue;
//}



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
	//if (tess.shader->rtstages[0]->bundle[0].numImageAnimations > 1) {
	//	indexAnim = (int)(tess.shaderTime * tess.shader->rtstages[0]->bundle[0].imageAnimationSpeed * FUNCTABLE_SIZE);
	//	indexAnim >>= FUNCTABLE_SIZE2;
	//	if (indexAnim < 0) {
	//		indexAnim = 0;	// may happen with shader time offsets
	//	}
	//	indexAnim %= tess.shader->stages[0]->bundle[0].numImageAnimations;	
	//}
	/*if (bAS->data.texIdx != (uint32_t)tess.shader->rtstages[0]->bundle[0].image[indexAnim]->index) {
		bAS->data.texIdx = (uint32_t)tess.shader->rtstages[0]->bundle[0].image[indexAnim]->index;
		tess.shader->rtstages[0]->bundle[0].image[indexAnim]->frameUsed = tr.frameCount;
	}*/

	//bAS->data.blendfunc = (uint32_t)(tess.shader->rtstages[0]->stateBits);

	//// set material
	//if (!RB_MaterialException(bAS)) {

	//	bAS->data.material &= 0xfffffff0;
	//	switch (tess.shader->contentFlags & 0x0000007f) {
	//	case CONTENTS_SOLID: bAS->data.material |= MATERIAL_KIND_REGULAR; break;
	//	case CONTENTS_LAVA: bAS->data.material |= MATERIAL_KIND_LAVA; break;
	//	case CONTENTS_SLIME: bAS->data.material |= MATERIAL_KIND_SLIME; break;
	//	case CONTENTS_WATER: bAS->data.material |= MATERIAL_KIND_WATER; break;
	//	case CONTENTS_FOG: bAS->data.material |= MATERIAL_KIND_FOG; break;
	//	default: bAS->data.material |= MATERIAL_KIND_INVALID; break;
	//	}
	//CONTENTS_TRANSLUCENT
	//	if (tess.shader->sort == SS_PORTAL && strstr(tess.shader->name, "mirror") != NULL) bAS->data.material |= MATERIAL_FLAG_MIRROR;
	//	//else if (tess.shader->sort == SS_PORTAL && strstr(tess.shader->name, "mirror") == NULL) bAS->data.material |= MATERIAL_FLAG_PORTAL;
	//	////if (tess.shader->sort <= SS_OPAQUE) bAS->data.material |= MATERIAL_FLAG_OPAQUE;
	//	//if (tess.shader->sort == SS_BLEND0 || tess.shader->sort == SS_BLEND1) bAS->data.material |= MATERIAL_FLAG_TRANSPARENT;
	//	//if ((tess.shader->contentFlags & CONTENTS_TRANSLUCENT) == CONTENTS_TRANSLUCENT) {
	//	//	bAS->data.material |= MATERIAL_FLAG_SEE_THROUGH;
	//	//}
	//	//if (tess.shader->sort <= SS_OPAQUE && tess.shader->contentFlags != CONTENTS_TRANSLUCENT /*!strstr(tess.shader->rtstages[0]->bundle->image[0]->imgName, "proto_grate4.tga")*/) bAS->data.material |= MATERIAL_FLAG_OPAQUE;
	//}

	//if ((backEnd.currentEntity->e.renderfx & RF_FIRST_PERSON)) bAS->data.material = MATERIAL_FLAG_PLAYER_OR_WEAPON;
	VK_UploadBufferDataOffset(&vk_d.instanceDataBuffer[vk.swapchain.currentImage], vk_d.bottomASTraceListCount * sizeof(ASInstanceData), sizeof(ASInstanceData), (void*)&bAS->data);
}

void RB_UpdateInstanceBuffer(vkbottomAS_t* bAS) {
	bAS->geometryInstance.instanceCustomIndex = 0;
	// set visibility for first and third person (eye and mirror)
	if ((backEnd.currentEntity->e.renderfx & RF_THIRD_PERSON)) bAS->geometryInstance.mask = RAY_MIRROR_OPAQUE_VISIBLE;
	else if ((backEnd.currentEntity->e.renderfx & RF_FIRST_PERSON)) bAS->geometryInstance.mask = RAY_FIRST_PERSON_OPAQUE_VISIBLE;
	else bAS->geometryInstance.mask = RAY_FIRST_PERSON_MIRROR_OPAQUE_VISIBLE;

	

	/*if (bAS->data.material & MATERIAL_FLAG_PARTICLE || tess.shader->sort == SS_BLEND0 || tess.shader->sort == SS_BLEND1 || tess.shader->sort == SS_DECAL) {
		bAS->geometryInstance.instanceOffset = 1;
		if ((backEnd.currentEntity->e.renderfx & RF_THIRD_PERSON)) bAS->geometryInstance.mask = RAY_MIRROR_PARTICLE_VISIBLE;
		else if ((backEnd.currentEntity->e.renderfx & RF_FIRST_PERSON)) bAS->geometryInstance.mask = RAY_FIRST_PERSON_PARTICLE_VISIBLE;
		else bAS->geometryInstance.mask = RAY_FIRST_PERSON_MIRROR_PARTICLE_VISIBLE;
	}
	else {
		bAS->geometryInstance.instanceOffset = 0;
	}*/

	if (tess.shader->sort <= SS_OPAQUE) {
		bAS->geometryInstance.flags = VK_GEOMETRY_INSTANCE_FORCE_OPAQUE_BIT_NV;
	}
	else {
		bAS->geometryInstance.flags = VK_GEOMETRY_INSTANCE_FORCE_NO_OPAQUE_BIT_NV;
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
	bAS->geometryInstance.accelerationStructureHandle = bAS->handle;

	VK_UploadBufferDataOffset(&vk_d.instanceBuffer[vk.swapchain.currentImage], vk_d.bottomASTraceListCount * sizeof(VkGeometryInstanceNV), sizeof(VkGeometryInstanceNV), (void*)&bAS->geometryInstance);
}

static uint32_t RB_FindPrevInstance(drawSurf_t* drawSurfs, int numDrawSurfs, vec3_t prevOrigin) {
	for (int i = 0; i < vk_d.prevEntityCount; i++)
	{
		//if (!memcmp(vk_d.entityOriginsPrev[i].origin, prevOrigin, sizeof(vec3_t))) return vk_d.entityOriginsPrev[i].instance;

		if (Distance(vk_d.entityOriginsPrev[i].origin, prevOrigin) < 3) {
			return vk_d.entityOriginsPrev[i].instance;
		}
	}
	return 0;
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
	
	// add static world
	VK_UploadBufferDataOffset(&vk_d.instanceDataBuffer[vk.swapchain.currentImage], vk_d.bottomASTraceListCount * sizeof(ASInstanceData), sizeof(ASInstanceData), (void*)&vk_d.bottomASWorldStatic.data);
	VK_UploadBufferDataOffset(&vk_d.instanceBuffer[vk.swapchain.currentImage], vk_d.bottomASTraceListCount * sizeof(VkGeometryInstanceNV), sizeof(VkGeometryInstanceNV), (void*)&vk_d.bottomASWorldStatic.geometryInstance);
	Com_Memcpy(&vk_d.bottomASTraceList[vk_d.bottomASTraceListCount], &vk_d.bottomASWorldStatic, sizeof(vkbottomAS_t));
	vk_d.bottomASTraceListCount++;
	// add static world trans
	VK_UploadBufferDataOffset(&vk_d.instanceDataBuffer[vk.swapchain.currentImage], vk_d.bottomASTraceListCount * sizeof(ASInstanceData), sizeof(ASInstanceData), (void*)&vk_d.bottomASWorldStaticTrans.data);
	VK_UploadBufferDataOffset(&vk_d.instanceBuffer[vk.swapchain.currentImage], vk_d.bottomASTraceListCount * sizeof(VkGeometryInstanceNV), sizeof(VkGeometryInstanceNV), (void*)&vk_d.bottomASWorldStaticTrans.geometryInstance);
	memcpy(&vk_d.bottomASTraceList[vk_d.bottomASTraceListCount], &vk_d.bottomASWorldStaticTrans, sizeof(vkbottomAS_t));
	vk_d.bottomASTraceListCount++;
	// add world with dynamic data
	VK_UploadBufferDataOffset(&vk_d.instanceDataBuffer[vk.swapchain.currentImage], vk_d.bottomASTraceListCount * sizeof(ASInstanceData), sizeof(ASInstanceData), (void*)&vk_d.bottomASWorldDynamicData.data);
	VK_UploadBufferDataOffset(&vk_d.instanceBuffer[vk.swapchain.currentImage], vk_d.bottomASTraceListCount * sizeof(VkGeometryInstanceNV), sizeof(VkGeometryInstanceNV), (void*)&vk_d.bottomASWorldDynamicData.geometryInstance);
	Com_Memcpy(&vk_d.bottomASTraceList[vk_d.bottomASTraceListCount], &vk_d.bottomASWorldDynamicData, sizeof(vkbottomAS_t));
	vk_d.bottomASTraceListCount++;
	// add world with dynamic data trans
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
	VK_UploadBufferDataOffset(&vk_d.instanceDataBuffer[vk.swapchain.currentImage], vk_d.bottomASTraceListCount * sizeof(ASInstanceData), sizeof(ASInstanceData), (void*)&vk_d.bottomASWorldDynamicAS[vk.swapchain.currentImage].data);
	VK_UploadBufferDataOffset(&vk_d.instanceBuffer[vk.swapchain.currentImage], vk_d.bottomASTraceListCount * sizeof(VkGeometryInstanceNV), sizeof(VkGeometryInstanceNV), (void*)&vk_d.bottomASWorldDynamicAS[vk.swapchain.currentImage].geometryInstance);
	VK_UpdateBottomAS(vk.swapchain.CurrentCommandBuffer(), &vk_d.bottomASWorldDynamicAS[vk.swapchain.currentImage],
		&vk_d.bottomASWorldDynamicAS[vk.swapchain.currentImage],
		&vk_d.basBufferWorldDynamicAS, NULL, VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV);
	backEnd.refdef.floatTime = originalTime;
	Com_Memcpy(&vk_d.bottomASTraceList[vk_d.bottomASTraceListCount], &vk_d.bottomASWorldDynamicAS[vk.swapchain.currentImage], sizeof(vkbottomAS_t));
	vk_d.bottomASTraceListCount++;
	// trans
	VK_UploadBufferDataOffset(&vk_d.instanceDataBuffer[vk.swapchain.currentImage], vk_d.bottomASTraceListCount * sizeof(ASInstanceData), sizeof(ASInstanceData), (void*)&vk_d.bottomASWorldDynamicASTrans[vk.swapchain.currentImage].data);
	VK_UploadBufferDataOffset(&vk_d.instanceBuffer[vk.swapchain.currentImage], vk_d.bottomASTraceListCount * sizeof(VkGeometryInstanceNV), sizeof(VkGeometryInstanceNV), (void*)&vk_d.bottomASWorldDynamicASTrans[vk.swapchain.currentImage].geometryInstance);
	VK_UpdateBottomAS(vk.swapchain.CurrentCommandBuffer(), &vk_d.bottomASWorldDynamicASTrans[vk.swapchain.currentImage],
		&vk_d.bottomASWorldDynamicASTrans[vk.swapchain.currentImage],
		&vk_d.basBufferWorldDynamicAS, NULL, VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV);
	backEnd.refdef.floatTime = originalTime;
	Com_Memcpy(&vk_d.bottomASTraceList[vk_d.bottomASTraceListCount], &vk_d.bottomASWorldDynamicASTrans[vk.swapchain.currentImage], sizeof(vkbottomAS_t));
	vk_d.bottomASTraceListCount++;

	// lights
	//VK_UploadBufferDataOffset(&vk_d.uboLightList[vk.swapchain.currentImage], 0, sizeof(LightList_s), (void*)&vk_d.lightList);
	//VK_UploadBufferDataOffset(&vk_d.uboLightList[vk.swapchain.currentImage], RTX_MAX_LIGHTS * sizeof(vec4_t), 1 * sizeof(uint32_t), (void*)&vk_d.lightCount);

	for (i = 0, drawSurf = drawSurfs; i < numDrawSurfs; i++, drawSurf++) {

		R_DecomposeSort(drawSurf->sort, &entityNum, &shader, &fogNum, &dlighted);
		// skip stuff
		if (strstr(shader->name, "models/mapobjects/console/under") || strstr(shader->name, "textures/sfx/beam") || strstr(shader->name, "models/mapobjects/lamps/flare03")
			|| strstr(shader->name, "Shadow") || shader->isSky ||
			(shader->contentFlags & CONTENTS_TRANSLUCENT) == CONTENTS_TRANSLUCENT || shader->sort > SS_OPAQUE) {
			if(!strstr(shader->name, "green_sphere") && !strstr(shader->name, "yellow_sphere") && !strstr(shader->name, "red_sphere") && !strstr(shader->name, "energy_grn1")
				&& !strstr(shader->name, "teleportEffect") && !strstr(shader->name, "bloodExplosion")) continue;
		}

		if (strstr(shader->name, "green_sphere") || strstr(shader->name, "yellow_sphere") || strstr(shader->name, "red_sphere") || strstr(shader->name, "plasma_glass")) {
			shader->rtstages[0]->active = qfalse;
		}
	
		if (strstr(shader->name, "bloodExplosion")) {// bloodExplotion
			int i = 2;
			//rb_surfaceTable[*drawSurf->surface](drawSurf->surface);
			continue;
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
			if (!drawSurf->bAS->isWorldSurface) {
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
			//continue;
			if (RB_ASDataDynamic(tess.shader) && !RB_ASDynamic(tess.shader)) {
				//continue;
				drawSurf->bAS->data.world = BAS_ENTITY_DYNAMIC;

				rb_surfaceTable[*drawSurf->surface](drawSurf->surface);

				drawSurf->bAS->data.offsetIDX = vk_d.geometry.idx_entity_dynamic_offset;
				drawSurf->bAS->data.offsetXYZ = vk_d.geometry.xyz_entity_dynamic_offset;
				RB_UploadIDX(&vk_d.geometry.idx_entity_dynamic[vk.swapchain.currentImage], drawSurf->bAS->data.offsetIDX, 0);
				RB_UploadXYZ(&vk_d.geometry.xyz_entity_dynamic[vk.swapchain.currentImage], drawSurf->bAS->data.offsetXYZ, cluster);

				vk_d.geometry.idx_entity_dynamic_offset += tess.numIndexes;
				vk_d.geometry.xyz_entity_dynamic_offset += tess.numVertexes;

				Com_Memcpy(&drawSurf->bAS->geometryInstance.transform, &tM, sizeof(float[12]));
				Com_Memcpy(&drawSurf->bAS->data.modelmat, &tM, sizeof(float[12]));
				RB_UpdateInstanceDataBuffer(drawSurf->bAS);
				RB_UpdateInstanceBuffer(drawSurf->bAS);
				// add bottom to trace as list
				Com_Memcpy(&vk_d.bottomASTraceList[vk_d.bottomASTraceListCount], drawSurf->bAS, sizeof(vkbottomAS_t));
				vk_d.bottomASTraceListCount++;
			}
			else if (RB_ASDynamic(tess.shader)) {
				//continue;
				// ?leak only in debug build?
				vkbottomAS_t *newAS = &vk_d.bottomASDynamicList[vk.swapchain.currentImage][vk_d.bottomASDynamicCount[vk.swapchain.currentImage]];
				vk_d.bottomASDynamicCount[vk.swapchain.currentImage]++;
				Com_Memcpy(newAS, drawSurf->bAS, sizeof(vkbottomAS_t));
				newAS->data.world = BAS_ENTITY_DYNAMIC;

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
				RB_UpdateInstanceDataBuffer(newAS);
				RB_UpdateInstanceBuffer(newAS);
				// add bottom to trace as list
				Com_Memcpy(&vk_d.bottomASTraceList[vk_d.bottomASTraceListCount], newAS, sizeof(vkbottomAS_t));
				vk_d.bottomASTraceListCount++;
			}
			else {
				drawSurf->bAS->data.world = BAS_ENTITY_STATIC;
				drawSurf->bAS->data.cluster = cluster;

				if ((backEnd.currentEntity->e.renderfx & RF_FIRST_PERSON)) {
					drawSurf->bAS->data.isPlayer = qtrue;
				}
				else drawSurf->bAS->data.isPlayer = qfalse;

				/*Com_Memcpy(&vk_d.entityOriginsCurrent[vk_d.currentEntityCount].origin, &backEnd.currentEntity->e.origin, sizeof(float[3]));
				vk_d.entityOriginsCurrent[vk_d.currentEntityCount].instance = vk_d.bottomASTraceListCount;
				vk_d.currentEntityCount++;
				drawSurf->bAS->data.prevInstance = RB_FindPrevInstance(drawSurfs, numDrawSurfs, backEnd.currentEntity->e.origin);*/
				//drawSurf->bAS->data.currentInstance = vk_d.bottomASTraceListCount;
				Com_Memcpy(&drawSurf->bAS->geometryInstance.transform, &tM, sizeof(float[12]));	
				Com_Memcpy(&drawSurf->bAS->data.modelmat, &tM, sizeof(float[12]));
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
		//	int x = 2;
		//																			//RB_CreateBottomAS(&drawSurf->bAS, dynamic);
		//	/*drawSurf->bAS = &vk_d.bottomASList[0];
		//	dynamic = qtrue;
		//	forceUpdate = qtrue;
		//	drawSurf->bAS->data.material |= (MATERIAL_FLAG_NEEDSCOLOR | MATERIAL_FLAG_PARTICLE);*/
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
		
		
		/*if (drawSurf->bAS->as_for_each_swapchain_image) {
			Com_Memcpy(&drawSurf->bAS[vk.swapchain.currentImage].geometryInstance.transform, &tM, sizeof(float[12]));
			RB_AddBottomAS(&drawSurf->bAS[vk.swapchain.currentImage], dynamic, forceUpdate);
		}
		else {
			Com_Memcpy(&drawSurf->bAS->geometryInstance.transform, &tM, sizeof(float[12]));
			RB_AddBottomAS(drawSurf->bAS, dynamic, forceUpdate);
		}*/
	}
	backEnd.refdef.floatTime = originalTime;

	/*Com_Memcpy(&vk_d.entityOriginsPrev, &vk_d.entityOriginsCurrent, sizeof(vk_d.entityOriginsCurrent));
	vk_d.prevEntityCount = vk_d.currentEntityCount;
	vk_d.currentEntityCount = 0;*/

	VK_DestroyTopAccelerationStructure(&vk_d.topAS[vk.swapchain.currentImage]);
	VK_MakeTopAS(vk.swapchain.CurrentCommandBuffer(), &vk_d.topAS[vk.swapchain.currentImage], &vk_d.topASBuffer[vk.swapchain.currentImage], vk_d.bottomASTraceList, vk_d.bottomASTraceListCount, vk_d.instanceBuffer[vk.swapchain.currentImage], RTX_TOP_AS_FLAG);
	//VK_UpdateTopAS(vk.swapchain.CurrentCommandBuffer(), &vk_d.topAS[vk.swapchain.currentImage], &vk_d.topAS[vk.swapchain.currentImage], &vk_d.topASBuffer[vk.swapchain.currentImage], vk_d.bottomASTraceList, vk_d.bottomASTraceListCount, vk_d.instanceBuffer[vk.swapchain.currentImage], VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV);

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
	ubo->numRandomIL = pt_numRandomIL->integer;
	ubo->numBounces = pt_numBounces->integer;
	ubo->randSampleLight = rt_softshadows->integer;

	ubo->aperture = rt_aperture->value;
	ubo->focalLength = rt_focalLength->value;
	ubo->dof = rt_dof->integer;
	ubo->randomPixelOffset = rt_antialiasing->integer;
	ubo->accumulate = rt_accumulate->integer;

	if (rt_accumulate->integer) {
		Cvar_Set("cl_paused", "1");
		Cvar_Set("sv_paused", "1");
		ubo->numSamples = vk_d.uboGlobal[(vk.swapchain.currentImage + (vk.swapchain.imageCount - 1)) % vk.swapchain.imageCount].numSamples + 1;
	}
	else {
		Cvar_Set("cl_paused", "0");
		Cvar_Set("sv_paused", "0");
		ubo->numSamples = 1;
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


	//VK_BindComputePipeline(&vk_d.accelerationStructures.rngPipeline);
	//VK_BindCompute2DescriptorSets(&vk_d.accelerationStructures.rngPipeline, &vk_d.computeDescriptor[vk.swapchain.currentImage], &vk_d.imageDescriptor);
	//VK_Dispatch(vk_d.lightList.numLights, 1, 1);

	VK_SetAccelerationStructure(&vk_d.rtxDescriptor[vk.swapchain.currentImage], BINDING_OFFSET_AS, VK_SHADER_STAGE_RAYGEN_BIT_NV, &vk_d.topAS[vk.swapchain.currentImage].accelerationStructure);
	VK_UpdateDescriptorSet(&vk_d.rtxDescriptor[vk.swapchain.currentImage]);
	
	//VK_BindRayTracingPipeline(&vk_d.accelerationStructures.pipeline);
	//VK_Bind2RayTracingDescriptorSets(&vk_d.accelerationStructures.pipeline, &vk_d.rtxDescriptor[vk.swapchain.currentImage], &vk_d.imageDescriptor);
	//VK_TraceRays(&vk_d.accelerationStructures.pipeline);

	/*VkImageMemoryBarrier barrier = { 0 };
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
	barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_MEMORY_READ_BIT;
	barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
	int prevIndex = (vk.swapchain.currentImage + (vk.swapchain.imageCount - 1)) % vk.swapchain.imageCount;
	barrier.image = vk_d.gBuffer[vk.swapchain.currentImage].position.handle;
	vkCmdPipelineBarrier(vk.swapchain.CurrentCommandBuffer(),
		VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV,
		VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV,
		0, 0, NULL, 0, NULL,
		1, &barrier);*/

	
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

	/*memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	memoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
	memoryBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	vkCmdPipelineBarrier(vk.swapchain.CurrentCommandBuffer(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, 0, 1, &memoryBarrier, 0, 0, 0, 0);*/



	vk_d.portalInView = qfalse;
	vk_d.mirrorInView = qfalse;


	//VK_UnmapBuffer(&vk_d.uboBuffer[vk.swapchain.currentImage]);
	//VK_UnmapBuffer(&vk_d.uboLightList[vk.swapchain.currentImage]);
	//VK_UnmapBuffer(&vk_d.instanceDataBuffer[vk.swapchain.currentImage]);
	//VK_UnmapBuffer(&vk_d.geometry.idx_entity_static);
	//VK_UnmapBuffer(&vk_d.geometry.xyz_entity_static);
	//VK_UnmapBuffer(&vk_d.geometry.xyz_dynamic[vk.swapchain.currentImage]);
	//VK_UnmapBuffer(&vk_d.geometry.idx_dynamic[vk.swapchain.currentImage]);
	
	// draw rt results to swap chain
	VK_BeginRenderClear();
	//VK_DrawFullscreenRect(&vk_d.accelerationStructures.resultImage[vk.swapchain.currentImage]);

	// create descriptor
	vkimage_t* drawImage = &vk_d.accelerationStructures.resultImage[vk.swapchain.currentImage];// &vk_d.gBuffer[vk.swapchain.currentImage].albedo;
	//vkimage_t* drawImage = &vk_d.gBuffer[vk.swapchain.currentImage].reflection;
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
		double build, prim, refref, direct;
		VK_TimeBetweenMarkers(&build, PROFILER_BUILD_AS_BEGIN, PROFILER_BUILD_AS_END);
		VK_TimeBetweenMarkers(&prim, PROFILER_PRIMARY_RAYS_BEGIN, PROFILER_PRIMARY_RAYS_END);
		VK_TimeBetweenMarkers(&refref, PROFILER_REFLECTION_REFRACTION_BEGIN, PROFILER_REFLECTION_REFRACTION_END);
		VK_TimeBetweenMarkers(&direct, PROFILER_DIRECT_ILLUMINATION_BEGIN, PROFILER_DIRECT_ILLUMINATION_END);
		ri.Printf(PRINT_ALL, "Build/Update AS           %f ms\n", build);
		ri.Printf(PRINT_ALL, "Primary Ray Stage         %f ms\n", prim);
		ri.Printf(PRINT_ALL, "Reflect/Refract Stage     %f ms\n", refref);
		ri.Printf(PRINT_ALL, "Direct Illumination Stage %f ms\n", direct);
		ri.Printf(PRINT_ALL, "Accumulated Images        %d\n", vk_d.uboGlobal[vk.swapchain.currentImage].numSamples);
		//rt_printPerformanceStatistic->integer = 0;
		ri.Cvar_Set("rt_printPerfStats", 0);
	}

	//ri.Printf(PRINT_ALL, "Prim %f ms\n", (prim));
	//ri.SCR_DrawBigString(400, 400, "aasdfasd", 1.0);
	//ri.CL_ConsolePrint("asdf");

	//qhandle_t	charSetShader = RE_RegisterShader("gfx/2d/bigchars");
	//RE_StretchPic(400, 400, 100, 100,
	//	0, 0,
	//	1, 1,
	//	charSetShader);
	//VK_DrawFullscreenRect(&vk_d.accelerationStructures.resultFramebuffer.image);
}

