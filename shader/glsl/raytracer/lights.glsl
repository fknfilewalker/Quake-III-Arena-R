// layout(binding = BINDING_OFFSET_UBO_LIGHTS, set = 0) uniform LightList 
// {
// 	LightList_s lightList;
// } uboLights;

layout(binding = BINDING_OFFSET_UBO_LIGHTS, set = 0) buffer LightList { LightList_s lightList; } uboLights;

// vis data
layout(binding = BINDING_OFFSET_VIS_DATA, set = 0, r8ui) uniform uimage2D vis_data;
layout(binding = BINDING_OFFSET_LIGHT_VIS_DATA, set = 0, r32ui) uniform uimage2D lightVis_data;

vec3
getLight(uint i){
	return uboLights.lightList.lights[i].pos.xyz;
}

bool
lightVisible(int i, uint hitCluster){
	uint cluster = uboLights.lightList.lights[i].cluster;
	if((imageLoad(vis_data, ivec2(cluster>>3, hitCluster)).r  & (1<<(cluster&7))) > 0 ) return true;
	return false;
}

vec3
sample_triangle(vec2 xi)
{
    float sqrt_xi = sqrt(xi.x);
    return vec3(
        1.0 - sqrt_xi,
        sqrt_xi * (1.0 - xi.y),
        sqrt_xi * xi.y);
}

DirectionalLight getLight2(in Light l, ivec2 rng, bool random){
	float rng_x = get_rng(RNG_LP_X(rng.x), rng.y);
	float rng_y = get_rng(RNG_LP_Y(rng.x), rng.y);

	DirectionalLight light;
	light.normal = l.normal;
	light.mag = l.size;
	if(random) light.pos = l.pos.xyz + (rng_x * l.AB.xyz + rng_y * l.AC.xyz);
	else light.pos = l.pos.xyz + (0.5 * l.AB.xyz + 0.5 * l.AC.xyz);

	light.color = l.color.xyz;
	return light;
}

// vec3 Shade(vec3 pos, vec3 norm, vec3 difColor, uint lightIdx, uint numLights){
// 	float sampleProb = 1.0f / float(numLights);

// 	DirectionalLight light = getLight2(uboLights.lightList.lights[lightIdx], ivec2(lightIdx, ubo.frameIndex), ubo.randSampleLight);
// 	vec3 lightPos = light.pos;
// 	float distToLight = length(lightPos - pos);
// 	vec3 lightIntensity = light.color * vec3(0.1);//(light.mag * light.color) / distToLight;
// 	vec3 dirToLight = normalize(lightPos  - pos);

// 	float NdotL = clamp(dot(norm, dirToLight), 0.0, 1.0);
// 	float isLit = trace_shadow_ray(pos, dirToLight, 0.01f, distToLight, false);
// 	vec3 rayColor = isLit * lightIntensity;

// 	return (NdotL * rayColor * (difColor / M_PI)) / sampleProb;
// }

vec3 calcShading(in vec4 primary_albedo, in vec3 P, in vec3 N, in uint cluster, in uint material){
	vec3 shadeColor = vec3(primary_albedo);

	uint numLight = uboLights.lightList.numLights; 
	if(ubo.cullLights) numLight = imageLoad(lightVis_data, ivec2(0, cluster)).r;
	else numLight = uboLights.lightList.numLights; 

	for(int i = 0; i < numLight; i++){
		uint lightIndex;
		//uint rand = int( get_rng(RNG_C(0), int(ubo.frameIndex)) * (numLight)) ;
		if(ubo.cullLights) {
			lightIndex = imageLoad(lightVis_data, ivec2( i + 1, cluster)).r; // first index is numLight so + 1
			// if(lightVisible(i, cluster)){
			// 	lightIndex = i;
			// }
			// else continue;
		}
		else {
			// if(lightVisible(i, cluster)){
			// 	lightIndex = i;
			// }
			// else continue;
			lightIndex = i;
		}
		
	
		DirectionalLight light = getLight2(uboLights.lightList.lights[lightIndex], ivec2(i, ubo.frameIndex), ubo.randSampleLight);
		vec3 posLight = light.pos;
		vec3 toLight = posLight - P;
		vec3 L = normalize(toLight);
		float distToLight =	length(toLight);
		float lightStrength =  min(light.mag / distToLight, 5);
		//if(lightStrength < 0.1f) continue;
		vec3 lightIntensity = light.color * lightStrength;
		//if(i > 30 && i < 50) lightIntensity = vec3(0.55,0,0);
	 	float shadowMult = trace_shadow_ray(P, L, 0.01f, distToLight, isPlayer(material));

		float LdotN = clamp(dot(N, L), 0.0, 1.0);
		float LdotN2 = clamp(dot(-light.normal, -L), 0.0, 1.0);
		shadeColor += shadowMult * LdotN * lightIntensity;
	}

	//shadeColor *= primary_albedo.rgb / M_PI;
	return shadeColor * primary_albedo.rgb / M_PI;
	//return primary_albedo.rgb;
}