
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 uv;

out gl_PerVertex {
    vec4 gl_Position;
};

layout(location = 0) out vec3 v_uv;

void main() {
    gl_Position = position;
    v_uv = uv;
}


