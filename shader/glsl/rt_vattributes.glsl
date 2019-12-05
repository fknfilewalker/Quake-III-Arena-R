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
layout(binding = BINDING_OFFSET_INSTANCE_DATA, set = 0) buffer Instance { ASInstanceData data[]; } iData;

// Buffer with indices and vertices
layout(binding = BINDING_OFFSET_IDX_STATIC, set = 0) buffer Indices_static { uint i[]; } indices_static;
layout(binding = BINDING_OFFSET_XYZ_STATIC, set = 0) buffer Vertices_static { VertexBuffer v[]; } vertices_static;
layout(binding = BINDING_OFFSET_IDX_DYNAMIC, set = 0) buffer Indices_dynamic { uint i[]; } indices_dynamic;
layout(binding = BINDING_OFFSET_XYZ_DYNAMIC, set = 0) buffer Vertices_dynamic { VertexBuffer v[]; } vertices_dynamic;

vec3 getBarycentricCoordinates(vec2 hitAttribute) { return vec3(1.0f - hitAttribute.x - hitAttribute.y, hitAttribute.x, hitAttribute.y); }

Triangle getTriangle(uint instanceID, uint primitiveID){
  	uint customIndex = uint(iData.data[instanceID].offsetIDX) + (primitiveID * 3);
	ivec3 index;
	if(!iData.data[instanceID].dynamic) index = (ivec3(indices_static.i[customIndex], indices_static.i[customIndex + 1], indices_static.i[customIndex + 2])) + int(iData.data[instanceID].offsetXYZ);
	else index = (ivec3(indices_dynamic.i[customIndex], indices_dynamic.i[customIndex + 1], indices_dynamic.i[customIndex + 2])) + int(iData.data[instanceID].offsetXYZ);

	VertexBuffer vData[3];
	if(!iData.data[instanceID].dynamic) {
		vData[0] = vertices_static.v[index.x];
		vData[1] = vertices_static.v[index.y];
		vData[2] = vertices_static.v[index.z];
	}
	else {
		vData[0] = vertices_dynamic.v[index.x];
		vData[1] = vertices_dynamic.v[index.y];
		vData[2] = vertices_dynamic.v[index.z];
	}

	Triangle hitTriangle;
	// POS
	hitTriangle.pos[0] = vData[0].pos.xyz;
	hitTriangle.pos[1] = vData[1].pos.xyz;
	hitTriangle.pos[2] = vData[2].pos.xyz;
	//hitTriangle.pos[0] = (mat4x3(iData.data[instanceID].modelMat) * vec4(vData[0].pos.xyz, 1)).xyz;
	//hitTriangle.pos[1] = (mat4x3(iData.data[instanceID].modelMat) * vec4(vData[1].pos.xyz, 1)).xyz;
	//hitTriangle.pos[2] = (mat4x3(iData.data[instanceID].modelMat) * vec4(vData[2].pos.xyz, 1)).xyz;
	// UV
	hitTriangle.uv[0] = vData[0].uv.xy;
	hitTriangle.uv[1] = vData[1].uv.xy;
	hitTriangle.uv[2] = vData[2].uv.xy;
	// Color
	hitTriangle.color[0] = vData[0].color;
	hitTriangle.color[1] = vData[1].color;
	hitTriangle.color[2] = vData[2].color;
	// NORMAL
	vec3 AB = vData[1].pos.xyz - vData[0].pos.xyz;
	vec3 AC = vData[2].pos.xyz - vData[0].pos.xyz;
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