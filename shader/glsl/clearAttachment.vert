#version 450

layout(location = 0) in vec4 position;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position = vec4(position.xyz, 1.0);
}


