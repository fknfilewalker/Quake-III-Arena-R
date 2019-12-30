#version 460
#extension GL_NV_ray_tracing : require
#extension GL_GOOGLE_include_directive : require
#include "rt_defines.glsl"
layout(location = PAYLOAD_SHADOW) rayPayloadInNV ShadowRayPayload rp;

void main()
{
  // If we miss all geometry, then the light is visibile
	rp.visFactor = 1.0f;
}