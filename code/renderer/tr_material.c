#include "tr_local.h"

qboolean RB_IsLight(shader_t
	* shader) {
	if (tess.numIndexes > 30) {
		return qfalse;
	}
	if (strstr(shader->name, "wsupprt1_12") || strstr(shader->name, "scrolllight") || strstr(shader->name, "runway")) return qfalse;

	if (strstr(shader->name, "base_light") || strstr(shader->name, "gothic_light") /*|| strstr(tess.shader->name, "eye")*/) { // all lamp textures
		return qtrue;
	}
	
	if (strstr(shader->name, "flame")) {
		return qfalse;
	}
	return qfalse;
}

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
}

uint32_t RB_GetMaterial() {
	uint32_t material = 0;
	material = MATERIAL_KIND_REGULAR;
	if (RB_IsLight(tess.shader)) {
		material |= MATERIAL_FLAG_LIGHT;
	}

	if (strstr(tess.shader->name, "glass")) material = MATERIAL_KIND_GLASS;

	if (tess.shader->sort == SS_PORTAL && strstr(tess.shader->name, "mirror") != NULL) material |= MATERIAL_FLAG_MIRROR;
	if ((tess.shader->contentFlags & CONTENTS_TRANSLUCENT) == CONTENTS_TRANSLUCENT) material |= MATERIAL_FLAG_SEE_THROUGH;

	if ((backEnd.currentEntity->e.renderfx & RF_FIRST_PERSON)) {
		material |= MATERIAL_FLAG_PLAYER_OR_WEAPON;
	}
	return material;
}

uint32_t RB_GetNextTex(int stage) {
	int indexAnim = 0;
	if (tess.shader->stages[stage]->bundle[0].numImageAnimations > 1) {
		indexAnim = (int)(tess.shaderTime * tess.shader->stages[stage]->bundle[0].imageAnimationSpeed * FUNCTABLE_SIZE);
		indexAnim >>= FUNCTABLE_SIZE2;
		if (indexAnim < 0) {
			indexAnim = 0;	// may happen with shader time offsets
		}
		indexAnim %= tess.shader->stages[stage]->bundle[0].numImageAnimations;
	}
	return indexAnim;
}

uint32_t RB_GetNextTexEncoded(int stage) {
	if (tess.shader->stages[stage] != NULL && tess.shader->stages[stage]->active) {
		int indexAnim = RB_GetNextTex(stage);

		qboolean blend = qfalse;
		uint32_t stateBits = tess.shader->stages[stage]->stateBits & (GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS);
		if ((stateBits & GLS_SRCBLEND_BITS) > GLS_SRCBLEND_ONE && (stateBits & GLS_DSTBLEND_BITS) > GLS_DSTBLEND_ONE) blend = qtrue;
		qboolean color = RB_StageNeedsColor(stage);

		uint32_t nextidx = (uint32_t)indexAnim;
		uint32_t idx = (uint32_t)tess.shader->stages[stage]->bundle[0].image[nextidx]->index;
		tess.shader->stages[stage]->bundle[0].image[nextidx]->frameUsed = tr.frameCount;
		return (idx) | (blend ? TEX0_BLEND_MASK : 0) | (color ? TEX0_COLOR_MASK : 0);
	}
	return TEX0_IDX_MASK;
}