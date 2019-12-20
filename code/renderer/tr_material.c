#include "tr_local.h"

static qboolean RB_NeedsColor() {
	for (int i = 0; i < MAX_SHADER_STAGES; i++) {
		if (tess.shader->stages[i] != NULL && tess.shader->stages[i]->active) {
			if (tess.shader->stages[i]->rgbGen == CGEN_WAVEFORM) {
				return qtrue;
			}
		}
	}
	return qfalse;
}

qboolean RB_StageNeedsColor(int stage) {
	if (tess.shader->stages[stage] != NULL && tess.shader->stages[stage]->active) {
		if (tess.shader->stages[stage]->rgbGen == CGEN_WAVEFORM) {
			return qtrue;
		}
	}
	return qfalse;
	//tess.shader->stages[0] != NULL ? ((tess.shader->stages[0]->bundle[0].tcGen != TCGEN_BAD) && tess.shader->stages[0]->bundle[0].numTexMods > 0) : qfalse;
}

uint32_t RB_GetMaterial() {
	uint32_t material = 0;
	if (strstr(tess.shader->name, "base_light") || strstr(tess.shader->name, "gothic_light") || strstr(tess.shader->name, "eye")) { // all lamp textures
		material = MATERIAL_KIND_REGULAR;
		material |= MATERIAL_FLAG_LIGHT;
	}
	else if (strstr(tess.shader->name, "flame")) { // all fire textures
		material = MATERIAL_KIND_REGULAR;
		material |= MATERIAL_FLAG_LIGHT;
	}

	if (tess.shader->sort == SS_PORTAL && strstr(tess.shader->name, "mirror") != NULL) material |= MATERIAL_FLAG_MIRROR;
	if (RB_NeedsColor()) material |= MATERIAL_FLAG_NEEDSCOLOR;
	return material;
}