#version 460
#extension GL_NV_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : require
#include "RayPayload.glsl"
#include "RTHelper.glsl"
#include "RTBlend.glsl"


layout(push_constant) uniform PushConstant {
    layout(offset = 160) mat4 mvp;
};

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
  bool isMirror;
  bool isOpaque;
  bool isSky;
};
layout(binding = 4, set = 0) buffer Instance { iData data[]; } instanceData;

const mat4 clip = mat4(1.0f,  0.0f, 0.0f, 0.0f,
                         0.0f, -1.0f, 0.0f, 0.0f,
                         0.0f,  0.0f, 0.5f, 0.0f,
                         0.0f,  0.0f, 0.5f, 1.0f);

// https://media.contentapi.ea.com/content/dam/ea/seed/presentations/2019-ray-tracing-gems-chapter-20-akenine-moller-et-al.pdf
float calcLOD(ivec3 index){
  // screen space
  vec2 p0 =  (clip * mvp * vertices.v[index.x].pos).xy;
  vec2 p1 =  (clip * mvp * vertices.v[index.y].pos).xy;
  vec2 p2 =  (clip * mvp * vertices.v[index.z].pos).xy;
  // world space
  vec3 P0 =  vertices.v[index.x].pos.xyz;
  vec3 P1 =  vertices.v[index.y].pos.xyz;
  vec3 P2 =  vertices.v[index.z].pos.xyz;


  vec2 uv0 = vertices.v[index.x].uv.xy;
  vec2 uv1 = vertices.v[index.y].uv.xy;
  vec2 uv2 = vertices.v[index.z].uv.xy;

  ivec2 texSize = textureSize(tex[uint(uint(instanceData.data[gl_InstanceID].texIdx))], 0);

  float t_a = texSize.x * texSize.y * abs((uv1.x - uv0.x)*(uv2.y - uv0.y)-
                                          (uv2.x - uv0.x)*(uv1.y - uv0.y));

  float txn = texSize.x * texSize.y * abs((uv0.x - uv1.y)+(uv1.x - uv2.y)+(uv2.x - uv0.y)-
                                          (uv0.y - uv1.x)-(uv1.y - uv2.x)-(uv2.y - uv0.x));
  float xps = (texSize.x * texSize.y)/(gl_LaunchSizeNV.x * gl_LaunchSizeNV.y);
  float txs = txn * xps;
  // screen space
  //float p_a = abs((p1.x - p0.x)*(p2.y - p0.y)-(p2.x - p0.x)*(p1.y - p0.y));
  // world space
  float p_a = length(cross(P1-P0,P2-P0));

  vec3 AB = vertices.v[index.y].pos.xyz - vertices.v[index.x].pos.xyz;
  vec3 AC = vertices.v[index.z].pos.xyz - vertices.v[index.x].pos.xyz;
  vec3 n = normalize(cross(AB, AC));

  float lod = (0.5f * log2(t_a/p_a)); 
  //lod += log2(abs(gl_HitTNV));
  //lod += 0.5f * log2(gl_LaunchSizeNV.x * gl_LaunchSizeNV.y); 
  //lod -= log2(abs(dot(normalize(n),normalize(gl_WorldRayOriginNV + gl_RayTmaxNV * gl_WorldRayDirectionNV))));
  return 1 / lod;//sqrt(txs/p_a);
}

void main()
{
  //rp.depth += 1;
  //if(rp.depth > 2) return;
  if(rp.depth > 3) return;
  rp.depth++;

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
  vec4 color = /*(c/255) */ texture(tex[uint(uint(instanceData.data[gl_InstanceID].texIdx))], uv.xy);
  //vec4 color = textureLod(tex[uint(uint(instanceData.data[gl_InstanceID].texIdx))], uv.xy, calcLOD(index));
	rp.color += color;//vec4(color.w, color.w, color.w, color.w);//barycentricCoords;
  rp.blendFunc = instanceData.data[gl_InstanceID].blendfunc;
  rp.distance = gl_RayTmaxNV;
  rp.transparent = uint(instanceData.data[gl_InstanceID].texIdx2);


  // NORMAL
  vec3 AB = vertices.v[index.y].pos.xyz - vertices.v[index.x].pos.xyz;
  vec3 AC = vertices.v[index.z].pos.xyz - vertices.v[index.x].pos.xyz;
  rp.normal = vec4(normalize(cross(AB, AC)),1);//vertices.v[index.x].normal;

  if(instanceData.data[gl_InstanceID].isMirror == true){
    //rp.color = vec4(255,0,0,0);

    //gl_WorldRayOriginNV;
    //gl_WorldRayDirectionNV;

    vec3 direction2 = reflect(gl_WorldRayDirectionNV, rp.normal.xyz);
    uint rayFlags = gl_RayFlagsCullBackFacingTrianglesNV;// = /*gl_RayFlagsOpaqueNV | */gl_RayFlagsCullFrontFacingTrianglesNV ;
    rp.cullMask = MIRROR_VISIBLE;
    float tmin = 0.01;
    float tmax = 10000.0;
    traceNV(topLevelAS, rayFlags, rp.cullMask, 0, 0, 0, gl_WorldRayOriginNV + gl_RayTmaxNV * gl_WorldRayDirectionNV, tmin, direction2, tmax, 0);
  //
  } else if(instanceData.data[gl_InstanceID].isOpaque == false){
    uint rayFlags = gl_RayFlagsCullBackFacingTrianglesNV;// = gl_RayFlagsCullFrontFacingTrianglesNV ;
    float tmin = 0.01;
    float tmax = 10000.0;
    //traceNV(topLevelAS, rayFlags, rp.cullMask, 0, 0, 0, gl_WorldRayOriginNV + ((gl_RayTmaxNV+ 0.1) * gl_WorldRayDirectionNV), tmin, gl_WorldRayDirectionNV, tmax, 0);
  //rp.color = vec4(255,0,0,0);
  }

}  