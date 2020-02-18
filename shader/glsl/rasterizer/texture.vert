#version 450

layout(push_constant) uniform PushConstant {
    layout(offset = 0) mat4 mvp;
    layout(offset = 64) mat4 mv;
    layout(offset = 128) vec4 clipping_plane;
    layout(offset = 144) bool clip;
};

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 in_color;
layout(location = 2) in vec2 uv1;
layout(location = 3) in vec2 uv2;

layout(location = 0) out vec4 frag_color;
layout(location = 1) out vec2 v_uv1;
layout(location = 2) out vec2 v_uv2;

out gl_PerVertex {
    vec4 gl_Position;
    float gl_ClipDistance[1];
};

void main() {
	vec4 pos = vec4(position.xyz, 1.0);

    gl_Position = mvp * pos;

    if(clip) gl_ClipDistance[0] = dot(clipping_plane, (mv * pos));
    else gl_ClipDistance[0] = 0;

    frag_color = in_color;
    v_uv1 = uv1;
    v_uv2 = uv2;
}