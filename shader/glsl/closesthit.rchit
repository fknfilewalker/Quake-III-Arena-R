#version 460
#extension GL_NV_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : require
#include "RayPayload.glsl"
#include "RTHelper.glsl"
#include "RTBlend.glsl"


layout(binding = 0, set = 0) uniform accelerationStructureNV topLevelAS;
layout(binding = 0, set = 1) uniform sampler2D tex[];

layout(location = 0) rayPayloadInNV RayPayload rp;

struct v_s
{
  vec4 pos;
  vec4 uv;
  vec4 color;
};
layout(binding = 2, set = 0) buffer Vertices { v_s v[]; } vertices;

layout(binding = 3, set = 0) buffer Indices { uint i[]; } indices;

struct iData{
  float offsetIdx;
  float offsetXYZ;
  float texIdx;
  float texIdx2;
  uint blendfunc;
  float a;
  float b;
  float c;
};
layout(binding = 4, set = 0) buffer Instance { iData data[]; } instanceData;

void main()
{
  const vec3 barycentricCoords = getBarycentricCoordinates();

  uint customIndex = uint(instanceData.data[gl_InstanceID].offsetIdx) + (gl_PrimitiveID * 3);
	ivec3 index = (ivec3(indices.i[customIndex], indices.i[customIndex + 1], indices.i[customIndex + 2])) + int(instanceData.data[gl_InstanceID].offsetXYZ);

	vec4 uv = vertices.v[index.x].uv * barycentricCoords.x +
            vertices.v[index.y].uv * barycentricCoords.y +
            vertices.v[index.z].uv * barycentricCoords.z;

  vec4 c = vertices.v[index.x].color * barycentricCoords.x +
           vertices.v[index.y].color * barycentricCoords.y +
           vertices.v[index.z].color * barycentricCoords.z;

  // COLOR AND DISTANCE
  vec4 color = /*(c/255)*/ texture(tex[uint(uint(instanceData.data[gl_InstanceID].texIdx))], uv.xy);
	rp.color += color;//vec4(color.w, color.w, color.w, color.w);//barycentricCoords;
  rp.blendFunc = instanceData.data[gl_InstanceID].blendfunc;
  rp.distance = gl_RayTmaxNV;
  rp.transparent = uint(instanceData.data[gl_InstanceID].texIdx2);

  // NORMAL
  vec3 AB = vertices.v[index.y].pos.xyz - vertices.v[index.x].pos.xyz;
  vec3 AC = vertices.v[index.z].pos.xyz - vertices.v[index.x].pos.xyz;
  rp.normal = vec4(normalize(cross(AB, AC)),1);//vertices.v[index.x].normal;

  if(instanceData.data[gl_InstanceID].a == 1){
    //rp.color = vec4(255,0,0,0);

    //gl_WorldRayOriginNV;
    //gl_WorldRayDirectionNV;

    vec3 direction2 = reflect(gl_WorldRayDirectionNV, rp.normal.xyz);
    uint rayFlags = gl_RayFlagsCullBackFacingTrianglesNV;// = /*gl_RayFlagsOpaqueNV | */gl_RayFlagsCullFrontFacingTrianglesNV ;
    uint cullMask = MIRROR_VISIBLE;
    float tmin = 0.001;
    float tmax = 10000.0;
    traceNV(topLevelAS, rayFlags, cullMask, 0, 0, 0, gl_WorldRayOriginNV + gl_RayTmaxNV * gl_WorldRayDirectionNV, tmin, direction2, tmax, 0);
  //
  }

}   