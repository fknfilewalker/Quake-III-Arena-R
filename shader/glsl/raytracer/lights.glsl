// layout(binding = BINDING_OFFSET_UBO_LIGHTS, set = 0) uniform LightList 
// {
// 	LightList_s lightList;
// } uboLights;

layout(binding = BINDING_OFFSET_UBO_LIGHTS, set = 0) buffer LightList { LightList_s lightList; } uboLights;

// vis data
layout(binding = BINDING_OFFSET_VIS_DATA, set = 0, r8ui) uniform uimage2D vis_data;
layout(binding = BINDING_OFFSET_LIGHT_VIS_DATA, set = 0, r32ui) uniform uimage2D lightVis_data;
layout(binding = BINDING_OFFSET_LIGHT_VIS_DATA2, set = 0, r32ui) uniform uimage2D lightVis_data2;

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
	float rng_x = getRNG(ubo.denoiser, RNG_LP_X(rng.x), rng.y);
	float rng_y = getRNG(ubo.denoiser, RNG_LP_Y(rng.x), rng.y);
	//float rng_x = get_rng(RNG_LP_X(rng.x), rng.y);
	//float rng_y = get_rng(RNG_LP_Y(rng.x), rng.y);
	// float rng_x = get_rng2(RNG_NEE_TRI_X(0));
	// float rng_y = get_rng2(RNG_NEE_TRI_Y(0));

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
	//if(!ubo.illumination) return primary_albedo.xyz;if(!ubo.illumination) return primary_albedo.xyz;
	vec3 shadeColor = vec3(0);

	float amplification = 1;
	uint numLights; 
	if(ubo.cullLights) numLights = imageLoad(lightVis_data, ivec2(0, cluster)).r;
	else numLights = uboLights.lightList.numLights;
	uint totalNumLights = numLights;

	if(ubo.numRandomDL > 0) {
		numLights = min(numLights, ubo.numRandomDL);
		amplification = float(totalNumLights) / float(numLights);
	}
	//amplification = uboLights.lightList.numLights;
	//if(!ubo.cullLights) amplification *= 14;

	for(int i = 0; i < numLights; i++){
		uint lightIndex;
		//uint rand = int(round(get_rng(RNG_C(0), int(ubo.frameIndex)) * (numLight))) ;
		if(ubo.cullLights) {
			if(ubo.numRandomDL > 0) {
				//uint rand = int(round(get_rng(RNG_C(i), int(ubo.frameIndex)) * (totalNumLights)));
				//uint rand = int(round(get_rng2(RNG_NEE_LH(0)) * (totalNumLights)));
				uint rand = int(round(getRNG(ubo.denoiser, RNG_NEE_LH(i), int(ubo.frameIndex)) * (totalNumLights)));
				lightIndex = imageLoad(lightVis_data, ivec2( rand + 1, cluster)).r;
			}
			else lightIndex = imageLoad(lightVis_data, ivec2( i + 1, cluster)).r; // first index is numLight so + 1
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
			if(ubo.numRandomDL > 0) {
				//lightIndex = int(round(get_rng(RNG_C(i), int(ubo.frameIndex)) * (totalNumLights)));
				//lightIndex = int(round(get_rng2(RNG_NEE_LH(0)) * (totalNumLights)));
				lightIndex = int(round(getRNG(ubo.denoiser, RNG_NEE_LH(i), int(ubo.frameIndex)) * (totalNumLights)));
			}
			else lightIndex = i;
		}
		
	
		DirectionalLight light = getLight2(uboLights.lightList.lights[lightIndex], ivec2(i, ubo.frameIndex), ubo.randSampleLight);
		vec3 posLight = light.pos;
		vec3 toLight = posLight - P;
		vec3 L = normalize(toLight);
		float distToLight =	length(toLight);
	 	float shadowMult = trace_shadow_ray(P, L, 0.01f, distToLight, isPlayer(material));
		if(shadowMult == 0) continue;
		shadowMult *= amplification; // if we only would use one light it would need to be multiplied with numLights
		float lightStrength =  min(light.mag / distToLight, 5);
		vec3 lightIntensity = light.color * lightStrength;

		float LdotN = clamp(dot(normalize(N), L), 0.0, 1.0);
		//float LdotN2 = clamp(dot(-light.normal, -L), 0.0, 1.0);
		shadeColor += shadowMult * LdotN * lightIntensity;
	}

	vec3 ret = shadeColor / M_PI;
	return ret;//clamp_color(ret, 2 * (totalNumLights/imageLoad(lightVis_data, ivec2(0, cluster)).r));
	//shadeColor *= primary_albedo.rgb / M_PI;
	//return (shadeColor * primary_albedo.rgb / M_PI);// + primary_albedo.rgb * 0.25f;
	//return primary_albedo.rgb;
}

