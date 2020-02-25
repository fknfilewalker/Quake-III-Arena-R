#include "tr_local.h"

qboolean RB_IsLight(shader_t
	* shader) {
	if (tess.numIndexes > 6) {
		return qfalse;
	}
	if (strstr(shader->name, "wsupprt1_12") || strstr(shader->name, "scrolllight") || strstr(shader->name, "runway")) return qfalse;

	if (strstr(shader->name, "base_light") || strstr(shader->name, "gothic_light") || strstr(shader->name, "lamplight_y") /*|| strstr(tess.shader->name, "eye")*/) { // all lamp textures
		return qtrue;
	}
	
	if (strstr(shader->name, "flame")) {
		return qtrue;
	}
	return qfalse;
}

static qboolean RB_NeedsColor() {
	if (strstr(tess.shader->name, "textures/liquids/calm_poollight")) {		int x = 2;
	}
	for (int i = 0; i < MAX_SHADER_STAGES; i++) {
		if (tess.shader->rtstages[i] != NULL && tess.shader->rtstages[i]->active) {
			if (tess.shader->rtstages[i]->rgbGen == CGEN_WAVEFORM) {
				return qtrue;
			}
		}
	}
	return qfalse;
}

qboolean RB_StageNeedsColor(int stage) {
	if (strstr(tess.shader->name, "textures/liquids/calm_poollight")) {
		int x = 2;
	}
	if (tess.shader->rtstages[stage] != NULL && tess.shader->rtstages[stage]->active) {
		if (tess.shader->rtstages[stage]->rgbGen == CGEN_WAVEFORM || tess.shader->rtstages[stage]->rgbGen == CGEN_CONST) {
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

	if (strstr(tess.shader->name, "glass") || strstr(tess.shader->name, "console/jacobs") || strstr(tess.shader->name, "kmlamp_white") || strstr(tess.shader->name, "slamp/slamp2") 
		|| strstr(tess.shader->name, "timlamp/timlamp") || strstr(tess.shader->name, "lamplight_y")) {
		material = MATERIAL_KIND_GLASS;
	}
	if (strstr(tess.shader->name, "textures/liquids/calm_poollight")) {
		material = MATERIAL_KIND_WATER;
	}
	if (strstr(tess.shader->name, "tesla")) {
		material = MATERIAL_KIND_WATER;
	}
	//+		tess.shader	0x0000027dd20c1da8 {name=0x0000027dd20c1da8 "textures/sfx/portal_sfx_ring" lightmapIndex=-3 index=118 ...}	shader_s *
	//+		name	0x0000020228d63a68 "textures/liquids/calm_poollight"	char[64]

	// consol/sphere2
	if (tess.shader->sort == SS_PORTAL && strstr(tess.shader->name, "mirror") != NULL) material |= MATERIAL_FLAG_MIRROR;
	else if ((strstr(tess.shader->name, "gratelamp/gratelamp") && !strstr(tess.shader->name, "gratelamp/gratelamp_b"))
		|| strstr(tess.shader->name, "gratelamp/gratetorch2b")
		|| strstr(tess.shader->name, "timlamp/timlamp")
		|| strstr(tess.shader->name, "proto_grate")
		|| strstr(tess.shader->name, "textures/sfx/portal_sfx_ring")
		|| strstr(tess.shader->name, "models/mapobjects/flag/banner_strgg")
		|| strstr(tess.shader->name, "skull/ribcage")) {
		material |= MATERIAL_FLAG_SEE_THROUGH;
	}
	else if (strstr(tess.shader->name, "flame") || strstr(tess.shader->name, "models/mapobjects/bitch/orb") || strstr(tess.shader->name, "console/sphere") || strstr(tess.shader->name, "console")
		|| strstr(tess.shader->name, "tesla") || strstr(tess.shader->name, "proto_zzz") || strstr(tess.shader->name, "cybergrate")
		|| strstr(tess.shader->name, "teleporter/energy") || strstr(tess.shader->name, "textures/liquids/calm_poollight")) {
		material |= MATERIAL_FLAG_SEE_THROUGH_ADD;
	}
	else if ((tess.shader->contentFlags & CONTENTS_TRANSLUCENT) == CONTENTS_TRANSLUCENT) {
		material |= MATERIAL_FLAG_SEE_THROUGH;
	}

	if ((backEnd.currentEntity->e.renderfx & RF_FIRST_PERSON)) {
		material |= MATERIAL_FLAG_PLAYER_OR_WEAPON;
	}
	return material;
}

uint32_t RB_GetNextTex(int stage) {
	int indexAnim = 0;
	if (tess.shader->rtstages[stage]->bundle[0].numImageAnimations > 1) {
		indexAnim = (int)(tess.shaderTime * tess.shader->rtstages[stage]->bundle[0].imageAnimationSpeed * FUNCTABLE_SIZE);
		indexAnim >>= FUNCTABLE_SIZE2;
		if (indexAnim < 0) {
			indexAnim = 0;	// may happen with shader time offsets
		}
		indexAnim %= tess.shader->rtstages[stage]->bundle[0].numImageAnimations;
	}
	return indexAnim;
}

uint32_t RB_GetNextTexEncoded(int stage) {
	if (tess.shader->rtstages[stage] != NULL && tess.shader->rtstages[stage]->active) {
		int indexAnim = RB_GetNextTex(stage);

		uint32_t blend = 0;
		uint32_t stateBits = tess.shader->rtstages[stage]->stateBits & (GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS);
		if ((stateBits & GLS_SRCBLEND_BITS) > GLS_SRCBLEND_ONE && (stateBits & GLS_DSTBLEND_BITS) > GLS_DSTBLEND_ONE) blend = TEX0_NORMAL_BLEND_MASK;
		if (stateBits == 19) blend = TEX0_MUL_BLEND_MASK;
		if (stateBits == 34) blend = TEX0_ADD_BLEND_MASK;
		if (stateBits == 101) blend = TEX0_NORMAL_BLEND_MASK;
		qboolean color = RB_StageNeedsColor(stage);

		uint32_t nextidx = (uint32_t)indexAnim;
		uint32_t idx = (uint32_t)tess.shader->rtstages[stage]->bundle[0].image[nextidx]->index;
		tess.shader->rtstages[stage]->bundle[0].image[nextidx]->frameUsed = tr.frameCount;
		return (idx) | (blend) | (color ? TEX0_COLOR_MASK : 0);
	}
	return TEX0_IDX_MASK;
}