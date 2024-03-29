#include "../constants.h"


// buffer with instance data
layout(binding = BINDING_OFFSET_INSTANCE_DATA, set = 0) buffer Instance { ASInstanceData data[]; } iData;
// Buffer with indices and vertices
layout(binding = BINDING_OFFSET_IDX_WORLD_STATIC, set = 0) buffer Indices_World_static { uint i[]; } indices_world_static;
layout(binding = BINDING_OFFSET_XYZ_WORLD_STATIC, set = 0) buffer Vertices_World_static { VertexBuffer v[]; } vertices_world_static;

layout(binding = BINDING_OFFSET_IDX_WORLD_DYNAMIC_DATA, set = 0) buffer Indices_dynamic_data { uint i[]; } indices_dynamic_data;
layout(binding = BINDING_OFFSET_XYZ_WORLD_DYNAMIC_DATA, set = 0) buffer Vertices_dynamic_data { VertexBuffer v[]; } vertices_dynamic_data;
layout(binding = BINDING_OFFSET_IDX_WORLD_DYNAMIC_AS, set = 0) buffer Indices_dynamic_as { uint i[]; } indices_dynamic_as;
layout(binding = BINDING_OFFSET_XYZ_WORLD_DYNAMIC_AS, set = 0) buffer Vertices_dynamic_as { VertexBuffer v[]; } vertices_dynamic_as;
layout(binding = BINDING_OFFSET_IDX_ENTITY_STATIC, set = 0) buffer Indices_entity_static { uint i[]; } indices_entity_static;
layout(binding = BINDING_OFFSET_XYZ_ENTITY_STATIC, set = 0) buffer Vertices_entity_static { VertexBuffer v[]; } vertices_entity_static;
layout(binding = BINDING_OFFSET_IDX_ENTITY_DYNAMIC, set = 0) buffer Indices_entity_dynamic { uint i[]; } indices_entity_dynamic;
layout(binding = BINDING_OFFSET_XYZ_ENTITY_DYNAMIC, set = 0) buffer Vertices_entity_dynamic { VertexBuffer v[]; } vertices_entity_dynamic;

layout(binding = BINDING_OFFSET_CLUSTER_WORLD_STATIC, set = 0) buffer Cluster_World_static { uint c[]; } cluster_world_static;
layout(binding = BINDING_OFFSET_CLUSTER_WORLD_DYNAMIC_DATA, set = 0) buffer Cluster_World_dynamic_data { uint c[]; } cluster_world_dynamic_data;
layout(binding = BINDING_OFFSET_CLUSTER_WORLD_DYNAMIC_AS, set = 0) buffer Cluster_World_dynamic_as { uint c[]; } cluster_world_dynamic_as;
layout(binding = BINDING_OFFSET_CLUSTER_ENTITY_STATIC, set = 0) buffer Cluster_Entity_static { uint c[]; } cluster_entity_static;

//prev
layout(binding = BINDING_OFFSET_INSTANCE_DATA_PREV, set = 0) buffer InstancePrev { ASInstanceData data[]; } iDataPrev;
layout(binding = BINDING_OFFSET_IDX_WORLD_DYNAMIC_DATA_PREV, set = 0) buffer Indices_dynamic_data_prev { uint i[]; } indices_dynamic_data_prev;
layout(binding = BINDING_OFFSET_XYZ_WORLD_DYNAMIC_DATA_PREV, set = 0) buffer Vertices_dynamic_data_prev { VertexBuffer v[]; } vertices_dynamic_data_prev;
layout(binding = BINDING_OFFSET_IDX_WORLD_DYNAMIC_AS_PREV, set = 0) buffer Indices_dynamic_as_prev { uint i[]; } indices_dynamic_as_prev;
layout(binding = BINDING_OFFSET_XYZ_WORLD_DYNAMIC_AS_PREV, set = 0) buffer Vertices_dynamic_as_prev { VertexBuffer v[]; } vertices_dynamic_as_prev;
layout(binding = BINDING_OFFSET_IDX_ENTITY_DYNAMIC_PREV, set = 0) buffer Indices_entity_dynamic_prev { uint i[]; } indices_entity_dynamic_prev;
layout(binding = BINDING_OFFSET_XYZ_ENTITY_DYNAMIC_PREV, set = 0) buffer Vertices_entity_dynamic_prev { VertexBuffer v[]; } vertices_entity_dynamic_prev;

