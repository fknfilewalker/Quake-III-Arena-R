layout(binding = BINDING_OFFSET_ASVGF_GRAD_SMPL_POS, set = 0, r32ui) uniform uimage2D gradSmplPosASVGFBuffer;
layout(binding = BINDING_OFFSET_ASVGF_GRAD_SMPL_POS_PREV, set = 0, r32ui) uniform uimage2D gradSmplPosASVGFBufferPrev;

layout(binding = BINDING_OFFSET_GRAD_HF_PING, set = 0, rg16f) uniform image2D gradHfASVGFPing;
layout(binding = BINDING_OFFSET_GRAD_HF_PONG, set = 0, rg16f) uniform image2D gradHfASVGFPong;

layout(binding = BINDING_OFFSET_POS_FWD, set = 0, rgba32f) uniform image2D posFwd;
layout(binding = BINDING_OFFSET_OBJECT_FWD, set = 0, rgba32f) uniform image2D objectFwd;
layout(binding = BINDING_OFFSET_TEX_GRAD_FWD_0, set = 0, rgba16f) uniform image2D texGradFwd0;
layout(binding = BINDING_OFFSET_TEX_GRAD_FWD_1, set = 0, rgba16f) uniform image2D texGradFwd1;
layout(binding = BINDING_OFFSET_TEX_GRAD_FWD_2, set = 0, rgba16f) uniform image2D texGradFwd2;
layout(binding = BINDING_OFFSET_TEX_GRAD_FWD_3, set = 0, rgba16f) uniform image2D texGradFwd3;


#define STRATUM_OFFSET_SHIFT 3
#define STRATUM_OFFSET_MASK ((1 << STRATUM_OFFSET_SHIFT) - 1)

bool get_is_gradient(ivec2 ipos)
{
	if(true)
	{
		uint u = imageLoad(gradSmplPosASVGFBuffer, ipos / GRAD_DWN).r;

		ivec2 grad_strata_pos = ivec2(
				u >> (STRATUM_OFFSET_SHIFT * 0),
				u >> (STRATUM_OFFSET_SHIFT * 1)) & STRATUM_OFFSET_MASK;

		return (u > 0 && all(equal(grad_strata_pos, ipos % GRAD_DWN)));
	}
	
	return false;
}