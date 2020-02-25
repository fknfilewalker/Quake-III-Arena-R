#include "../constants.h"

// global texture
layout(binding = 0, set = 1) uniform sampler2D texure_array[];

// texture getter
vec4
global_texture(in uint idx, in vec2 tex_coord)
{
	return texture(texure_array[idx], tex_coord);
}
vec4
global_textureLod(in uint idx, in vec2 tex_coord, in uint lod)
{
	return textureLod(texure_array[idx], tex_coord, lod);
}
vec4
global_textureGrad(in uint idx, in vec2 tex_coord, in vec2 d_x, in vec2 d_y)
{
	return textureGrad(texure_array[idx], tex_coord, d_x, d_y);
}
ivec2
global_textureSize(in uint idx, in int level)
{
	return textureSize(texure_array[idx], level);
}

// blend
vec4
alpha_blend(in vec4 top, in vec4 bottom)
{
    // assume top is alpha-premultiplied, bottom is not; result is premultiplied
    return vec4(top.rgb + bottom.rgb * (1 - top.a) * bottom.a, 1 - (1 - top.a) * (1 - bottom.a)); 
}
vec4 alpha_blend_premultiplied(in vec4 top,  in vec4 bottom)
{
    // assume everything is alpha-premultiplied
    return vec4(top.rgb + bottom.rgb * (1 - top.a), 1 - (1 - top.a) * (1 - bottom.a)); 
}

vec4 alphaBlendSimple(in vec4 top, in vec4 bottom)
{
	return bottom * bottom.w + top * (1-bottom.w);
}

// unpack texture idx, blend/add, and req color, data
TextureData unpackTextureData(in uint data){
	TextureData d;
	d.tex0 = int(data & TEX0_IDX_MASK);
	d.tex1 = int((data & TEX1_IDX_MASK) >> TEX_SHIFT_BITS);
	if(d.tex0 == TEX0_IDX_MASK) d.tex0 = -1;
	if(d.tex1 == TEX0_IDX_MASK) d.tex1 = -1;
	d.tex0Blend = (data & TEX0_BLEND_MASK);
	d.tex1Blend = (data & TEX1_BLEND_MASK);
	d.tex0Color = (data & TEX0_COLOR_MASK) != 0;
	d.tex1Color = (data & TEX1_COLOR_MASK) != 0;
	return d;
}

vec4 getTextureWithLod(in HitPoint hp, in uint lod){

	TextureData d = unpackTextureData(hp.tex0);
	TextureData d2 = unpackTextureData(hp.tex1);
	vec4 color = vec4(0);
	if(d.tex0 != -1){
		vec4 tex = global_textureLod(d.tex0, hp.uv0, lod);
		// if we have more tex to blend set alpha to 1
		if(d.tex1 != -1) color = vec4(tex.xyz, 1);
		else color = tex;
		if(d.tex0Color) color *= (hp.color0/255);
	} else return color;
	if(d.tex1 != -1){
		vec4 tex = global_textureLod(d.tex1, hp.uv1, lod);
		if(d.tex1Color) tex *= (hp.color1/255);
		if(d.tex1Blend == TEX1_NORMAL_BLEND_MASK) {
			color = alpha_blend(tex, color);
		}
		else if(d.tex1Blend == TEX1_MUL_BLEND_MASK){
			color.xyz *= tex.xyz;
		}
		else if(d.tex1Blend == TEX1_ADD_BLEND_MASK){
			color += tex;
		}
		else color += tex;
		//color.xyz *= tex.xyz;
	} else return color;
	if(d2.tex0 != -1){
		vec4 tex = global_textureLod(d2.tex0, hp.uv2, lod);
		if(d2.tex0Color) tex *= (hp.color2/255);
		if(d2.tex0Blend == TEX0_NORMAL_BLEND_MASK) {
			color = alpha_blend(tex, color);
		}
		else if(d2.tex0Blend == TEX0_MUL_BLEND_MASK){
			color.xyz *= tex.xyz;
		}
		else if(d2.tex0Blend == TEX0_ADD_BLEND_MASK){
			color += tex;
		}
		else color += tex;
	} else return color;
	if(d2.tex1 != -1){
		vec4 tex = global_textureLod(d2.tex1, hp.uv3, lod);
		if(d2.tex1Color) tex *= (hp.color3/255);
		if(d2.tex1Blend == TEX1_NORMAL_BLEND_MASK) {
			color = alpha_blend(tex, color);
		}
		else if(d2.tex1Blend == TEX1_MUL_BLEND_MASK){
			color.xyz *= tex.xyz;
		}
		else if(d2.tex1Blend == TEX1_ADD_BLEND_MASK){
			color += tex;
		}
		else color += tex;
	} 
	return color;
}