// layout(binding = BINDING_OFFSET_CLUSTER_WORLD_STATIC_PREV, set = 0) buffer Cluster_World_static_prev { uint c[]; } cluster_world_static_prev;
// layout(binding = BINDING_OFFSET_CLUSTER_WORLD_DYNAMIC_DATA_PREV, set = 0) buffer Cluster_World_dynamic_data_prev { uint c[]; } cluster_world_dynamic_data_prev;
// layout(binding = BINDING_OFFSET_CLUSTER_WORLD_DYNAMIC_AS_PREV, set = 0) buffer Cluster_World_dynamic_as_prev { uint c[]; } cluster_world_dynamic_as_prev;
// layout(binding = BINDING_OFFSET_CLUSTER_ENTITY_STATIC_PREV, set = 0) buffer Cluster_Entity_static_prev { uint c[]; } cluster_entity_static_prev;

vec3 getBarycentricCoordinates(vec2 hitAttribute) { return vec3(1.0f - hitAttribute.x - hitAttribute.y, hitAttribute.x, hitAttribute.y); }

vec4 unpackColor(in uint color) {
	return vec4(
		color & 0xff,
		(color & (0xff << 8)) >> 8,
		(color & (0xff << 16)) >> 16,
		(color & (0xff << 24)) >> 24
	);
}

ivec3 getVertexData(in RayPayload rp, out VertexBuffer vData[3]){
	uint customIndex = uint(iData.data[rp.instanceID].offsetIDX) + (rp.primitiveID * 3);
	ivec3 index;
	if(iData.data[rp.instanceID].type == BAS_WORLD_STATIC) index = (ivec3(indices_world_static.i[customIndex], indices_world_static.i[customIndex + 1], indices_world_static.i[customIndex + 2])) + int(iData.data[rp.instanceID].offsetXYZ);
	else if(iData.data[rp.instanceID].type == BAS_WORLD_DYNAMIC_DATA)  index = (ivec3(indices_dynamic_data.i[customIndex], indices_dynamic_data.i[customIndex + 1], indices_dynamic_data.i[customIndex + 2])) + int(iData.data[rp.instanceID].offsetXYZ);
	else if(iData.data[rp.instanceID].type == BAS_WORLD_DYNAMIC_AS)  index = (ivec3(indices_dynamic_as.i[customIndex], indices_dynamic_as.i[customIndex + 1], indices_dynamic_as.i[customIndex + 2])) + int(iData.data[rp.instanceID].offsetXYZ);
	else if(iData.data[rp.instanceID].type == BAS_ENTITY_STATIC)  index = (ivec3(indices_entity_static.i[customIndex], indices_entity_static.i[customIndex + 1], indices_entity_static.i[customIndex + 2])) + int(iData.data[rp.instanceID].offsetXYZ);
	else if(iData.data[rp.instanceID].type == BAS_ENTITY_DYNAMIC)  index = (ivec3(indices_entity_dynamic.i[customIndex], indices_entity_dynamic.i[customIndex + 1], indices_entity_dynamic.i[customIndex + 2])) + int(iData.data[rp.instanceID].offsetXYZ);

	if(iData.data[rp.instanceID].type == BAS_WORLD_STATIC){
		vData[0] = vertices_world_static.v[index.x];
		vData[1] = vertices_world_static.v[index.y];
		vData[2] = vertices_world_static.v[index.z];
	}else if(iData.data[rp.instanceID].type == BAS_WORLD_DYNAMIC_DATA){
		vData[0] = vertices_dynamic_data.v[index.x];
		vData[1] = vertices_dynamic_data.v[index.y];
		vData[2] = vertices_dynamic_data.v[index.z];
	}else if(iData.data[rp.instanceID].type == BAS_WORLD_DYNAMIC_AS){
		vData[0] = vertices_dynamic_as.v[index.x];
		vData[1] = vertices_dynamic_as.v[index.y];
		vData[2] = vertices_dynamic_as.v[index.z];
	}else if(iData.data[rp.instanceID].type == BAS_ENTITY_STATIC){
		vData[0] = vertices_entity_static.v[index.x];
		vData[1] = vertices_entity_static.v[index.y];
		vData[2] = vertices_entity_static.v[index.z];
	}else if(iData.data[rp.instanceID].type == BAS_ENTITY_DYNAMIC){
		vData[0] = vertices_entity_dynamic.v[index.x];
		vData[1] = vertices_entity_dynamic.v[index.y];
		vData[2] = vertices_entity_dynamic.v[index.z];
	}
	return index;
}

