#version 460
#extension GL_NV_ray_tracing : require

struct RayPayload {
	vec4 color;
	vec4 normal;
	float distance;
};
layout(location = 0) rayPayloadInNV RayPayload rp;

void main()
{
    rp.color = vec4(0.0, 0.0, 0.2, 0);
}