#version 450

layout(push_constant) uniform PushConstant {
    layout(offset = 192) int discardModeAlpha;
};
layout(binding = 0) uniform sampler2D tex;

layout(location = 0) in vec4 frag_color;
layout(location = 1) in vec2 v_uv;

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = frag_color * texture(tex, v_uv.xy);

    switch(discardModeAlpha)
    {
	    case 1: if (fragColor.a == 0.0f) discard;
	    case 2: if (fragColor.a >= 0.5f) discard;
	    case 3: if (fragColor.a <  0.5f) discard;
	}
}