Triangle getTriangle(in RayPayload rp){
	VertexBuffer vData[3];
	ivec3 index = getVertexData(rp, vData);

	Triangle hitTriangle;
	// POS
	// Some Entitiys are in Object Space
	hitTriangle.pos[0] = (vec4(vData[0].pos.xyz, 1) * iData.data[rp.instanceID].modelmat).xyz;
	hitTriangle.pos[1] = (vec4(vData[1].pos.xyz, 1) * iData.data[rp.instanceID].modelmat).xyz;
	hitTriangle.pos[2] = (vec4(vData[2].pos.xyz, 1) * iData.data[rp.instanceID].modelmat).xyz;
	// Color
	hitTriangle.color0[0] = unpackColor(vData[0].color0);
	hitTriangle.color0[1] = unpackColor(vData[1].color0);
	hitTriangle.color0[2] = unpackColor(vData[2].color0);

	hitTriangle.color1[0] = unpackColor(vData[0].color1);
	hitTriangle.color1[1] = unpackColor(vData[1].color1);
	hitTriangle.color1[2] = unpackColor(vData[2].color1);

	hitTriangle.color2[0] = unpackColor(vData[0].color2);
	hitTriangle.color2[1] = unpackColor(vData[1].color2);
	hitTriangle.color2[2] = unpackColor(vData[2].color2);
	
	hitTriangle.color3[0] = unpackColor(vData[0].color3);
	hitTriangle.color3[1] = unpackColor(vData[1].color3);
	hitTriangle.color3[2] = unpackColor(vData[2].color3);

	// UV
	hitTriangle.uv0[0] = vData[0].uv0.xy;
	hitTriangle.uv0[1] = vData[1].uv0.xy;
	hitTriangle.uv0[2] = vData[2].uv0.xy;

	hitTriangle.uv1[0] = vData[0].uv1.xy;
	hitTriangle.uv1[1] = vData[1].uv1.xy;
	hitTriangle.uv1[2] = vData[2].uv1.xy;

	hitTriangle.uv2[0] = vData[0].uv2.xy;
	hitTriangle.uv2[1] = vData[1].uv2.xy;
	hitTriangle.uv2[2] = vData[2].uv2.xy;

	hitTriangle.uv3[0] = vData[0].uv3.xy;
	hitTriangle.uv3[1] = vData[1].uv3.xy;
	hitTriangle.uv3[2] = vData[2].uv3.xy;
	// NORMAL
	// vec3 AB = hitTriangle.pos[1] - hitTriangle.pos[0];
	// vec3 AC = hitTriangle.pos[2] - hitTriangle.pos[0];
	// hitTriangle.normal = normalize(cross(AC, AB));
	const vec3 barycentricCoords = getBarycentricCoordinates(rp.barycentric);
	hitTriangle.normal = vData[0].normal.xyz * barycentricCoords.x +
						vData[1].normal.xyz * barycentricCoords.y +
           				vData[2].normal.xyz * barycentricCoords.z;
	hitTriangle.normal = (vec4(hitTriangle.normal, 0) * iData.data[rp.instanceID].modelmat).xyz;

	uint idx_c;
	switch(iData.data[rp.instanceID].type){
		case BAS_WORLD_STATIC:
			hitTriangle.tex0 = (vertices_world_static.v[index.x].texIdx0);
			hitTriangle.tex1 = (vertices_world_static.v[index.x].texIdx1);
			// cluster
			idx_c = uint(iData.data[rp.instanceID].offsetIDX) + (rp.primitiveID);
			hitTriangle.cluster = cluster_world_static.c[idx_c];
			// material
			hitTriangle.material = vertices_world_static.v[index.x].material;
			break;
		case BAS_WORLD_DYNAMIC_DATA:
			hitTriangle.tex0 = (vertices_dynamic_data.v[index.x].texIdx0);
			hitTriangle.tex1 = (vertices_dynamic_data.v[index.x].texIdx1);
			// cluster
			idx_c = uint(iData.data[rp.instanceID].offsetIDX) + (rp.primitiveID);
			hitTriangle.cluster = cluster_world_dynamic_data.c[idx_c];
			// material
			hitTriangle.material = vertices_dynamic_data.v[index.x].material;
			break;
		case BAS_WORLD_DYNAMIC_AS:
			hitTriangle.tex0 = (vertices_dynamic_as.v[index.x].texIdx0);
			hitTriangle.tex1 = (vertices_dynamic_as.v[index.x].texIdx1);
			// cluster
			idx_c = uint(iData.data[rp.instanceID].offsetIDX) + (rp.primitiveID);
			hitTriangle.cluster = cluster_world_dynamic_as.c[idx_c];
			// material
			hitTriangle.material = vertices_dynamic_as.v[index.x].material;
			break;
		case BAS_ENTITY_STATIC:
			hitTriangle.tex0 = (iData.data[rp.instanceID].texIdx0);
			hitTriangle.tex1 = (iData.data[rp.instanceID].texIdx1);
			// cluster
			if(iData.data[rp.instanceID].isBrushModel){
				idx_c = uint(iData.data[rp.instanceID].offsetIDX) + (rp.primitiveID);
				hitTriangle.cluster = cluster_entity_static.c[idx_c];
			}
			else hitTriangle.cluster = iData.data[rp.instanceID].cluster;
			// material
			hitTriangle.material = vertices_entity_static.v[index.x].material;
			break;
		case BAS_ENTITY_DYNAMIC:
			hitTriangle.tex0 = (iData.data[rp.instanceID].texIdx0);
			hitTriangle.tex1 = (iData.data[rp.instanceID].texIdx1);
			// cluster
			hitTriangle.cluster = iData.data[rp.instanceID].cluster;
			//hitTriangle.cluster = vData[1].cluster;
			// material
			hitTriangle.material = vertices_entity_dynamic.v[index.x].material;
			break;
		default:
			hitTriangle.tex0 = (iData.data[rp.instanceID].texIdx0);
			hitTriangle.tex1 = (iData.data[rp.instanceID].texIdx1);
			// cluster
			hitTriangle.cluster = vData[1].cluster;
			// material
			hitTriangle.material = 0;
	}

	return hitTriangle;
}

