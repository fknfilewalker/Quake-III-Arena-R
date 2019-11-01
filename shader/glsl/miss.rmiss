#version 460
#extension GL_NV_ray_tracing : require
#extension GL_GOOGLE_include_directive : require
#include "RayPayload.glsl"
#include "RTBlend.glsl"


layout(binding = 0, set = 0) uniform accelerationStructureNV topLevelAS;

layout(location = 0) rayPayloadInNV RayPayload rp;

//layout(binding = 5) uniform samplerCube samplerCubeMap;
//layout(binding = 5) uniform sampler2D samplerCubeMap;

/*
vec2 sampleCube(vec3 v)
{
	float faceIndex;
	vec3 vAbs = abs(v);
	float ma;
	vec2 uv;
	if(vAbs.z >= vAbs.x && vAbs.z >= vAbs.y)
	{
		faceIndex = v.z < 0.0 ? 5.0 : 4.0;
		ma = 0.5 / vAbs.z;
		uv = vec2(v.z < 0.0 ? -v.x : v.x, -v.y);
	}
	else if(vAbs.y >= vAbs.x)
	{
		faceIndex = v.y < 0.0 ? 3.0 : 2.0;
		ma = 0.5 / vAbs.y;
		uv = vec2(v.x, v.y < 0.0 ? -v.z : v.z);
	}
	else
	{
		faceIndex = v.x < 0.0 ? 1.0 : 0.0;
		ma = 0.5 / vAbs.x;
		uv = vec2(v.x < 0.0 ? v.z : -v.z, -v.y);
	}
	return uv * ma + 0.5;
}
*/

void main()
{
	//rp.color = texture(samplerCubeMap, gl_WorldRayDirectionNV);
	//rp.color = texture(samplerCubeMap, sampleCube(gl_WorldRayDirectionNV.xyz));
    
    if(rp.miss == true) rp.color = vec4(0.0, 0.0, 0.2, 0);
    else{
	    rp.miss = true;
	    uint rayFlags = 0;
	    uint cullMask = SKY_VISIBLE;
	    float tmin = 0.1;
	    float tmax = 10000.0;
	    traceNV(topLevelAS, rayFlags, cullMask, 0, 0, 0, gl_WorldRayOriginNV, tmin, gl_WorldRayDirectionNV, tmax, 0);
	}

}