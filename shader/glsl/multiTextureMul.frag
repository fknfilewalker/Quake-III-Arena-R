#version 450

layout(push_constant) uniform PushConstant {
    layout(offset = 192) int discardModeAlpha;
};
layout(set = 0, binding = 0) uniform sampler2D tex1;
layout(set = 1, binding = 0) uniform sampler2D tex2;

layout(location = 0) in vec4 frag_color;
layout(location = 1) in vec2 v_uv1;
layout(location = 2) in vec2 v_uv2;

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = frag_color * texture(tex1, v_uv1) * texture(tex2, v_uv2);

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