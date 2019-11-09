struct RayPayload {
	vec4 color;
	vec4 normal;
	uint blendFunc;
	uint transparent;
	float distance;
	uint depth;
	uint cullMask;
};

// cullMask
const uint FIRST_PERSON_VISIBLE 					= 0x00000001u;
const uint MIRROR_VISIBLE 							= 0x00000002u;
const uint FIRST_PERSON_MIRROR_VISIBLE 				= 0x00000003u;
const uint SKY_VISIBLE 								= 0x00000004u;

// blendBits
const uint GLS_SRCBLEND_ZERO						= 0x00000001;
const uint GLS_SRCBLEND_ONE							= 0x00000002;
const uint GLS_SRCBLEND_DST_COLOR					= 0x00000003;
const uint GLS_SRCBLEND_ONE_MINUS_DST_COLOR			= 0x00000004;
const uint GLS_SRCBLEND_SRC_ALPHA					= 0x00000005;
const uint GLS_SRCBLEND_ONE_MINUS_SRC_ALPHA			= 0x00000006;
const uint GLS_SRCBLEND_DST_ALPHA					= 0x00000007;
const uint GLS_SRCBLEND_ONE_MINUS_DST_ALPHA			= 0x00000008;
const uint GLS_SRCBLEND_ALPHA_SATURATE				= 0x00000009;
const uint		GLS_SRCBLEND_BITS					= 0x0000000f;

const uint GLS_DSTBLEND_ZERO						= 0x00000010;
const uint GLS_DSTBLEND_ONE							= 0x00000020;
const uint GLS_DSTBLEND_SRC_COLOR					= 0x00000030;
const uint GLS_DSTBLEND_ONE_MINUS_SRC_COLOR			= 0x00000040;
const uint GLS_DSTBLEND_SRC_ALPHA					= 0x00000050;
const uint GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA			= 0x00000060;
const uint GLS_DSTBLEND_DST_ALPHA					= 0x00000070;
const uint GLS_DSTBLEND_ONE_MINUS_DST_ALPHA			= 0x00000080;
const uint		GLS_DSTBLEND_BITS					= 0x000000f0;

// for shaderSort_t
const uint SS_BAD 									= 0x00000000u;
const uint SS_PORTAL 								= 0x00000001u;	// mirrors, portals, viewscreens
const uint SS_ENVIRONMENT 							= 0x00000002u;	// sky box
const uint SS_OPAQUE 								= 0x00000003u;	// opaque

const uint SS_DECAL 								= 0x00000004u;	// scorch marks, etc.
const uint SS_SEE_THROUGH 							= 0x00000005u;	// ladders, grates, grills that may have small blended edges
																	// in addition to alpha test
const uint SS_BANNER 								= 0x00000006u;

const uint SS_FOG 									= 0x00000007u;

const uint SS_UNDERWATER 							= 0x00000008u;	// for items that should be drawn in front of the water plane

const uint SS_BLEND0 								= 0x00000009u;	// regular transparency and filters
const uint SS_BLEND1 								= 0x0000000au;	// generally only used for additive type effects
const uint SS_BLEND2 								= 0x0000000bu;
const uint SS_BLEND3 								= 0x0000000cu;

const uint SS_BLEND6 								= 0x0000000du;
const uint SS_STENCIL_SHADOW 						= 0x0000000eu;
const uint SS_ALMOST_NEAREST 						= 0x0000000fu;	// gun smoke puffs

const uint SS_NEAREST 								= 0x00000010u;	// blood blobs

vec4 blendColor(vec4 src, vec4 dst, uint blendFunc){
	vec4 srcBlend;
	vec4 dstBlend;

	// src
	switch(blendFunc & GLS_SRCBLEND_BITS){
		case (GLS_SRCBLEND_ZERO):
			srcBlend = vec4(0,0,0,0);
			break;
		case (GLS_SRCBLEND_ONE):
			srcBlend = vec4(1,1,1,1);
			break;
		case (GLS_SRCBLEND_DST_COLOR):
			srcBlend = vec4(dst/1);
			break;
		case (GLS_SRCBLEND_ONE_MINUS_DST_COLOR):
			srcBlend = 1 - vec4(dst/1);
			break;
		case (GLS_SRCBLEND_SRC_ALPHA): // light
			srcBlend = vec4(src.w, src.w, src.w, src.w)/1;
			break;
		case (GLS_SRCBLEND_ONE_MINUS_SRC_ALPHA):
			srcBlend = 1 - (vec4(src.w, src.w, src.w, src.w)/1);
			break;
		case (GLS_SRCBLEND_DST_ALPHA):
			srcBlend = vec4(dst.w, dst.w, dst.w, dst.w)/1;
			break;
		case (GLS_SRCBLEND_ONE_MINUS_DST_ALPHA):
			srcBlend = 1 - (vec4(dst.w, dst.w, dst.w, dst.w)/1);
			break;
		default:
			srcBlend = vec4(1,1,1,1);
	}
	// dst
	switch(blendFunc & GLS_DSTBLEND_BITS){
		case (GLS_DSTBLEND_ZERO):
			dstBlend = vec4(0,0,0,0);
			break;
		case (GLS_DSTBLEND_ONE):
			dstBlend = vec4(1,1,1,1);
			break;
		case (GLS_DSTBLEND_SRC_COLOR):
			dstBlend = vec4(src/1);
			break;
		case (GLS_DSTBLEND_ONE_MINUS_SRC_COLOR):
			dstBlend = 1 - vec4(src/1);
			break;
		case (GLS_DSTBLEND_SRC_ALPHA):
			dstBlend = vec4(src.w, src.w, src.w, src.w)/1;
			break;
		case (GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA): //light
			dstBlend = 1 - (vec4(src.w, src.w, src.w, src.w)/1);
			break;
		case (GLS_DSTBLEND_DST_ALPHA):
			dstBlend = vec4(dst.w, dst.w, dst.w, dst.w)/1;
			break;
		case (GLS_DSTBLEND_ONE_MINUS_DST_ALPHA):
			dstBlend = 1 - (vec4(dst.w, dst.w, dst.w, dst.w)/1);
			break;
		default:
			dstBlend = vec4(1,1,1,1);
	}
	return (src * srcBlend) + (dst * dstBlend);

}