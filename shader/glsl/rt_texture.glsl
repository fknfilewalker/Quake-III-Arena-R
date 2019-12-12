#include "constants.h"

// global texture
layout(binding = 0, set = 1) uniform sampler2D texure_array[];
// blue noise
layout(binding = BINDING_OFFSET_BLUE_NOISE, set = 0) uniform sampler2DArray blue_noise;

// texture getter
vec4
global_texture(uint idx, vec2 tex_coord)
{
	return texture(texure_array[idx], tex_coord);
}
vec4
global_textureLod(uint idx, vec2 tex_coord, uint lod)
{
	return textureLod(texure_array[idx], tex_coord, lod);
}
vec4
global_textureGrad(uint idx, vec2 tex_coord, vec2 d_x, vec2 d_y)
{
	return textureGrad(texure_array[idx], tex_coord, d_x, d_y);
}
ivec2
global_textureSize(uint idx, int level)
{
	return textureSize(texure_array[idx], level);
}

uint
get_rng_seed(int frame_num)
{
	ivec2 ipos = ivec2(gl_LaunchIDNV);
	uint rng_seed = 0;
	uint frame_offset = frame_num / NUM_BLUE_NOISE_TEX;

	rng_seed |= (uint(ipos.x + frame_offset) % BLUE_NOISE_RES) <<  0u;
	rng_seed |= (uint(ipos.y + (frame_offset << 4)) % BLUE_NOISE_RES) << 10u;
	rng_seed |= uint(frame_num) << 20;
	return rng_seed;
}
float
get_rng(uint idx, int frame_num)
{
	uint rng_seed = get_rng_seed(frame_num);
	uvec3 p = uvec3(rng_seed, rng_seed >> 10, rng_seed >> 20);
	p.z = (p.z + idx);
	p &= uvec3(BLUE_NOISE_RES - 1, BLUE_NOISE_RES - 1, NUM_BLUE_NOISE_TEX - 1);

	return min(texelFetch(blue_noise, ivec3(p), 0).r, 0.9999999999999);
	//return fract(vec2(get_rng_uint(idx)) / vec2(0xffffffffu));
}
