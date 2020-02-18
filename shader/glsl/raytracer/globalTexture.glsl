#include "../constants.h"

// global texture
layout(binding = 0, set = 1) uniform sampler2D texure_array[];
// vis data
layout(binding = BINDING_OFFSET_VIS_DATA, set = 0, r8ui) uniform uimage2D vis_data;
layout(binding = BINDING_OFFSET_LIGHT_VIS_DATA, set = 0, r32ui) uniform uimage2D lightVis_data;

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

vec4
alpha_blend(vec4 top, vec4 bottom)
{
    // assume top is alpha-premultiplied, bottom is not; result is premultiplied
    return vec4(top.rgb + bottom.rgb * (1 - top.a) * bottom.a, 1 - (1 - top.a) * (1 - bottom.a)); 
}

vec4 alpha_blend_premultiplied(vec4 top, vec4 bottom)
{
    // assume everything is alpha-premultiplied
    return vec4(top.rgb + bottom.rgb * (1 - top.a), 1 - (1 - top.a) * (1 - bottom.a)); 
}