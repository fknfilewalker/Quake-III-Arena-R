#include "../constants.h"
layout(binding = BINDING_OFFSET_ENVMAP) uniform samplerCube samplerCubeMap;

mat3 s_flipMatrix = mat3(
	0.0f, 0.0f, 1.0f,
	-1.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f
);

vec4 sampleSky(vec3 direction){
	return texture(samplerCubeMap, direction);
}