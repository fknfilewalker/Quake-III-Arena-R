
const uint GLS_SRCBLEND_ZERO						= 0x00000001;
const uint GLS_SRCBLEND_ONE						= 0x00000002;
const uint GLS_SRCBLEND_DST_COLOR					= 0x00000003;
const uint GLS_SRCBLEND_ONE_MINUS_DST_COLOR		= 0x00000004;
const uint GLS_SRCBLEND_SRC_ALPHA					= 0x00000005;
const uint GLS_SRCBLEND_ONE_MINUS_SRC_ALPHA		= 0x00000006;
const uint GLS_SRCBLEND_DST_ALPHA					= 0x00000007;
const uint GLS_SRCBLEND_ONE_MINUS_DST_ALPHA		= 0x00000008;
const uint GLS_SRCBLEND_ALPHA_SATURATE				= 0x00000009;
const uint		GLS_SRCBLEND_BITS					= 0x0000000f;

const uint GLS_DSTBLEND_ZERO						= 0x00000010;
const uint GLS_DSTBLEND_ONE						= 0x00000020;
const uint GLS_DSTBLEND_SRC_COLOR					= 0x00000030;
const uint GLS_DSTBLEND_ONE_MINUS_SRC_COLOR		= 0x00000040;
const uint GLS_DSTBLEND_SRC_ALPHA					= 0x00000050;
const uint GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA		= 0x00000060;
const uint GLS_DSTBLEND_DST_ALPHA					= 0x00000070;
const uint GLS_DSTBLEND_ONE_MINUS_DST_ALPHA		= 0x00000080;
const uint		GLS_DSTBLEND_BITS					= 0x000000f0;

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