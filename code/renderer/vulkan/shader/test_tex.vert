#version 450

layout(push_constant) uniform PushConstant {
    layout(offset = 0) mat4 mvp;
};

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 in_color;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec4 frag_color;
layout(location = 1) out vec2 v_uv;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position = mvp * vec4(position.xyz, 1.0);
    frag_color = in_color;
    v_uv = uv;
}


