#include "tr_local.h"

qboolean RB_IsLight(shader_t
	* shader) {
	//if (strstr(tess.shader->name, "lava")) return qtrue;
	if (tess.numIndexes > 12) {
		if(!strstr(shader->name, "proto_light_2k") && !strstr(shader->name, "gothic_light3_2K")) return qfalse;
	}
	if (strstr(shader->name, "wsupprt1_12") || strstr(shader->name, "scrolllight") || strstr(shader->name, "runway")) return qfalse;

	if (strstr(shader->name, "base_light") || strstr(shader->name, "gothic_light") || strstr(shader->name, "lamplight_y") /*|| strstr(tess.shader->name, "eye")*/) { // all lamp textures
		if (tess.numIndexes > 12) {
			//int a = 2;
		}
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
	if (strstr(tess.shader->name, "fog")) {
		//return qtrue;
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
	if (strstr(tess.shader->name, "fog")) {
		//return qtrue;
	}
	if (tess.shader->rtstages[stage] != NULL && tess.shader->rtstages[stage]->active) {
		if (tess.shader->rtstages[stage]->rgbGen == CGEN_WAVEFORM || tess.shader->rtstages[stage]->rgbGen == CGEN_CONST) {
			return qtrue;
		}
	}
	return qfalse;
}

qboolean RB_SkipObject(shader_t* shader) {
	if (strstr(shader->name, "glass") || strstr(shader->name, "console/jacobs") || strstr(shader->name, "kmlamp_white") || strstr(shader->name, "slamp/slamp2")
		|| strstr(shader->name, "timlamp/timlamp") || strstr(shader->name, "lamplight_y") || strstr(shader->name, "textures/liquids/calm_poollight")) {
		return qfalse;
	}

	//if(strstr(shader->name, "portal_sfx_ring")) return qtrue;

	if (strstr(shader->name, "models/mapobjects/console/under") || strstr(shader->name, "textures/sfx/beam") || strstr(shader->name, "models/mapobjects/lamps/flare03")
		|| strstr(shader->name, "Shadow") || shader->isSky
		|| shader->surfaceFlags == SURF_NODRAW || shader->surfaceFlags == SURF_SKIP
		|| shader->rtstages[0] == NULL || !shader->rtstages[0]->active
		|| strstr(shader->name, "slamp/slamp3")
		|| strstr(shader->name, "gratelamp_flare")) return qtrue;
	return qfalse;
}

qboolean RB_IsTransparent(shader_t* shader) {
	// skip certain objects that are transparent but should be handled like opaque objects
	if (strstr(shader->name, "glass") || strstr(shader->name, "console/jacobs") || strstr(shader->name, "kmlamp_white") || strstr(shader->name, "slamp/slamp2")
		|| strstr(shader->name, "lamplight_y") || strstr(shader->name, "textures/liquids/calm_poollight") || strstr(shader->name, "textures/liquids/clear_ripple1") 
		|| strstr(shader->name, "flag") || strstr(shader->name, "green_sphere") || strstr(shader->name, "yellow_sphere") || strstr(shader->name, "red_sphere")) {
		return qfalse;
	}

	if (strstr(shader->name, "health/mega1")) {
		return qfalse;
	}
	if (strstr(shader->name, "armor/energy_yel1")) {
		return qtrue;
	}
	if (strstr(shader->name, "f_plasma")) {
		return qtrue;

	}
	if (strstr(shader->name, "slashskate")) {
		return qtrue;
	}
	if (strstr(shader->name, "railCore")) {
		return qtrue;
	}
	if (strstr(shader->name, "f_machine")) {// bloodExplotion
		return qtrue;
	}
	if (strstr(shader->name, "f_railgun")) {// bloodExplotion
		return qtrue;
	}
	if (strstr(shader->name, "shotgun_laser")) {// bloodExplotion
		return qtrue;
	}
	if (strstr(shader->name, "plasma_glo")) {
		return qfalse;
	}
	if (strstr(shader->name, "viewBloodBlend")) {// bloodExplotion
		return qtrue;
	}
	if (strstr(shader->name, "railgun2")) {
		return qtrue;
	}
	if (strstr(shader->name, "f_rocket")) {
		return qtrue;
	}

	if (strstr(shader->name, "console/sphere2")) {
		return qfalse;
	}
	if (strstr(shader->name, "portal") && !strstr(shader->name, "portal_2")) {
		return qtrue;
	}
	// check if transparent
	//shader	0x000001fec843ece8 {name=0x000001fec843ece8 "textures/base_floor/proto_grate4" lightmapIndex=-3 index=...}	shader_s *

	if ((shader->contentFlags & CONTENTS_TRANSLUCENT) == CONTENTS_TRANSLUCENT && shader->sort == SS_OPAQUE) {
		return qfalse;
	}
	if ((shader->contentFlags & CONTENTS_TRANSLUCENT) == CONTENTS_TRANSLUCENT || shader->sort > SS_OPAQUE) {
		return qtrue;
	}
	return qfalse;
}

uint32_t RB_GetMaterial() {
	uint32_t material = 0;
	material = MATERIAL_KIND_REGULAR;
	if (RB_IsLight(tess.shader)) {
		material |= MATERIAL_FLAG_LIGHT;
	}

	/*if (strstr(tess.shader->name, "lava")) {
		material |= MATERIAL_FLAG_LIGHT;
	}*/
	//+		tess.shader	0x0000023576446ce8 {name=0x0000023576446ce8 "textures/base_floor/rusty_pentagrate" lightmapIndex=-3 ...}	shader_s *
	//proto_rustygrate gratetorch2b 0x00000206adcea548 {name=0x00000206adcea548 "models/mapobjects/gratelamp/gratetorch2b" lightmapIndex=...}
	if (strstr(tess.shader->name, "timlamp/timlamp") || strstr(tess.shader->name, "gratelamp/gratelamp") || strstr(tess.shader->name, "proto_grate")
		|| strstr(tess.shader->name, "base_floor/rusty_pentagrate") || strstr(tess.shader->name, "base_floor/proto_rustygrate") || strstr(tess.shader->name, "gratelamp/gratetorch2b") || (strstr(tess.shader->name, "base_floor") && strstr(tess.shader->name, "grate") ||
			strstr(tess.shader->name, "flag") || strstr(tess.shader->name, "protobanner")) ) {
		material = MATERIAL_FLAG_SEE_THROUGH;
	}

	if (strstr(tess.shader->name, "powerups/armor/energy_grn1")) {
		material = MATERIAL_FLAG_SEE_THROUGH_NO_ALPHA;
	}

	if (strstr(tess.shader->name, "portal")) {
		material = MATERIAL_FLAG_SEE_THROUGH_ADD;
	}
	
	if (strstr(tess.shader->name, "glass") || strstr(tess.shader->name, "console/jacobs") || strstr(tess.shader->name, "kmlamp_white") || strstr(tess.shader->name, "slamp/slamp2") 
		|| strstr(tess.shader->name, "lamplight_y") || strstr(tess.shader->name, "green_sphere") 
		|| strstr(tess.shader->name, "yellow_sphere") || strstr(tess.shader->name, "red_sphere") || strstr(tess.shader->name, "plasma_glass") 
		) {
		material = MATERIAL_KIND_GLASS;
	}
	if (strstr(tess.shader->name, "health/mega1")) {
		material = MATERIAL_KIND_GLASS;
	}

	if (strstr(tess.shader->name, "console/sphere2")) {
		material = MATERIAL_FLAG_SEE_THROUGH;
	}



	//+		tess.shader	0x0000027dd20c1da8 {name=0x0000027dd20c1da8 "textures/sfx/portal_sfx_ring" lightmapIndex=-3 index=118 ...}	shader_s *
	//+		name	0x0000020228d63a68 "textures/liquids/calm_poollight"	char[64]

	// consol/sphere2
	//!strstr(shader->name, "green_sphere") && !strstr(shader->name, "energy_grn1")
	if (tess.shader->sort == SS_PORTAL && strstr(tess.shader->name, "mirror") != NULL) material |= MATERIAL_FLAG_MIRROR;
	/*else if ((strstr(tess.shader->name, "gratelamp/gratelamp") && !strstr(tess.shader->name, "gratelamp/gratelamp_b"))
		|| strstr(tess.shader->name, "gratelamp/gratetorch2b")
		|| strstr(tess.shader->name, "timlamp/timlamp")
		|| strstr(tess.shader->name, "models/mapobjects/flag/banner_strgg")
		|| strstr(tess.shader->name, "proto_grate")
		|| strstr(tess.shader->name, "skull/ribcage")
		|| strstr(tess.shader->name, "models/mapobjects/flares/electric")
		|| strstr(tess.shader->name, "hologirl")
		) {
		material |= MATERIAL_FLAG_SEE_THROUGH;
	}*/
	//else if (strstr(tess.shader->name, "flame") || strstr(tess.shader->name, "models/mapobjects/bitch/orb") || strstr(tess.shader->name, "console/sphere") || strstr(tess.shader->name, "console")
	//	 || strstr(tess.shader->name, "proto_zzz") || strstr(tess.shader->name, "cybergrate")
	//	|| strstr(tess.shader->name, "teleporter/energy")
	//	|| strstr(tess.shader->name, "textures/sfx/portal_sfx_ring")
	//	|| strstr(tess.shader->name, "tesla")
	//		|| strstr(tess.shader->name, "flame1_hell")
	//	|| strstr(tess.shader->name, "energy_grn1")
	//	|| strstr(tess.shader->name, "teleportEffect")
	//	
	//	){
	//	material |= MATERIAL_FLAG_SEE_THROUGH_ADD;
	//}
	else if (strstr(tess.shader->name, "bitch")) {
		material = MATERIAL_FLAG_SEE_THROUGH;
		if(tess.shader->rtstages[1] != NULL )tess.shader->rtstages[1]->active = qfalse;
	}
	else if (strstr(tess.shader->name, "skel")) {
		material = MATERIAL_FLAG_SEE_THROUGH;
		if (tess.shader->rtstages[1] != NULL)tess.shader->rtstages[1]->active = qfalse;
	}
	else if (strstr(tess.shader->name, "bloodExplosion") || strstr(tess.shader->name, "base_trim/wires02")) {
		material  = MATERIAL_FLAG_SEE_THROUGH;
	}
	else if (strstr(tess.shader->name, "models/players/hunter/hunter_f")) {
		material |= MATERIAL_FLAG_SEE_THROUGH_ADD;
	}
	else if (tess.shader->sort == SS_BLEND0) {
		material |= MATERIAL_FLAG_SEE_THROUGH_ADD;
	}
	if (strstr(tess.shader->name, "bitch/orb")) {
		material = MATERIAL_FLAG_SEE_THROUGH_ADD;
	}
	if (strstr(tess.shader->name, "armor/energy_yel1")) {
		material = MATERIAL_FLAG_SEE_THROUGH_ADD;
	}

	if (strstr(tess.shader->name, "f_plasma")) {
		material = MATERIAL_FLAG_SEE_THROUGH_ADD;

	}
	if (strstr(tess.shader->name, "f_railgun")) {
		material = MATERIAL_FLAG_SEE_THROUGH_ADD;

	}
	if (strstr(tess.shader->name, "plasma_glo")) {
		material = MATERIAL_FLAG_SEE_THROUGH_ADD;
	}

	if (strstr(tess.shader->name, "railCore")) {
		material = MATERIAL_FLAG_SEE_THROUGH_ADD;
	}
	if (strstr(tess.shader->name, "railgun2")) {
		material = MATERIAL_FLAG_SEE_THROUGH_ADD;
	}
	if (strstr(tess.shader->name, "textures/liquids/calm_poollight") || strstr(tess.shader->name, "textures/liquids/clear_ripple1")) {
		material = MATERIAL_KIND_WATER;
		//tess.shader->rtstages[0]->active = qfalse;
		//tess.shader->rtstages[1]->active = qfalse;
		//tess.shader->rtstages[2]->active = qfalse;
		//tess.shader->rtstages[3]->active = qfalse;
	}
	if (strstr(tess.shader->name, "slashskate")) {
		material = MATERIAL_FLAG_SEE_THROUGH_ADD;
	}
	if (strstr(tess.shader->name, "f_machine")) {// bloodExplotion
		material = MATERIAL_FLAG_SEE_THROUGH_ADD;
	}
	if (strstr(tess.shader->name, "shotgun_laser")) {// bloodExplotion
		material = MATERIAL_FLAG_SEE_THROUGH_ADD;
	}
	if (strstr(tess.shader->name, "viewBloodBlend")) {// bloodExplotion
		material = MATERIAL_FLAG_SEE_THROUGH_ADD;
	}
	if (strstr(tess.shader->name, "fog")) {
		material = MATERIAL_FLAG_SEE_THROUGH;
	}
	if (strstr(tess.shader->name, "f_rocket")) {
		material = MATERIAL_FLAG_SEE_THROUGH_ADD;
	}
	
	//0x000002c0bebbae48 {name=0x000002c0bebbae48 "textures/base_trim/wires02" lightmapIndex=-3 index=87 ...}
	/*else if (tess.shader->sort == SS_UNDERWATER || tess.shader->sort == SS_BANNER) {
		material |= MATERIAL_FLAG_SEE_THROUGH;
	}
	else if (tess.shader->sort == SS_DECAL && ((tess.shader->contentFlags & CONTENTS_TRANSLUCENT) == CONTENTS_TRANSLUCENT)) {
		material |= MATERIAL_FLAG_SEE_THROUGH;
	}
	else if ((tess.shader->contentFlags & CONTENTS_TRANSLUCENT) == CONTENTS_TRANSLUCENT) {
		material |= MATERIAL_FLAG_SEE_THROUGH;
	}*/

	if ((backEnd.currentEntity->e.renderfx & RF_FIRST_PERSON) || strstr(tess.shader->name, "plasma_glass")) {
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
		if (stateBits == 34 || stateBits == 1073742080) blend = TEX0_ADD_BLEND_MASK;
		if (stateBits == 101) blend = TEX0_NORMAL_BLEND_MASK;
		if (strstr(tess.shader->name, "console/sphere2") && stage == 1) {
			//int x = 2; blend = TEX0_NORMAL_BLEND_MASK;
		}
		// 34 one one
		// 101 GLS_SRCBLEND_SRC_ALPHA GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA
		qboolean color = RB_StageNeedsColor(stage);

		uint32_t nextidx = (uint32_t)indexAnim;
		uint32_t idx = (uint32_t)tess.shader->rtstages[stage]->bundle[0].image[nextidx]->index;
		//if(strstr(tess.shader->name, "bloodExplosion")) idx = (uint32_t)tess.shader->rtstages[stage]->bundle[0].image[0]->index;
		tess.shader->rtstages[stage]->bundle[0].image[nextidx]->frameUsed = tr.frameCount;
		return (idx) | (blend) | (color ? TEX0_COLOR_MASK : 0);
	}
	return TEX0_IDX_MASK;
}