HitPoint getHitPoint(in RayPayload rp){
	const vec3 barycentricCoords = getBarycentricCoordinates(rp.barycentric);
	Triangle triangle = getTriangle(rp);

	HitPoint hitPoint;
	hitPoint.pos = triangle.pos[0] * barycentricCoords.x +
					triangle.pos[1] * barycentricCoords.y +
            		triangle.pos[2] * barycentricCoords.z;
	hitPoint.color0 = triangle.color0[0] * barycentricCoords.x +
  	          		triangle.color0[1] * barycentricCoords.y +
  	           		triangle.color0[2] * barycentricCoords.z;
	hitPoint.color1 = triangle.color1[0] * barycentricCoords.x +
  	          		triangle.color1[1] * barycentricCoords.y +
  	           		triangle.color1[2] * barycentricCoords.z;
	hitPoint.color2 = triangle.color2[0] * barycentricCoords.x +
  	          		triangle.color2[1] * barycentricCoords.y +
  	           		triangle.color2[2] * barycentricCoords.z;
	hitPoint.color3 = triangle.color3[0] * barycentricCoords.x +
  	          		triangle.color3[1] * barycentricCoords.y +
  	           		triangle.color3[2] * barycentricCoords.z;
	hitPoint.uv0 = triangle.uv0[0] * barycentricCoords.x +
            		triangle.uv0[1] * barycentricCoords.y +
            		triangle.uv0[2] * barycentricCoords.z;
	hitPoint.uv1 = triangle.uv1[0] * barycentricCoords.x +
            		triangle.uv1[1] * barycentricCoords.y +
            		triangle.uv1[2] * barycentricCoords.z;
	hitPoint.uv2 = triangle.uv2[0] * barycentricCoords.x +
            		triangle.uv2[1] * barycentricCoords.y +
            		triangle.uv2[2] * barycentricCoords.z;
	hitPoint.uv3 = triangle.uv3[0] * barycentricCoords.x +
            		triangle.uv3[1] * barycentricCoords.y +
            		triangle.uv3[2] * barycentricCoords.z;
	hitPoint.normal = triangle.normal;

	hitPoint.tex0 = triangle.tex0;
	hitPoint.tex1 = triangle.tex1;

	hitPoint.cluster = triangle.cluster;
	hitPoint.material = triangle.material;
	return hitPoint;
}

