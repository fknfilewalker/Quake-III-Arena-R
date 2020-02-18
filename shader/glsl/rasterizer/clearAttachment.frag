#version 450

layout(push_constant) uniform PushConstant {
    layout(offset = 0) vec4 color;
};

layout(location = 0) out vec4 fragColor;


void main() {

    fragColor = color;
}