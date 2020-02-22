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

	traceNV( topLevelAS, rayFlags, cullMask,
			SBT_RCHIT_OPAQUE /*sbtRecordOffset*/, 0 /*sbtRecordStride*/, SBT_RMISS_PATH_TRACER /*missIndex*/,
			ray.origin, ray.t_min, ray.direction, ray.t_max, PAYLOAD_BRDF);
}

void traceRayOpaque(Ray ray, uint cullMask)
{
	const uint rayFlags =   gl_RayFlagsTerminateOnFirstHitNV ;
	rp.transparent = vec4(0);
	rp.max_transparent_distance = 0;

	traceNV( topLevelAS, rayFlags, cullMask,
			SBT_RCHIT_OPAQUE /*sbtRecordOffset*/, 0 /*sbtRecordStride*/, SBT_RMISS_PATH_TRACER /*missIndex*/,
			ray.origin, ray.t_min, ray.direction, ray.t_max, PAYLOAD_BRDF);
}
void traceRay(Ray ray, uint cullMask)
{
	const uint rayFlags = gl_RayFlagsNoneNV;
	rp.transparent = vec4(0);
	rp.max_transparent_distance = 0;

	traceNV( topLevelAS, rayFlags, cullMask,
			SBT_RCHIT_OPAQUE /*sbtRecordOffset*/, 0 /*sbtRecordStride*/, SBT_RMISS_PATH_TRACER /*missIndex*/,
			ray.origin, ray.t_min, ray.direction, ray.t_max, PAYLOAD_BRDF);
}

float trace_shadow_ray(vec3 pos, vec3 dir, float t_min, float t_max, bool is_player)
{
	const uint rayFlags = gl_RayFlagsCullNoOpaqueNV | gl_RayFlagsOpaqueNV | gl_RayFlagsTerminateOnFirstHitNV | gl_RayFlagsSkipClosestHitShaderNV;
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