ivec3 getVertexDataPrev(in RayPayload rp, out VertexBuffer vData[3]){
	uint customIndex = uint(iDataPrev.data[rp.instanceID].offsetIDX) + (rp.primitiveID * 3);
	ivec3 index;
	if(iDataPrev.data[rp.instanceID].type == BAS_WORLD_STATIC) index = (ivec3(indices_world_static.i[customIndex], indices_world_static.i[customIndex + 1], indices_world_static.i[customIndex + 2])) + int(iDataPrev.data[rp.instanceID].offsetXYZ);
	else if(iDataPrev.data[rp.instanceID].type == BAS_WORLD_DYNAMIC_DATA)  index = (ivec3(indices_dynamic_data_prev.i[customIndex], indices_dynamic_data_prev.i[customIndex + 1], indices_dynamic_data_prev.i[customIndex + 2])) + int(iDataPrev.data[rp.instanceID].offsetXYZ);
	else if(iDataPrev.data[rp.instanceID].type == BAS_WORLD_DYNAMIC_AS)  index = (ivec3(indices_dynamic_as_prev.i[customIndex], indices_dynamic_as_prev.i[customIndex + 1], indices_dynamic_as_prev.i[customIndex + 2])) + int(iDataPrev.data[rp.instanceID].offsetXYZ);
	else if(iDataPrev.data[rp.instanceID].type == BAS_ENTITY_STATIC)  index = (ivec3(indices_entity_static.i[customIndex], indices_entity_static.i[customIndex + 1], indices_entity_static.i[customIndex + 2])) + int(iDataPrev.data[rp.instanceID].offsetXYZ);
	else if(iDataPrev.data[rp.instanceID].type == BAS_ENTITY_DYNAMIC)  index = (ivec3(indices_entity_dynamic_prev.i[customIndex], indices_entity_dynamic_prev.i[customIndex + 1], indices_entity_dynamic_prev.i[customIndex + 2])) + int(iDataPrev.data[rp.instanceID].offsetXYZ);

	if(iDataPrev.data[rp.instanceID].type == BAS_WORLD_STATIC){
		vData[0] = vertices_world_static.v[index.x];
		vData[1] = vertices_world_static.v[index.y];
		vData[2] = vertices_world_static.v[index.z];
	}else if(iDataPrev.data[rp.instanceID].type == BAS_WORLD_DYNAMIC_DATA){
		vData[0] = vertices_dynamic_data_prev.v[index.x];
		vData[1] = vertices_dynamic_data_prev.v[index.y];
		vData[2] = vertices_dynamic_data_prev.v[index.z];
	}else if(iDataPrev.data[rp.instanceID].type == BAS_WORLD_DYNAMIC_AS){
		vData[0] = vertices_dynamic_as_prev.v[index.x];
		vData[1] = vertices_dynamic_as_prev.v[index.y];
		vData[2] = vertices_dynamic_as_prev.v[index.z];
	}else if(iDataPrev.data[rp.instanceID].type == BAS_ENTITY_STATIC){
		vData[0] = vertices_entity_static.v[index.x];
		vData[1] = vertices_entity_static.v[index.y];
		vData[2] = vertices_entity_static.v[index.z];
	}else if(iDataPrev.data[rp.instanceID].type == BAS_ENTITY_DYNAMIC){
		vData[0] = vertices_entity_dynamic_prev.v[index.x];
		vData[1] = vertices_entity_dynamic_prev.v[index.y];
		vData[2] = vertices_entity_dynamic_prev.v[index.z];
	}
	return index;
}

