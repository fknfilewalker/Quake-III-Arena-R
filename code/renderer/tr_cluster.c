#include "tr_local.h"

qboolean RB_ClusterVisIdent(byte* aVis, byte* bVis) {
	if (!memcmp(aVis, bVis, vk_d.clusterBytes * sizeof(byte))) return qtrue;
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
		if (cluster == -1) cluster = R_FindClusterForPos(tess.xyz[i]);
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

// another try for the pvs
//int BSP_PointLeaf(vec3_t p)
//{
//	mnode_t* node;
//	float		d;
//	cplane_t* plane;
//
//	node = s_worldData.nodes;
//
//	while (node->plane) {
//		plane = node->plane;
//		d = DotProduct(p, plane->normal) - plane->dist;
//		if (d < 0)
//			node = node->children[1];
//		else
//			node = node->children[0];
//	}
//
//	return node->cluster;
//}

#define Q_IsBitSet(data, bit)   (((data)[(bit) >> 3] & (1 << ((bit) & 7))) != 0)
#define Q_SetBit(data, bit)     ((data)[(bit) >> 3] |= (1 << ((bit) & 7)))
#define Q_ClearBit(data, bit)   ((data)[(bit) >> 3] &= ~(1 << ((bit) & 7)))

#define FOREACH_BIT_BEGIN(SET,ROWSIZE,VAR) \
	for (int _byte_idx = 0; _byte_idx < ROWSIZE; _byte_idx++) { \
	if (SET[_byte_idx]) { \
		for (int _bit_idx = 0; _bit_idx < 8; _bit_idx++) { \
			if (SET[_byte_idx] & (1 << _bit_idx)) { \
				int VAR = (_byte_idx << 3) | _bit_idx;

#define FOREACH_BIT_END  } } } }
static void merge_pvs_rows(world_t* world, char* src, char* dst)
{
	for (int i = 0; i < world->clusterBytes; i++)
	{
		dst[i] |= src[i];
	}
}
void build_pvs2(world_t* world)
{
	size_t matrix_size = world->clusterBytes * world->numClusters;

	world->vis2 = Z_Malloc(matrix_size);

	for (int cluster = 0; cluster < world->numClusters; cluster++)
	{
		char* pvs = world->vis + cluster * world->clusterBytes;
		char* dest_pvs = world->vis2 + cluster * world->clusterBytes;
		memcpy(dest_pvs, pvs, world->clusterBytes);

		FOREACH_BIT_BEGIN(pvs, world->clusterBytes, vis_cluster)
			char* pvs2 = world->vis + vis_cluster * world->clusterBytes;
		merge_pvs_rows(world, pvs2, dest_pvs);
		FOREACH_BIT_END
	}

}

// Computes a point at a small distance above the center of the triangle.
// Returns qfalse if the triangle is degenerate, qtrue otherwise.
qboolean
get_triangle_norm(const float* positions, float* normal)
{
	const float* v0 = positions + 0;
	const float* v1 = positions + 3;
	const float* v2 = positions + 6;


	// Compute the normal

	vec3_t e1, e2;
	VectorSubtract(v1, v0, e1);
	VectorSubtract(v2, v0, e2);
	CrossProduct(e1, e2, normal);
	VectorNormalize(normal);
}
// Computes a point at a small distance above the center of the triangle.
// Returns qfalse if the triangle is degenerate, qtrue otherwise.
qboolean
get_triangle_off_center(const float* positions, float* center, float* anti_center)
{
	const float* v0 = positions + 0;
	const float* v1 = positions + 3;
	const float* v2 = positions + 6;

	// Compute the triangle center

	VectorCopy(v0, center);
	VectorAdd(center, v1, center);
	VectorAdd(center, v2, center);
	VectorScale(center, 1.f / 3.f, center);

	// Compute the normal

	vec3_t e1, e2, normal;
	VectorSubtract(v1, v0, e1);
	VectorSubtract(v2, v0, e2);
	CrossProduct(e1, e2, normal);
	float length = VectorNormalize(normal);

	// Offset the center by one normal to make sure that the point is
	// inside a BSP leaf and not on a boundary plane.

	VectorAdd(center, normal, center);

	if (anti_center)
	{
		VectorMA(center, -2.f, normal, anti_center);
	}

	return (length > 0.f);
}