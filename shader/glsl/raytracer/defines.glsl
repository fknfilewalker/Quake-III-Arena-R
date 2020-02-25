// struct RayPayload {
// 	vec4 color;
// 	vec4 normal;
// 	uint blendFunc;
// 	uint transparent;
// 	float distance;
// 	uint depth;
// 	uint cullMask;
// };
#define PAYLOAD_BRDF 0
#define PAYLOAD_SHADOW 1

#define M_PI 3.14159265358979323846264338327950288f

#define ALBEDO_MULT 1.3

#define RNG_C(bounce)                (1 + 0 + 7 * bounce)
#define RNG_LP_X(bounce)             (3 + 0 + 7 * bounce)
#define RNG_LP_Y(bounce)             (4 + 0 + 7 * bounce)
#define RNG_BOUNCE_X(bounce)         (5 + 0 + 7 * bounce)
#define RNG_BOUNCE_Y(bounce)         (6 + 0 + 7 * bounce)
#define RNG_FRESNEL(bounce)        (7 + 0 + 7 * bounce)


struct RayPayload {
	vec2 barycentric;
	uint instanceID;
	uint primitiveID;
	float hit_distance;
	vec4 transparent;
	float max_transparent_distance;
	//mat4x3 modelmat;
};
struct ShadowRayPayload
{
  	float visFactor;  // Will be 1.0 for fully lit, 0.0 for fully shadowed
};
struct Ray {
	vec3 origin, direction;
	float t_min, t_max;
};
struct Triangle {
	mat3 pos;
	mat3x2 uv0;
	mat3x2 uv1;
	mat3x2 uv2;
	mat3x2 uv3;
	vec3 normal;
	mat3x4 color0;
	mat3x4 color1;
	mat3x4 color2;
	mat3x4 color3;
	uint tex0;
	uint tex1;
	uint cluster;
	uint material;
};
struct HitPoint {
	vec3 pos;
	vec3 normal;
	vec2 uv0;
	vec2 uv1;
	vec2 uv2;
	vec2 uv3;
	vec4 color0;
	vec4 color1;
	vec4 color2;
	vec4 color3;
	uint tex0;
	uint tex1;
	uint cluster;
	uint material;
};

struct DirectionalLight {
	vec3 pos;
	vec3 color;
	float mag;
	vec3 normal;
};

struct TextureData {
	int tex0;
	int tex1;
	uint tex0Blend;
	uint tex1Blend;
	bool tex0Color;
	bool tex1Color;
};

// for shaderSort_t
const float SS_BAD 									= 0;
const float SS_PORTAL 								= 1;	// mirrors, portals, viewscreens
const float SS_ENVIRONMENT 							= 2;	// sky box
const float SS_OPAQUE 								= 3;	// opaque

const float SS_DECAL 								= 4;	// scorch marks, etc.
const float SS_SEE_THROUGH 							= 5;	// ladders, grates, grills that may have small blended edges
																	// in addition to alpha test
const float SS_BANNER 								= 6;

const float SS_FOG 									= 7;

const float SS_UNDERWATER 							= 8;	// for items that should be drawn in front of the water plane

const float SS_BLEND0 								= 9;	// regular transparency and filters
const float SS_BLEND1 								= 10;	// generally only used for additive type effects
const float SS_BLEND2 								= 11;
const float SS_BLEND3 								= 12;

const float SS_BLEND6 								= 13;
const float SS_STENCIL_SHADOW 						= 14;
const float SS_ALMOST_NEAREST 						= 15;	// gun smoke puffs

const float SS_NEAREST 								= 16;	// blood blobs
