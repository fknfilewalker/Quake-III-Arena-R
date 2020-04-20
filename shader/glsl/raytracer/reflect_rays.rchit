#version 460
#extension GL_NV_ray_tracing : require
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : enable
#include "../constants.h"
#include "defines.glsl"
#include "globalTexture.glsl"
#include "vertexData.glsl"
#include "traceRay.glsl"

hitAttributeNV vec2 hitAttribute;

layout(location = PAYLOAD_REFLECT) rayPayloadInNV RayPayloadReflect rrp;

void main()
{
	if(rrp.depth > 5) return;
	
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
	rrp.depth = depth;
	
	//rrp.color += vec4(0.2, 0.2, 0.2, 1);

	if(isGlass(hp.material)){
		// if texture not see through enough then do not trace further
		if(tex.w > 0.5) {
			rrp.pos = hp.pos;
			rrp.normal = hp.normal;
			rrp.cluster = hp.cluster;
			rrp.material = hp.material;
			rrp.color = tex;
			return;
		}

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
		float ratio = 0.2;//0.2;
		vec4 albedo = ratio * albedoReflection + (1 - ratio) * albedoRefraction;
		
		vec4 color = tex * tex.w + albedo * (1-tex.w);
		rrp.color = color;	
	} else {
		rrp.color += tex;
		rrp.pos = hp.pos;
		rrp.normal = hp.normal;
		rrp.cluster = hp.cluster;
		rrp.material = hp.material;
	}
}