vec3 calcShadingIndirect(in vec4 primary_albedo, in vec3 P, in vec3 N, in uint cluster, in uint material){
	//if(!ubo.illumination) return primary_albedo.xyz;if(!ubo.illumination) return primary_albedo.xyz;
	vec3 shadeColor = vec3(0);

	float amplification = 1;
	uint numLights; 
	if(ubo.cullLights) numLights = imageLoad(lightVis_data, ivec2(0, cluster)).r;
	else numLights = uboLights.lightList.numLights;
	uint totalNumLights = numLights;

	if(ubo.numRandomIL > 0) {
		numLights = min(numLights, ubo.numRandomIL);
		amplification = float(totalNumLights) / float(numLights);
	}
	//amplification = uboLights.lightList.numLights;
	//if(!ubo.cullLights) amplification *= 14;

	for(int i = 0; i < numLights; i++){
		uint lightIndex;
		//uint rand = int(round(get_rng(RNG_C(0), int(ubo.frameIndex)) * (numLight))) ;
		if(ubo.cullLights) {
			if(ubo.numRandomIL > 0) {
				//uint rand = int(round(get_rng(RNG_C(i), int(ubo.frameIndex)) * (totalNumLights)));
				//uint rand = int(round(get_rng2(RNG_NEE_LL(0)) * (totalNumLights)));
				uint rand = int(round(getRNG(ubo.denoiser, RNG_NEE_LL(i), int(ubo.frameIndex)) * (totalNumLights)));
				lightIndex = imageLoad(lightVis_data, ivec2( rand + 1, cluster)).r;
			}
			else lightIndex = imageLoad(lightVis_data, ivec2( i + 1, cluster)).r; // first index is numLight so + 1
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
			if(ubo.numRandomIL > 0) {
				//lightIndex = int(round(get_rng(RNG_C(i), int(ubo.frameIndex)) * (totalNumLights)));
				//lightIndex = int(round(get_rng2(RNG_NEE_LL(0)) * (totalNumLights)));
				lightIndex = int(round(getRNG(ubo.denoiser, RNG_NEE_LL(i), int(ubo.frameIndex)) * (totalNumLights)));
			}
			else lightIndex = i;
		}
		
	
		DirectionalLight light = getLight2(uboLights.lightList.lights[lightIndex], ivec2(i, ubo.frameIndex), ubo.randSampleLight);
		vec3 posLight = light.pos;
		vec3 toLight = posLight - P;
		vec3 L = normalize(toLight);
		float distToLight =	length(toLight);
	 	float shadowMult = trace_shadow_ray(P, L, 0.01f, distToLight, isPlayer(material));
		if(shadowMult == 0) continue;
		shadowMult *= amplification; // if we only would use one light it would need to be multiplied with numLights
		float lightStrength =  min(light.mag / distToLight, 5);
		vec3 lightIntensity = light.color * lightStrength;

		float LdotN = clamp(dot(normalize(N), L), 0.0, 1.0);
		//float LdotN2 = clamp(dot(-light.normal, -L), 0.0, 1.0);
		bool gCosSampling = false;
		float sampleProb = gCosSampling ? (LdotN / M_PI) : (1.0f / (2.0f * M_PI));
		shadeColor += shadowMult * LdotN * lightIntensity;
		//shadeColor /= sampleProb;
	}
	vec3 ret = shadeColor / M_PI;
	return ret;//clamp_color(ret, 2 * (totalNumLights/imageLoad(lightVis_data, ivec2(0, cluster)).r));
	//shadeColor *= primary_albedo.rgb / M_PI;
	//return (shadeColor * primary_albedo.rgb / M_PI);// + primary_albedo.rgb * 0.25f;
	//return primary_albedo.rgb;
}