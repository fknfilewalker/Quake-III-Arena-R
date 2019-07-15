
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 v_uv;

layout(binding = 0) uniform sampler2D tex;

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = texture(tex, v_uv.xy);// vec4(v_uv.x, v_uv.y, v_uv.z, 1);
}