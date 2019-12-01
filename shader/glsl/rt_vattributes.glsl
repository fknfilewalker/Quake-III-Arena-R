#include "constants.h"

// out data
struct Triangle {
	mat3 pos;
	mat3x2 uv;
	vec3 normal;
	mat3x4 color;
};
struct HitPoint {
	vec3 pos;
	vec3 normal;
	vec2 uv;
	vec4 color;
};

// buffer with instance data
struct InstanceData{
  float offsetIdx;
  float offsetXYZ;
  float texIdx;
  uint material;
  uint blendfunc;
  float shaderSort;
  uint type;
};
layout(binding = BINDING_OFFSET_INSTANCE_DATA, set = 0) buffer Instance { InstanceData data[]; } iData;

// Buffer with indices and vertices
struct VABuffer
{
  vec4 pos;
  vec4 uv;
  vec4 color;
};
layout(binding = BINDING_OFFSET_XYZ, set = 0) buffer Vertices { VABuffer v[]; } vertices;
layout(binding = BINDING_OFFSET_IDX, set = 0) buffer Indices { uint i[]; } indices;

vec3 getBarycentricCoordinates(vec2 hitAttribute) { return vec3(1.0f - hitAttribute.x - hitAttribute.y, hitAttribute.x, hitAttribute.y); }

Triangle getTriangle(uint instanceID, uint primitiveID){
  	uint customIndex = uint(iData.data[instanceID].offsetIdx) + (primitiveID * 3);
	ivec3 index = (ivec3(indices.i[customIndex], indices.i[customIndex + 1], indices.i[customIndex + 2])) + int(iData.data[instanceID].offsetXYZ);

	Triangle hitTriangle;
	// POS
	hitTriangle.pos[0] = vertices.v[index.x].pos.xyz;
	hitTriangle.pos[1] = vertices.v[index.y].pos.xyz;
	hitTriangle.pos[2] = vertices.v[index.z].pos.xyz;
	// UV
	hitTriangle.uv[0] = vertices.v[index.x].uv.xy;
	hitTriangle.uv[1] = vertices.v[index.y].uv.xy;
	hitTriangle.uv[2] = vertices.v[index.z].uv.xy;
	// Color
	hitTriangle.color[0] = vertices.v[index.x].color;
	hitTriangle.color[1] = vertices.v[index.y].color;
	hitTriangle.color[2] = vertices.v[index.z].color;
	// NORMAL
	vec3 AB = vertices.v[index.y].pos.xyz - vertices.v[index.x].pos.xyz;
	vec3 AC = vertices.v[index.z].pos.xyz - vertices.v[index.x].pos.xyz;
	hitTriangle.normal = normalize(cross(AB, AC));

	return hitTriangle;
}

HitPoint getHitPoint(uint instanceID, uint primitiveID, vec2 hitAttribute){
	const vec3 barycentricCoords = getBarycentricCoordinates(hitAttribute);
	Triangle triangle = getTriangle(instanceID, primitiveID);

	HitPoint hitPoint;
	hitPoint.pos = triangle.pos[0] * barycentricCoords.x +
					triangle.pos[1] * barycentricCoords.y +
            		triangle.pos[2] * barycentricCoords.z;
	hitPoint.uv = triangle.uv[0] * barycentricCoords.x +
            		triangle.uv[1] * barycentricCoords.y +
            		triangle.uv[2] * barycentricCoords.z;
	hitPoint.color = triangle.color[0] * barycentricCoords.x +
  	          		triangle.color[1] * barycentricCoords.y +
  	           		triangle.color[2] * barycentricCoords.z;
	hitPoint.normal = triangle.normal;
	return hitPoint;
}