#include "constants.h"

// global texture
layout(binding = 0, set = 1) uniform sampler2D texure_array[];
// blue noise
layout(binding = BINDING_OFFSET_BLUE_NOISE, set = 0) uniform sampler2DArray blue_noise;
// vis data
layout(binding = BINDING_OFFSET_VIS_DATA, set = 0, r8ui) uniform uimage2D vis_data;

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

vec4 getTexture(HitPoint hp, uint lod){
	TextureData d = unpackTextureData(hp.tex0);
	vec4 color;
	vec4 tex = global_textureLod(d.tex0, hp.uv0, lod);
	color = vec4(tex.xyz, 1);
	if(d.tex0Color) color *= (hp.color0/255);

	if(d.tex1 != -1){
		tex = global_textureLod(d.tex1, hp.uv1, lod);
		if(d.tex1Color) tex *= (hp.color1/255);

		if(d.tex1Blend) {
			color = alpha_blend(tex, color);
		}
		else color += tex;
	} else return color;

	d = unpackTextureData(hp.tex1);
	if(d.tex0 != -1){
		tex = global_textureLod(d.tex0, hp.uv2, lod);
		if(d.tex0Color) tex *= (hp.color2/255);

		if(d.tex0Blend) {
			color = alpha_blend(tex, color);
		}
		else color += tex;
	} else return color;

	if(d.tex1 != -1){
		tex = global_textureLod(d.tex1, hp.uv3, lod);
		if(d.tex1Color) tex *= (hp.color3/255);

		if(d.tex1Blend) {
			color = alpha_blend(tex, color);
		}
		else color += tex;
	} 
	return color;
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
