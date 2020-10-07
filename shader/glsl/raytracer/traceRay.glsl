layout(location = PAYLOAD_BRDF) rayPayloadNV RayPayload rp;
layout(location = PAYLOAD_SHADOW) rayPayloadNV ShadowRayPayload rp_shadow;

layout(binding = BINDING_OFFSET_AS, set = 0) uniform accelerationStructureNV topLevelAS;



void trace_ray(Ray ray, uint cullMask)
{
	const uint rayFlags = gl_RayFlagsNoneNV;//gl_RayFlagsCullFrontFacingTrianglesNV;//gl_RayFlagsOpaqueNV;// | gl_RayFlagsCullBackFacingTrianglesNV;
	//const uint rayFlags = gl_RayFlagsOpaqueNV | gl_RayFlagsCullBackFacingTrianglesNV;
	//const uint cullMask = RAY_FIRST_PERSON_VISIBLE;
	rp.transparent = vec4(0);
	rp.max_transparent_distance = 0;
	rp.addCount = 0;
	rp.transLight = false;
	rp.trans = false;

	traceNV( topLevelAS, rayFlags, cullMask,
			SBT_RCHIT_OPAQUE /*sbtRecordOffset*/, 0 /*sbtRecordStride*/, SBT_RMISS_PATH_TRACER /*missIndex*/,
			ray.origin, ray.t_min, ray.direction, ray.t_max, PAYLOAD_BRDF);
}

void traceRayOpaque(Ray ray, uint cullMask)
{
	const uint rayFlags = gl_RayFlagsCullFrontFacingTrianglesNV;
	rp.transparent = vec4(0);
	rp.max_transparent_distance = 0;
	rp.addCount = 0;
	rp.transLight = false;
	rp.trans = false;

	traceNV( topLevelAS, rayFlags, cullMask,
			SBT_RCHIT_OPAQUE /*sbtRecordOffset*/, 0 /*sbtRecordStride*/, SBT_RMISS_PATH_TRACER /*missIndex*/,
			ray.origin, ray.t_min, ray.direction, ray.t_max, PAYLOAD_BRDF);
}
void traceRay(Ray ray, uint cullMask)
{
	const uint rayFlags = gl_RayFlagsNoOpaqueNV  ;
	rp.transparent = vec4(0);
	rp.max_transparent_distance = 0;
	rp.addCount = 0;
	rp.transLight = false;
	rp.trans = false;

	traceNV( topLevelAS, rayFlags, cullMask,
			SBT_RCHIT_OPAQUE /*sbtRecordOffset*/, 0 /*sbtRecordStride*/, SBT_RMISS_PATH_TRACER /*missIndex*/,
			ray.origin, ray.t_min, ray.direction, ray.t_max, PAYLOAD_BRDF);
}

float trace_shadow_ray(vec3 pos, vec3 dir, float t_min, float t_max, bool is_player)
{
	const uint rayFlags = gl_RayFlagsNoOpaqueNV ;//gl_RayFlagsTerminateOnFirstHitNV | gl_RayFlagsSkipClosestHitShaderNV;
	uint cullMask = RAY_MIRROR_OPAQUE_VISIBLE;
	if(is_player) {
		cullMask = RAY_FIRST_PERSON_OPAQUE_VISIBLE;
	}
	
	Ray ray;
	ray.origin = pos;
	ray.direction = dir;
	ray.t_min = t_min;
	ray.t_max = t_max - t_min;

	rp_shadow.visFactor = 0.0f;

	traceNV( topLevelAS, rayFlags, cullMask,
			SBT_RCHIT_SHADOW_RAY /*sbtRecordOffset*/, 0 /*sbtRecordStride*/, SBT_RMISS_SHADOW_RAY /*missIndex*/,
			ray.origin, ray.t_min, ray.direction, ray.t_max, PAYLOAD_SHADOW);

	return rp_shadow.visFactor;
}


void traceStraight(vec3 P, vec3 D, vec3 N){
	// if(dot(D, N) >= 0)
	// 	P += D * 0.001;
	// else
	// 	P -= N * 0.001;

	traceNV( topLevelAS, gl_RayFlagsNoneNV, RAY_FIRST_PERSON_OPAQUE_VISIBLE | RAY_FIRST_PERSON_PARTICLE_VISIBLE,
			SBT_RCHIT_OPAQUE, 0, SBT_RMISS_PATH_TRACER,
			P, 0.01, D, 10000.0, PAYLOAD_REFLECT);
}
void traceReflect(vec3 P, vec3 D, vec3 N){
	D = reflect(D, N);
	if(dot(D, N) >= 0)
		P -= D * 0.001;
	else
		P -= N * 0.001;

	traceNV( topLevelAS, gl_RayFlagsNoneNV, RAY_MIRROR_OPAQUE_VISIBLE | RAY_MIRROR_PARTICLE_VISIBLE,
			SBT_RCHIT_OPAQUE, 0, SBT_RMISS_PATH_TRACER,
			P, 0.01, D, 10000.0, PAYLOAD_REFLECT);
}
void traceRefract(vec3 P, vec3 D, vec3 N, float n1, float n2){
	D = refract( D, N, n2/n1 );
	D = refract( D, N, n1/n2 );
	if(dot(D, N) >= 0)
		P -= D * 0.001;
	else
		P -= N * 0.001;
		
	traceNV( topLevelAS, gl_RayFlagsNoneNV, RAY_MIRROR_OPAQUE_VISIBLE | RAY_MIRROR_PARTICLE_VISIBLE,
			SBT_RCHIT_OPAQUE, 0, SBT_RMISS_PATH_TRACER,
			P, 0.01, D, 10000.0, PAYLOAD_REFLECT);
}