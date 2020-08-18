#version 460
#extension GL_NV_ray_tracing : require
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : enable
#include "../constants.h"
#include "defines.glsl"
#include "globalTexture.glsl"
#include "vertexData.glsl"
#include "traceRay.glsl"
#include "rt_Helper.glsl"

vec3 reflect_point_vs_plane(vec3 plane_pt, vec3 plane_normal, vec3 point)
{
	return point - 2 * plane_normal * dot(plane_normal, point - plane_pt);
}

float ray_distance_to_plane(vec3 plane_pt, vec3 plane_normal, vec3 ray_origin, vec3 ray_direction)
{
	return dot(plane_pt - ray_origin, plane_normal) / dot(ray_direction, plane_normal);
}

hitAttributeNV vec2 hitAttribute;

layout(binding = BINDING_OFFSET_GLOBAL_UBO, set = 0) uniform global_ubo
{
	GlobalUbo ubo;
};

layout(location = PAYLOAD_REFLECT) rayPayloadInNV RayPayloadReflect rrp;

#define PRIMARY_RAY_T_MAX (10000.0)
Ray
getRay(vec2 pos_cs)
{
	vec4 v_near = ubo.inverseViewMat * ubo.inverseProjMat * vec4(pos_cs, -1, 1.0);
	vec4 v_far  = ubo.inverseViewMat * ubo.inverseProjMat * vec4(pos_cs,  1, 1.0);
	v_near /= v_near.w;
	v_far  /= v_far.w;

	Ray ray;
	ray.origin = v_near.xyz;
	ray.direction = normalize(v_far.xyz - v_near.xyz);
	ray.t_min = 0.01;
	ray.t_max = PRIMARY_RAY_T_MAX;
	return ray;
}


void main()
{
	//if(rrp.depth == 1) return;
	if(rrp.depth > 4) return;
	
	RayPayload rp;
	rp.barycentric = hitAttribute;
  	rp.instanceID = gl_InstanceID;
	rp.primitiveID = gl_PrimitiveID;
	rp.hit_distance = gl_RayTmaxNV;

	HitPoint hp = getHitPoint(rp);
	Triangle triangle = getTriangle(rp);
	vec4 tex = getTextureWithLod(hp,0);
	//rrp.color += tex;
	uint depth = rrp.depth + 1;
	rrp.prevPos = getPrevPos(rp);
	rrp.depth = depth;
	rrp.object = vec4(rp.barycentric, uintBitsToFloat(rp.instanceID), uintBitsToFloat(rp.primitiveID));
	
	//rrp.color += vec4(0.2, 0.2, 0.2, 1);

	if(isGlass(hp.material) /*|| isWater(hp.material)*/){
		// if texture (alpha) not see through enough then do not trace further
		if(tex.w > 0.5) {
			rrp.pos = hp.pos;
			rrp.normal = normalize(hp.normal);
			rrp.cluster = hp.cluster;
			rrp.material = hp.material;
			rrp.color = tex;
			return;
		}

		vec3 originalDir = normalize(gl_WorldRayDirectionNV);
		vec3 N = normalize(hp.normal);
		float n1 = 1.5, n2 = 1.0, ndotr = dot(originalDir, N);  
		if( ndotr > 0.0f ) {
			N = -N;
		}

		vec4 albedoReflection;
		vec4 albedoRefraction;
		rrp.color = vec4(0);
		rrp.depth = depth;
		traceReflect(hp.pos, originalDir, N);
		albedoReflection = rrp.color;

		rrp.color = vec4(0);
		rrp.depth = depth;
		traceRefract(hp.pos, originalDir, N, n1, n2);
		albedoRefraction = rrp.color;
		
		float r0 = (n1-n2)/(n1+n2); r0 *= r0;
		float fresnel = r0 + (1.-r0) * pow(1.0-abs(ndotr),5.);
		float ratio = fresnel*2;//0.2;
		vec4 albedo = ratio * albedoReflection + (1 - ratio) * albedoRefraction;
		
		vec4 color = tex * tex.w + albedo * (1-tex.w);
		rrp.color = color;	
	} 
	else 
	if(isWater(hp.material)){
		// if texture (alpha) not see through enough then do not trace further
	

		vec3 originalDir = normalize(gl_WorldRayDirectionNV);
		vec3 N = normalize(hp.normal);
		float n1 = 1.3, n2 = 1.0, ndotr = dot(originalDir, N);  
		if( ndotr > 0.0f ) {
			N = -N;
		}

		vec4 albedoReflection;
		vec4 albedoRefraction;
		rrp.color = vec4(0);
		rrp.depth = depth;
		traceReflect(hp.pos, originalDir, N);
		albedoReflection = rrp.color;

		rrp.color = vec4(0);
		rrp.depth = depth;
		traceRefract(hp.pos, originalDir, N, n1, n2);
		albedoRefraction = rrp.color;
		
		float r0 = (n1-n2)/(n1+n2); r0 *= r0;
		float fresnel = r0 + (1.-r0) * pow(1.0-abs(ndotr),5.);
		float ratio = fresnel*2;//0.2;
		vec4 albedo = ratio * albedoReflection + (1 - ratio) * albedoRefraction;
		
		//vec4 color = tex * tex.w + albedo * (1-tex.w);
		vec4 color = albedo * tex;
		rrp.color = color;	
	} 
	else
	if (isSeeThrough(hp.material)){
		if(tex.w > 0.5) {
			rrp.pos = hp.pos;
			rrp.normal = normalize(hp.normal);
			rrp.cluster = hp.cluster;
			rrp.material = hp.material;
			rrp.color = tex;
			return;
		}

		vec3 originalDir = normalize(gl_WorldRayDirectionNV);
		vec3 N = normalize(hp.normal);
		vec4 albedo;
		rrp.color = vec4(0);
		rrp.depth = depth;
		traceStraight(hp.pos, originalDir, N);
		albedo = rrp.color;

		vec4 color = tex * tex.w + albedo * (1-tex.w);
		rrp.color = color;
		rrp.color.w = 0;
	} 
	else 
	if(isMirror(hp.material)){
		// if texture (alpha) not see through enough then do not trace further
		if(tex.w > 0.5) {
			rrp.pos = hp.pos;
			rrp.normal = normalize(hp.normal);
			rrp.cluster = hp.cluster;
			rrp.material = hp.material;
			rrp.color = tex;
			return;
		}

		vec3 originalDir = normalize(gl_WorldRayDirectionNV);
		vec3 N = normalize(hp.normal);
		

		vec4 albedoReflection;
		rrp.color = vec4(0);
		rrp.depth = depth;
		traceReflect(hp.pos, originalDir, N);
		albedoReflection = rrp.color;

		vec4 mirrorColor = tex;
		mirrorColor = 1 - vec4(mirrorColor/1);
		vec4 color = vec4(mirrorColor.xyz * rrp.color.xyz,1);
		rrp.color = color;	
		
	}
	else {
		rrp.color += tex;
		rrp.pos = hp.pos;
		rrp.normal = normalize(hp.normal);
		rrp.cluster = hp.cluster;
		rrp.material = hp.material;
	}
}