Triangle getTrianglePrev(in RayPayload rp){
	if(iData.data[rp.instanceID].type == BAS_ENTITY_STATIC) rp.instanceID = iData.data[rp.instanceID].prevInstanceID;

	VertexBuffer vData[3];
	ivec3 index = getVertexDataPrev(rp, vData);

	Triangle hitTriangle;
	// POS
	// Some Entitiys are in Object Space
	hitTriangle.pos[0] = (vec4(vData[0].pos.xyz, 1) * iDataPrev.data[rp.instanceID].modelmat).xyz;
	hitTriangle.pos[1] = (vec4(vData[1].pos.xyz, 1) * iDataPrev.data[rp.instanceID].modelmat).xyz;
	hitTriangle.pos[2] = (vec4(vData[2].pos.xyz, 1) * iDataPrev.data[rp.instanceID].modelmat).xyz;

	return hitTriangle;
}

HitPoint getHitPointPrev(in RayPayload rp){
	const vec3 barycentricCoords = getBarycentricCoordinates(rp.barycentric);
	Triangle triangle = getTrianglePrev(rp);

	HitPoint hitPoint;
	hitPoint.pos = triangle.pos[0] * barycentricCoords.x +
					triangle.pos[1] * barycentricCoords.y +
            		triangle.pos[2] * barycentricCoords.z;
	return hitPoint;
}

uint getInstanceType(in RayPayload rp){
	return iData.data[rp.instanceID].type;
}
uint getPrevInstanceID(RayPayload rp){
	return iData.data[rp.instanceID].prevInstanceID;
}

uint getCluster(RayPayload rp){
	VertexBuffer vData[3];
	ivec3 index = getVertexDataPrev(rp, vData);

	uint idx_c;
	switch(iData.data[rp.instanceID].type){
		case BAS_WORLD_STATIC:
			idx_c = uint(iData.data[rp.instanceID].offsetIDX) + (rp.primitiveID);
			return cluster_world_static.c[idx_c];
		case BAS_WORLD_DYNAMIC_DATA:
			idx_c = uint(iData.data[rp.instanceID].offsetIDX) + (rp.primitiveID);
			return cluster_world_dynamic_data.c[idx_c];
		case BAS_WORLD_DYNAMIC_AS:
			idx_c = uint(iData.data[rp.instanceID].offsetIDX) + (rp.primitiveID);
			return cluster_world_dynamic_as.c[idx_c];
		case BAS_ENTITY_STATIC:
			if(iData.data[rp.instanceID].isBrushModel){
				idx_c = uint(iData.data[rp.instanceID].offsetIDX) + (rp.primitiveID);
				return cluster_entity_static.c[idx_c];
			}
			else return iData.data[rp.instanceID].cluster;
		case BAS_ENTITY_DYNAMIC:
			return vData[1].cluster;
		default:
			return vData[1].cluster;
	}
}

