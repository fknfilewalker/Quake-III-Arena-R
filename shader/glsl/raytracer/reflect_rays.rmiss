#version 460
#extension GL_NV_ray_tracing : require
#extension GL_GOOGLE_include_directive : require
#include "../constants.h"
#include "defines.glsl"
#include "sky.glsl"

layout(location = PAYLOAD_SHADOW) rayPayloadInNV RayPayloadReflect rrp;

void main()
{
    //if(rrp.depth > 1) return;
    rrp.pos = vec3(0);
    rrp.normal = vec3(0,0,1);
    rrp.material = MATERIAL_KIND_SKY;
    rrp.color = sampleSky(gl_WorldRayDirectionNV);
    rrp.object = vec4(-1, -1, uintBitsToFloat(uvec2(~0u)));
}