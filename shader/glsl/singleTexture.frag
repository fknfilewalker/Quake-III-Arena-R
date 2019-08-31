#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(push_constant) uniform PushConstant {
    layout(offset = 192) int discardModeAlpha;
    layout(offset = 196) int textureIdx1; 
    layout(offset = 200) int textureIdx2;
};
layout(binding = 0) uniform sampler2D tex[];

layout(location = 0) in vec4 frag_color;
layout(location = 1) in vec2 v_uv;

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = frag_color * texture(tex[textureIdx1], v_uv.xy);

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