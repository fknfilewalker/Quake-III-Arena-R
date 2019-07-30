#version 450

layout(push_constant) uniform PushConstant {
    layout(offset = 0) mat4 mvp;
    layout(offset = 64) mat4 mv;
    layout(offset = 128) vec4 clipping_plane; // in eye space
};

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 in_color;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec4 frag_color;
layout(location = 1) out vec2 v_uv;

out gl_PerVertex {
    vec4 gl_Position;
    float gl_ClipDistance[1];
};

void main() {
	vec4 pos = vec4(position.xyz, 1.0);

    gl_Position = mvp * pos;
	gl_ClipDistance[0] = dot(clipping_plane, (mv * pos));

    frag_color = in_color;
    v_uv = uv;


}


