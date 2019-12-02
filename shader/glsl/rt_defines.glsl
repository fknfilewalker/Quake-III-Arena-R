// struct RayPayload {
// 	vec4 color;
// 	vec4 normal;
// 	uint blendFunc;
// 	uint transparent;
// 	float distance;
// 	uint depth;
// 	uint cullMask;
// };
#define PAYLOAD_BRDF 0
#define PAYLOAD_SHADOW 1

#define ALBEDO_MULT 1.3

struct RayPayload {
	vec2 barycentric;
	uint instanceID;
	uint primitiveID;
	float hit_distance;
	vec4 transparent;
	float max_transparent_distance;
	mat4x3 modelmat;
};

struct RayPayloadShadow {
	int missed;
};

struct Ray {
	vec3 origin, direction;
	float t_min, t_max;
};



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
const float SS_BAD 									= 0;
const float SS_PORTAL 								= 1;	// mirrors, portals, viewscreens
const float SS_ENVIRONMENT 							= 2;	// sky box
const float SS_OPAQUE 								= 3;	// opaque

const float SS_DECAL 								= 4;	// scorch marks, etc.
const float SS_SEE_THROUGH 							= 5;	// ladders, grates, grills that may have small blended edges
																	// in addition to alpha test
const float SS_BANNER 								= 6;

const float SS_FOG 									= 7;

const float SS_UNDERWATER 							= 8;	// for items that should be drawn in front of the water plane

const float SS_BLEND0 								= 9;	// regular transparency and filters
const float SS_BLEND1 								= 10;	// generally only used for additive type effects
const float SS_BLEND2 								= 11;
const float SS_BLEND3 								= 12;

const float SS_BLEND6 								= 13;
const float SS_STENCIL_SHADOW 						= 14;
const float SS_ALMOST_NEAREST 						= 15;	// gun smoke puffs

const float SS_NEAREST 								= 16;	// blood blobs

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