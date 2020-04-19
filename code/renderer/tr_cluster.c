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