uint getPrevCluster(RayPayload rp){
	rp.instanceID = iData.data[rp.instanceID].prevInstanceID;

	VertexBuffer vData[3];
	ivec3 index = getVertexDataPrev(rp, vData);

	uint idx_c;
	switch(iDataPrev.data[rp.instanceID].type){
		case BAS_WORLD_STATIC:
			idx_c = uint(iDataPrev.data[rp.instanceID].offsetIDX) + (rp.primitiveID);
			return cluster_world_static.c[idx_c];
		case BAS_WORLD_DYNAMIC_DATA:
			idx_c = uint(iDataPrev.data[rp.instanceID].offsetIDX) + (rp.primitiveID);
			return cluster_world_dynamic_data.c[idx_c];
		case BAS_WORLD_DYNAMIC_AS:
			idx_c = uint(iDataPrev.data[rp.instanceID].offsetIDX) + (rp.primitiveID);
			return cluster_world_dynamic_as.c[idx_c];
		case BAS_ENTITY_STATIC:
			if(iDataPrev.data[rp.instanceID].isBrushModel){
				idx_c = uint(iDataPrev.data[rp.instanceID].offsetIDX) + (rp.primitiveID);
				return cluster_entity_static.c[idx_c];
			}
			else return iDataPrev.data[rp.instanceID].cluster;
		case BAS_ENTITY_DYNAMIC:
			return vData[1].cluster;
		default:
			return vData[1].cluster;
	}
}

vec3 getPrevPos(RayPayload rp){
	rp.instanceID = iData.data[rp.instanceID].prevInstanceID;

	VertexBuffer vData[3];
	ivec3 index = getVertexDataPrev(rp, vData);

	const vec3 barycentricCoords = getBarycentricCoordinates(rp.barycentric);
	// POS
	// Some Entitiys are in Object Space
	vec3 pos[3];
	pos[0] = (vec4(vData[0].pos.xyz, 1) * iDataPrev.data[rp.instanceID].modelmat).xyz;
	pos[1] = (vec4(vData[1].pos.xyz, 1) * iDataPrev.data[rp.instanceID].modelmat).xyz;
	pos[2] = (vec4(vData[2].pos.xyz, 1) * iDataPrev.data[rp.instanceID].modelmat).xyz;
	return pos[0] * barycentricCoords.x + pos[1] * barycentricCoords.y + pos[2] * barycentricCoords.z;
}

uint 
get_material(in RayPayload rp){
	uint customIndex = uint(iData.data[rp.instanceID].offsetIDX) + (rp.primitiveID * 3);
	ivec3 index;
	switch(iData.data[rp.instanceID].type){
		case BAS_WORLD_STATIC:
			index = (ivec3(indices_world_static.i[customIndex], indices_world_static.i[customIndex + 1], indices_world_static.i[customIndex + 2])) + int(iData.data[rp.instanceID].offsetXYZ);
			return vertices_world_static.v[index.x].material;
		case BAS_WORLD_DYNAMIC_DATA:
			index = (ivec3(indices_dynamic_data.i[customIndex], indices_dynamic_data.i[customIndex + 1], indices_dynamic_data.i[customIndex + 2])) + int(iData.data[rp.instanceID].offsetXYZ);
			return vertices_dynamic_data.v[index.x].material;
		case BAS_WORLD_DYNAMIC_AS:
			index = (ivec3(indices_dynamic_as.i[customIndex], indices_dynamic_as.i[customIndex + 1], indices_dynamic_as.i[customIndex + 2])) + int(iData.data[rp.instanceID].offsetXYZ);
			return vertices_dynamic_as.v[index.x].material;
		case BAS_ENTITY_STATIC:
			index = (ivec3(indices_entity_static.i[customIndex], indices_entity_static.i[customIndex + 1], indices_entity_static.i[customIndex + 2])) + int(iData.data[rp.instanceID].offsetXYZ);
			return vertices_entity_static.v[index.x].material;
		case BAS_ENTITY_DYNAMIC:
			index = (ivec3(indices_entity_dynamic.i[customIndex], indices_entity_dynamic.i[customIndex + 1], indices_entity_dynamic.i[customIndex + 2])) + int(iData.data[rp.instanceID].offsetXYZ);
			return vertices_entity_dynamic.v[index.x].material;
		default:
			return 0;
	}
}

