#include "constants.h"

layout(binding = 0, set = 1) uniform sampler2D texure_array[];

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

float
get_rng(uint idx)
{
	return texelFetch(blue_noise, ivec3(gl_LaunchIDNV.x % BLUE_NOISE_RES,
								gl_LaunchIDNV.y % BLUE_NOISE_RES, 
								idx), 0).r;
}
