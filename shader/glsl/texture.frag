#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : require
#include "constants.h"

layout(push_constant) uniform PushConstant {
	layout(offset = 148) uint textureType;
    layout(offset = 152) uint discardModeAlpha;
    layout(offset = 156) uint textureIdx1; 
    layout(offset = 160) uint textureIdx2;
};
layout(binding = 0) uniform sampler2D tex[];

layout(location = 0) in vec4 frag_color;
layout(location = 1) in vec2 v_uv1;
layout(location = 2) in vec2 v_uv2;

layout(location = 0) out vec4 fragColor;

void main() {
	switch(textureType)
    {
		case TEXTURE_ADD: 
			// multi texture add
			vec4 color_a = frag_color * texture(tex[textureIdx1], v_uv1);
			vec4 color_b = texture(tex[textureIdx2], v_uv2);
			fragColor = vec4(color_a.rgb + color_b.rgb, color_a.a * color_b.a);
			break;
		case TEXTURE_MUL: 
			// multi texture mul
			fragColor = frag_color * texture(tex[textureIdx1], v_uv1) * texture(tex[textureIdx2], v_uv2);
			break;
		case TEXTURE_DEFAULT: 
		default:
			// single texture
			fragColor = frag_color * texture(tex[textureIdx1], v_uv1.xy);
			break;
	}

    switch(discardModeAlpha)
    {
	    case 1: 
	    	if (fragColor.a == 0.0f) discard;
	    	break;
	    case 2: 
	    	if (fragColor.a >= 0.5f) discard;
	    	break;
	    case 3: 
	    	if (fragColor.a <  0.5f) discard;
	    	break;
	}
}