

void initPayload(out RayPayload rp){
	// rp.color = vec4(0,0,0,0);
	// rp.normal = vec4(0,0,0,0);
	// rp.blendFunc = 0;
	// rp.transparent = 0;
	// rp.distance = 0;
	// rp.depth = 0;
	// rp.cullMask = FIRST_PERSON_VISIBLE;
}
void initRay(){

}


vec2
sample_disk(vec2 uv)
{
	float theta = 2.0 * 3.141592653589 * uv.x;
	float r = sqrt(uv.y);

	return vec2(cos(theta), sin(theta)) * r;
}

vec3
sample_cos_hemisphere(vec2 uv)
{
	vec2 disk = sample_disk(uv);

	return vec3(disk.x, sqrt(max(0.0, 1.0 - dot(disk, disk))), disk.y);
}

vec3
clamp_color(vec3 color, float max_val)
{
    float m = max(color.r, max(color.g, color.b));
    if(m < max_val)
        return color;

    return color * (max_val / m);
}

vec4
alpha_blend(vec4 top, vec4 bottom)
{
    // assume top is alpha-premultiplied, bottom is not; result is premultiplied
    return vec4(top.rgb + bottom.rgb * (1 - top.a) * bottom.a, 1 - (1 - top.a) * (1 - bottom.a)); 
}

vec4 alpha_blend_premultiplied(vec4 top, vec4 bottom)
{
    // assume everything is alpha-premultiplied
    return vec4(top.rgb + bottom.rgb * (1 - top.a), 1 - (1 - top.a) * (1 - bottom.a)); 
}

float
blinn_phong_based_brdf(vec3 V, vec3 L, vec3 N, float phong_exp)
{
	vec3 H = normalize(V + L);
		float F = pow(1.0 - max(0.0, dot(H, V)), 5.0);
		return mix(0.15, 0.05 + 10.25 * pow(max(0.0, dot(H, N)), phong_exp), F) / M_PI;
}
float schlick_ross_fresnel(float F0, float roughness, float NdotV)
{
    if(F0 < 0)
        return 0;

    // Shlick's approximation for Ross BRDF -- makes Fresnel converge to less than 1.0 when N.V is low
    return F0 + (1 - F0) * pow(1 - NdotV, 5 * exp(-2.69 * roughness)) / (1.0 + 22.7 * pow(roughness, 1.5));
}

vec3
compute_barycentric(mat3 v, vec3 ray_origin, vec3 ray_direction)
{
	vec3 edge1 = v[1] - v[0];
	vec3 edge2 = v[2] - v[0];

	vec3 pvec = cross(ray_direction, edge2);

	float det = dot(edge1, pvec);
	float inv_det = 1.0f / det;

	vec3 tvec = ray_origin - v[0];

	float alpha = dot(tvec, pvec) * inv_det;

	vec3 qvec = cross(tvec, edge1);

	float beta = dot(ray_direction, qvec) * inv_det;

	return vec3(1.f - alpha - beta, alpha, beta);
}