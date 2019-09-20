#version 460
#extension GL_NV_ray_tracing : require

struct RayPayload {
	vec3 color;
	float distance;
	vec4 normal;
};
layout(location = 0) rayPayloadInNV RayPayload rp;

void main()
{
    rp.color = vec3(0.0, 0.0, 0.2);
}