bool
found_intersection(in RayPayload rp)
{
	return rp.instanceID != ~0u;
}
bool
is_light(RayPayload rp)
{
	return (get_material(rp) & MATERIAL_FLAG_LIGHT) == MATERIAL_FLAG_LIGHT;
	
}
bool
is_mirror(RayPayload rp)
{
	return (get_material(rp) & MATERIAL_FLAG_MIRROR) == MATERIAL_FLAG_MIRROR;
}
bool
is_glass(RayPayload rp)
{
	return (get_material(rp) & MATERIAL_KIND_MASK) == MATERIAL_KIND_GLASS;
}
bool
is_see_through(RayPayload rp)
{
	return (get_material(rp) & MATERIAL_FLAG_SEE_THROUGH) == MATERIAL_FLAG_SEE_THROUGH;
}

bool
is_player(RayPayload rp) // for shadows etc, so the third person model does not cast shadows on the first person weapon
{
	if(iData.data[rp.instanceID].type == BAS_ENTITY_STATIC) return iData.data[rp.instanceID].isPlayer;
	else return false;
	//return (get_material(rp) & MATERIAL_FLAG_PLAYER_OR_WEAPON) == MATERIAL_FLAG_PLAYER_OR_WEAPON;
}

bool
isSeeThrough(in uint material) {
	return ((material & MATERIAL_FLAG_SEE_THROUGH) == MATERIAL_FLAG_SEE_THROUGH);
}
bool
isSeeThroughAdd(in uint material) {
	return ((material & MATERIAL_FLAG_SEE_THROUGH_ADD) == MATERIAL_FLAG_SEE_THROUGH_ADD);
}
bool
isSeeThroughNoAlpha(in uint material) {
	return ((material & MATERIAL_FLAG_SEE_THROUGH_NO_ALPHA) == MATERIAL_FLAG_SEE_THROUGH_NO_ALPHA);
}
bool
isLight(in uint material) {
	return ((material & MATERIAL_FLAG_LIGHT) == MATERIAL_FLAG_LIGHT);
}
bool
isGlass(in uint material) {
	return ((material & MATERIAL_KIND_MASK) == MATERIAL_KIND_GLASS);
}
bool
isMirror(in uint material) {
	return ((material & MATERIAL_FLAG_MIRROR) == MATERIAL_FLAG_MIRROR);
}
bool
isWater(in uint material) {
	return ((material & MATERIAL_KIND_MASK) == MATERIAL_KIND_WATER);
}
bool
isPlayer(in uint material) {
	return ((material & MATERIAL_FLAG_PLAYER_OR_WEAPON) == MATERIAL_FLAG_PLAYER_OR_WEAPON);
}
bool
isSky(in uint material) {
	return (material == MATERIAL_KIND_SKY);
}
bool
isIgnoreLuminance(in uint material) {
	return ((material & MATERIAL_FLAG_IGNORE_LUMINANCE) == MATERIAL_FLAG_IGNORE_LUMINANCE);
}
