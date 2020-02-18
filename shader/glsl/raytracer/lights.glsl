layout(binding = BINDING_OFFSET_UBO_LIGHTS, set = 0) uniform LightList 
{
	LightList_s lightList;
} uboLights;

vec3
sample_triangle(vec2 xi)
{
    float sqrt_xi = sqrt(xi.x);
    return vec3(
        1.0 - sqrt_xi,
        sqrt_xi * (1.0 - xi.y),
        sqrt_xi * xi.y);
}

DirectionalLight getLight2(Light l, ivec2 rng, bool random){
	uint customIndex = uint(l.offsetIDX);
	uvec3 index;
	if(l.type == BAS_WORLD_STATIC) index = (ivec3(indices_world_static.i[customIndex], indices_world_static.i[customIndex + 1], indices_world_static.i[customIndex + 2])) + uint(l.offsetXYZ);
	else if(l.type == BAS_WORLD_DYNAMIC_DATA)  index = (ivec3(indices_dynamic_data.i[customIndex], indices_dynamic_data.i[customIndex + 1], indices_dynamic_data.i[customIndex + 2])) + uint(l.offsetXYZ);
	else if(l.type == BAS_WORLD_DYNAMIC_AS)  index = (ivec3(indices_dynamic_as.i[customIndex], indices_dynamic_as.i[customIndex + 1], indices_dynamic_as.i[customIndex + 2])) + uint(l.offsetXYZ);

	VertexBuffer vData[3];
	TextureData d;
	TextureData d2;
	if(l.type == BAS_WORLD_STATIC){
		vData[0] = vertices_world_static.v[index.x];
		vData[1] = vertices_world_static.v[index.y];
		vData[2] = vertices_world_static.v[index.z];
		d = unpackTextureData(vertices_world_static.v[index.x].texIdx0);
		d2 = unpackTextureData(vertices_world_static.v[index.x].texIdx1);
	}else if(l.type == BAS_WORLD_DYNAMIC_DATA){
		vData[0] = vertices_dynamic_data.v[index.x];
		vData[1] = vertices_dynamic_data.v[index.y];
		vData[2] = vertices_dynamic_data.v[index.z];
		d = unpackTextureData(vertices_dynamic_data.v[index.x].texIdx0);
		d2 = unpackTextureData(vertices_dynamic_data.v[index.x].texIdx1);
	}else if(l.type == BAS_WORLD_DYNAMIC_AS){
		vData[0] = vertices_dynamic_as.v[index.x];
		vData[1] = vertices_dynamic_as.v[index.y];
		vData[2] = vertices_dynamic_as.v[index.z];
		d = unpackTextureData(vertices_dynamic_as.v[index.x].texIdx0);
		d2 = unpackTextureData(vertices_dynamic_as.v[index.x].texIdx1);
	}

	vec2 uv0 = (vData[0].uv0 + vData[1].uv0 + vData[2].uv0) / 3.0f;
	vec2 uv1 = (vData[0].uv1 + vData[1].uv1 + vData[2].uv1) / 3.0f;
	vec2 uv2 = (vData[0].uv2 + vData[1].uv2 + vData[2].uv2) / 3.0f;

	// vec4 color0 = (unpackColor(vData[0].color0) + unpackColor(vData[1].color0) + unpackColor(vData[2].color0)) / 3.0f;
	// vec4 color1 = (unpackColor(vData[0].color1) + unpackColor(vData[1].color1) + unpackColor(vData[2].color1)) / 3.0f;
	// vec4 color2 = (unpackColor(vData[0].color2) + unpackColor(vData[1].color2) + unpackColor(vData[2].color2)) / 3.0f;
	// vec4 color3 = (unpackColor(vData[0].color3) + unpackColor(vData[1].color3) + unpackColor(vData[2].color3)) / 3.0f;

	float rng_x = get_rng(RNG_LP_X(rng.x), rng.y);
	float rng_y = get_rng(RNG_LP_Y(rng.x), rng.y);
	//int( round( get_rng(RNG_C(i), int(ubo.frameIndex)) * uboLights.lightList.numLights ) )

	vec3 dir_x = vData[0].pos.xyz - vData[1].pos.xyz;
	vec3 dir_y = vData[2].pos.xyz - vData[1].pos.xyz;

	DirectionalLight light;
	light.normal = cross(dir_x, dir_y);
	light.mag = length(light.normal);
	if(random) light.pos = vData[1].pos.xyz + (rng_x * dir_x + rng_y * dir_y);
	else light.pos = l.pos.xyz;
	//light.pos = (vData[0].pos.xyz + vData[1].pos.xyz + vData[2].pos.xyz) / 3;
	//light.pos = l.pos.xyz * sample_triangle(vec2(rng_x, rng_y)); 


	// light.color = global_textureLod(d.tex0, vec2(0.5f, 0.5f), 2).xyz;
	// if(d.tex1 != -1) light.color += global_textureLod(d.tex1, vec2(0.5f, 0.5f), 2).xyz;
	// if(d2.tex0 != -1) light.color += global_textureLod(d2.tex0, vec2(0.5f, 0.5f), 2).xyz;
	// if(d2.tex1 != -1) light.color += global_textureLod(d2.tex1, vec2(0.5f, 0.5f), 2).xyz;

	vec4 color = vec4(0);
	// vec4 tex = global_textureLod(d.tex0, uv0, 2);
	// color = vec4(tex.xyz, 1);
	// //if(d.tex0Color) color *= (color0/255);
	// //if(d.tex0Color) color *= (hp.color0/255);

	// if(d.tex1 != -1){
	// 	tex = global_textureLod(d.tex1, uv1, 2);
	// 	//if(d.tex1Color) tex *= (color1/255);

	// 	if(d.tex1Blend) {
	// 		color = alpha_blend(tex, color);
	// 	}
	// 	else color += tex;
	// }

	// if(d2.tex0 != -1){
	// 	tex = global_textureLod(d2.tex0, uv2, 2);
	// 	//if(d.tex0Color) tex *= (color2/255);

	// 	if(d.tex0Blend) {
	// 		color = alpha_blend(tex, color);
	// 	}
	// 	else color += tex;
	// }

	// if(d2.tex1 != -1){
	// 	tex = global_textureLod(d2.tex1, vec2(0.5f, 0.5f), 2);
	// 	//if(d.tex1Color) tex *= (color3/255);

	// 	if(d.tex1Blend) {
	// 		color = alpha_blend(tex, color);
	// 	}
	// 	else color += tex;
	// } 
	light.color = color.xyz;
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
	vec3 shadeColor = vec3(0);

	uint numLight;
	if(ubo.cullLights) numLight = imageLoad(lightVis_data, ivec2(0, cluster)).r;
	else numLight = uboLights.lightList.numLights; 

	for(int i = 0; i < numLight; i++){
		uint lightIndex;
		//uint rand = int( get_rng(RNG_C(0), int(ubo.frameIndex)) * (numLight)) ;
		if(ubo.cullLights) lightIndex = imageLoad(lightVis_data, ivec2( i + 1, cluster)).r; // first index is numLight so + 1
		else lightIndex = i;
		
	
		DirectionalLight light = getLight2(uboLights.lightList.lights[lightIndex], ivec2(i, ubo.frameIndex), ubo.randSampleLight);
		vec3 posLight = light.pos;
		vec3 toLight = posLight - P;
		vec3 L = normalize(toLight);
		float distToLight =	length(toLight);
		float lightStrength =  min(light.mag / distToLight, 1);
		if(lightStrength < 0.1f) continue;
		vec3 lightIntensity = vec3(0.75) * lightStrength; //light.color * lightStrength;
		//if(i > 30 && i < 50) lightIntensity = vec3(0.55,0,0);
	 	float shadowMult = trace_shadow_ray(P, L, 0.01f, distToLight, isPlayer(material));

		float LdotN = clamp(dot(N, L), 0.0, 1.0);
		float LdotN2 = clamp(dot(-light.normal, -L), 0.0, 1.0);
		shadeColor += shadowMult * LdotN * lightIntensity; 
	}

	//shadeColor *= primary_albedo.rgb / M_PI;
	return shadeColor * primary_albedo.rgb / M_PI;
}