#include "constants.h"

// out data
struct Triangle {
	mat3 pos;
	mat3x2 uv;
	vec3 normal;
	mat3x4 color;
	uint tex;
};
struct HitPoint {
	vec3 pos;
	vec3 normal;
	vec2 uv;
	vec4 color;
	uint tex;
};

// buffer with instance data
layout(binding = BINDING_OFFSET_INSTANCE_DATA, set = 0) buffer Instance { ASInstanceData data[]; } iData;
// Buffer with indices and vertices
layout(binding = BINDING_OFFSET_IDX_STATIC, set = 0) buffer Indices_static { uint i[]; } indices_static;
layout(binding = BINDING_OFFSET_XYZ_STATIC, set = 0) buffer Vertices_static { VertexBuffer v[]; } vertices_static;
layout(binding = BINDING_OFFSET_IDX_DYNAMIC, set = 0) buffer Indices_dynamic { uint i[]; } indices_dynamic;
layout(binding = BINDING_OFFSET_XYZ_DYNAMIC, set = 0) buffer Vertices_dynamic { VertexBuffer v[]; } vertices_dynamic;

vec3 getBarycentricCoordinates(vec2 hitAttribute) { return vec3(1.0f - hitAttribute.x - hitAttribute.y, hitAttribute.x, hitAttribute.y); }

Triangle getTriangle(RayPayload rp){
// if(iData.data[rp.instanceID].world) {
// 		ivec3 index = ivec3(indices_static.i[iData.data[rp.instanceID].offsetIDX + 3 * rp.primitiveID], indices_static.i[iData.data[rp.instanceID].offsetIDX + 3 * rp.primitiveID + 1], indices_static.i[iData.data[rp.instanceID].offsetIDX + 3 * rp.primitiveID + 2]);

// 		const vec3 barycentricCoords = vec3(1.0f - rp.barycentric.x - rp.barycentric.y, rp.barycentric.x, rp.barycentric.y);
// 		vec2 uv = 		vertices_static.v[index.x + iData.data[rp.instanceID].offsetXYZ].uv.xy * barycentricCoords.x +
//                   	vertices_static.v[index.y + iData.data[rp.instanceID].offsetXYZ].uv.xy * barycentricCoords.y +
//                   	vertices_static.v[index.z + iData.data[rp.instanceID].offsetXYZ].uv.xy * barycentricCoords.z;
// 		color = global_textureGrad(int(vertices_static.v[index.x + iData.data[rp.instanceID].offsetXYZ].pos.w), uv, tex_coord_x, tex_coord_y);
// 	}

  	uint customIndex = uint(iData.data[rp.instanceID].offsetIDX) + (rp.primitiveID * 3);
	ivec3 index;
	if(!iData.data[rp.instanceID].dynamic) index = (ivec3(indices_static.i[customIndex], indices_static.i[customIndex + 1], indices_static.i[customIndex + 2])) + int(iData.data[rp.instanceID].offsetXYZ);
	else index = (ivec3(indices_dynamic.i[customIndex], indices_dynamic.i[customIndex + 1], indices_dynamic.i[customIndex + 2])) + int(iData.data[rp.instanceID].offsetXYZ);
	//if(!iData.data[rp.instanceID].world) index + int(iData.data[rp.instanceID].offsetXYZ);

	VertexBuffer vData[3];
	if(!iData.data[rp.instanceID].dynamic) {
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
	// hitTriangle.pos[0] = vData[0].pos.xyz;
	// hitTriangle.pos[1] = vData[1].pos.xyz;
	// hitTriangle.pos[2] = vData[2].pos.xyz;
	// Some Entitiys are in Object Space
	hitTriangle.pos[0] = (rp.modelmat * vec4(vData[0].pos.xyz, 1)).xyz;
	hitTriangle.pos[1] = (rp.modelmat * vec4(vData[1].pos.xyz, 1)).xyz;
	hitTriangle.pos[2] = (rp.modelmat * vec4(vData[2].pos.xyz, 1)).xyz;
	// UV
	hitTriangle.uv[0] = vData[0].uv.xy;
	hitTriangle.uv[1] = vData[1].uv.xy;
	hitTriangle.uv[2] = vData[2].uv.xy;
	// Color
	hitTriangle.color[0] = vData[0].color;
	hitTriangle.color[1] = vData[1].color;
	hitTriangle.color[2] = vData[2].color;
	// NORMAL
	//vec3 AB = vData[1].pos.xyz - vData[0].pos.xyz;
	//vec3 AC = vData[2].pos.xyz - vData[0].pos.xyz;
	vec3 AB = hitTriangle.pos[1] - hitTriangle.pos[0];
	vec3 AC = hitTriangle.pos[2] - hitTriangle.pos[0];
	hitTriangle.normal = normalize(cross(AB, AC));

	if(iData.data[rp.instanceID].world) hitTriangle.tex = uint(vertices_static.v[index.x + iData.data[rp.instanceID].offsetXYZ].texIdx0);
	else hitTriangle.tex = iData.data[rp.instanceID].texIdx;

	return hitTriangle;
}

HitPoint getHitPoint(RayPayload rp){
	const vec3 barycentricCoords = getBarycentricCoordinates(rp.barycentric);
	Triangle triangle = getTriangle(rp);

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

	hitPoint.tex = triangle.tex;

	return hitPoint;
}

bool
found_intersection(RayPayload rp)
{
	return rp.instanceID != ~0u;
}
bool
is_light(RayPayload rp)
{
	if(iData.data[rp.instanceID].world) {
		uint customIndex = uint(iData.data[rp.instanceID].offsetIDX) + (rp.primitiveID * 3);
		ivec3 index = (ivec3(indices_static.i[customIndex], indices_static.i[customIndex + 1], indices_static.i[customIndex + 2])) + int(iData.data[rp.instanceID].offsetXYZ);
		return (vertices_static.v[index.x].material & MATERIAL_FLAG_LIGHT) == MATERIAL_FLAG_LIGHT;
	}
	return (iData.data[rp.instanceID].material & MATERIAL_FLAG_LIGHT) == MATERIAL_FLAG_LIGHT;
}
bool
is_mirror(RayPayload rp)
{
	if(iData.data[rp.instanceID].world) {
		uint customIndex = uint(iData.data[rp.instanceID].offsetIDX) + (rp.primitiveID * 3);
		ivec3 index = (ivec3(indices_static.i[customIndex], indices_static.i[customIndex + 1], indices_static.i[customIndex + 2])) + int(iData.data[rp.instanceID].offsetXYZ);
		return (vertices_static.v[index.x].material & MATERIAL_FLAG_MIRROR) == MATERIAL_FLAG_MIRROR;
	}
	else return (iData.data[rp.instanceID].material & MATERIAL_FLAG_MIRROR) == MATERIAL_FLAG_MIRROR;
}
bool
is_glass(RayPayload rp)
{
	return (iData.data[rp.instanceID].material & MATERIAL_KIND_MASK) == MATERIAL_KIND_GLASS;
}

bool
is_player(RayPayload rp)
{
	if(iData.data[rp.instanceID].world) return false;
	return (iData.data[rp.instanceID].material & MATERIAL_FLAG_MASK) == MATERIAL_FLAG_PLAYER_OR_WEAPON;
}