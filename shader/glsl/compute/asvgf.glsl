

layout(binding = BINDING_OFFSET_POS_FWD, set = 0, rgba32f) uniform image2D posFwd;
layout(binding = BINDING_OFFSET_OBJECT_FWD, set = 0, rgba32f) uniform image2D objectFwd;




layout(binding = BINDING_OFFSET_ASVGF_HIST_COLOR_S, set = 0) uniform sampler2D texHistColor_S;

// layout(binding = BINDING_OFFSET_ASVGF_ATROUS_A, set = 0, rgba16f) uniform image2D atrousASVGF_A;
// layout(binding = BINDING_OFFSET_ASVGF_ATROUS_B, set = 0, rgba16f) uniform image2D atrousASVGF_B;

layout(binding = BINDING_OFFSET_ASVGF_ATROUS_A_SAMPLER, set = 0) uniform sampler2D atrousASVGF_A_S;
layout(binding = BINDING_OFFSET_ASVGF_ATROUS_B_SAMPLER, set = 0) uniform sampler2D atrousASVGF_B_S;

layout(binding = BINDING_OFFSET_ASVGF_TAA, set = 0, rgba16f) uniform image2D imgtaaASVGF;
layout(binding = BINDING_OFFSET_ASVGF_TAA_SAMPLER, set = 0) uniform sampler2D taaASVGF;
layout(binding = BINDING_OFFSET_ASVGF_TAA_PREV, set = 0) uniform sampler2D taaASVGF_Prev;



layout(binding = BINDING_OFFSET_ASVGF_DEBUG, set = 0, rgba32f) uniform image2D IMG_ASVGF_DEBUG;

layout(binding = BINDING_OFFSET_ASVGF_RNG, set = 0, r32ui) uniform uimage2D IMG_ASVGF_RNG_SEED;
layout(binding = BINDING_OFFSET_ASVGF_RNG_PREV, set = 0, r32ui) uniform uimage2D IMG_ASVGF_RNG_SEED_PREV;

layout(binding = BINDING_OFFSET_ASVGF_GRAD_SMPL_POS, set = 0, r32ui) uniform uimage2D IMG_ASVGF_GRAD_SAMPLE_POS;
layout(binding = BINDING_OFFSET_ASVGF_GRAD_SMPL_POS_PREV, set = 0, r32ui) uniform uimage2D IMG_ASVGF_GRAD_SAMPLE_POS_PREV;

layout(binding = BINDING_OFFSET_ASVGF_GRAD_A, set = 0, rgba16f) uniform image2D IMG_ASVGF_GRAD_A;
layout(binding = BINDING_OFFSET_ASVGF_GRAD_B, set = 0, rgba16f) uniform image2D IMG_ASVGF_GRAD_B;
layout(binding = BINDING_OFFSET_ASVGF_GRAD_A_SAMPLER, set = 0) uniform sampler2D TEX_ASVGF_GRAD_A;
layout(binding = BINDING_OFFSET_ASVGF_GRAD_B_SAMPLER, set = 0) uniform sampler2D TEX_ASVGF_GRAD_B;

layout(binding = BINDING_OFFSET_ASVGF_HIST_MOMENTS, set = 0, rgba16f) uniform image2D IMG_ASVGF_HIST_MOM;
layout(binding = BINDING_OFFSET_ASVGF_HIST_MOMENTS_PREV, set = 0, rgba16f) uniform image2D IMG_ASVGF_HIST_MOM_PREV;
layout(binding = BINDING_OFFSET_ASVGF_HIST_COLOR, set = 0, rgba16f) uniform image2D IMG_ASVGF_HIST_COLOR;
layout(binding = BINDING_OFFSET_ASVGF_HIST_COLOR_PREV, set = 0, rgba16f) uniform image2D IMG_ASVGF_HIST_COLOR_PREV;

layout(binding = BINDING_OFFSET_ASVGF_ATROUS_A, set = 0, rgba16f) uniform image2D IMG_ASVGF_ATROUS_A;
layout(binding = BINDING_OFFSET_ASVGF_ATROUS_B, set = 0, rgba16f) uniform image2D IMG_ASVGF_ATROUS_B;

layout(binding = BINDING_OFFSET_ASVGF_COLOR, set = 0, rgba16f) uniform image2D IMG_ASVGF_COLOR;


layout(binding = BINDING_OFFSET_RESULT, set = 0, rgba32f) uniform image2D resultImg;

#define STRATUM_OFFSET_SHIFT 3
#define STRATUM_OFFSET_MASK ((1 << STRATUM_OFFSET_SHIFT) - 1)

const float gaussian_kernel[2][2] = {
	{ 1.0 / 4.0, 1.0 / 8.0  },
	{ 1.0 / 8.0, 1.0 / 16.0 }
};

bool is_gradient(ivec2 ipos)
{
    uint u = imageLoad(IMG_ASVGF_GRAD_SAMPLE_POS, ipos / GRAD_DWN).r;

    ivec2 grad_strata_pos = ivec2(
            u >> (STRATUM_OFFSET_SHIFT * 0),
            u >> (STRATUM_OFFSET_SHIFT * 1)) & STRATUM_OFFSET_MASK;

    return (u > 0 && all(equal(grad_strata_pos, ipos % GRAD_DWN)));

}
