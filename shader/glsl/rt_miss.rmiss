#version 460
#extension GL_NV_ray_tracing : require
#extension GL_GOOGLE_include_directive : require
#include "rt_defines.glsl"

layout(location = 0) rayPayloadInNV RayPayload rp;

void main()
{
	rp.instanceID = ~0u;
}