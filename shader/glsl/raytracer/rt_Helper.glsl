

float luminance(vec3 v)
{
    return dot(v, vec3(0.2126f, 0.7152f, 0.0722f));
}

vec3 change_luminance(vec3 c_in, float l_out)
{
    float l_in = luminance(c_in);
    return c_in * (l_out / l_in);
}

// float
// luminance(in vec3 color)
// {
//     return dot(color, vec3(0.299, 0.587, 0.114));
// }
float
clamped_luminance(vec3 c)
{
    float l = luminance(c);
    return min(l, 150.0);
}
vec3
clamp_color(vec3 color, float max_val)
{
    float m = max(color.r, max(color.g, color.b));
    if(m < max_val)
        return color;

    return color * (max_val / m);
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

uint
encode_normal(vec3 normal)
{
    uint projected0, projected1;
    const float invL1Norm = 1.0f / dot(abs(normal), vec3(1));

    // first find floating point values of octahedral map in [-1,1]:
    float enc0, enc1;
    if (normal[2] < 0.0f) {
        enc0 = (1.0f - abs(normal[1] * invL1Norm)) * ((normal[0] < 0.0f) ? -1.0f : 1.0f);
        enc1 = (1.0f - abs(normal[0] * invL1Norm)) * ((normal[1] < 0.0f) ? -1.0f : 1.0f);
    }
    else {
        enc0 = normal[0] * invL1Norm;
        enc1 = normal[1] * invL1Norm;
    }
    // then encode:
    uint enci0 = floatBitsToUint((abs(enc0) + 2.0f)/2.0f);
    uint enci1 = floatBitsToUint((abs(enc1) + 2.0f)/2.0f);
    // copy over sign bit and truncated mantissa. could use rounding for increased precision here.
    projected0 = ((floatBitsToUint(enc0) & 0x80000000u)>>16) | ((enci0 & 0x7fffffu)>>8);
    projected1 = ((floatBitsToUint(enc1) & 0x80000000u)>>16) | ((enci1 & 0x7fffffu)>>8);
    // avoid -0 cases:
    if((projected0 & 0x7fffu) == 0) projected0 = 0;
    if((projected1 & 0x7fffu) == 0) projected1 = 0;
    return (projected1 << 16) | projected0;
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
float fresnel(float F0, float NdotV)
{
    if(F0 < 0) return 0;
    return F0 + (1 - F0) * pow(1 - NdotV, 5);
}



uvec2 packHalf4x16(vec4 v)
{
    return uvec2(packHalf2x16(v.xy), packHalf2x16(v.zw));
}

vec4 unpackHalf4x16(uvec2 v)
{
    return vec4(unpackHalf2x16(v.x), unpackHalf2x16(v.y));
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

// Utility function to get a vector perpendicular to an input vector 
//    (from "Efficient Construction of Perpendicular Vectors Without Branching")
vec3 getPerpendicularVector(vec3 u)
{
	vec3 a = abs(u);
	uint xm = ((a.x - a.y)<0 && (a.x - a.z)<0) ? 1 : 0;
	uint ym = (a.y - a.z)<0 ? (1 ^ xm) : 0;
	uint zm = 1 ^ (xm | ym);
	return cross(u, vec3(xm, ym, zm));
}

// Generates a seed for a random number generator from 2 inputs plus a backoff
uint initRand(uint val0, uint val1, uint backoff/* = 16*/)
{
	uint v0 = val0, v1 = val1, s0 = 0;

	for (uint n = 0; n < backoff; n++)
	{
		s0 += 0x9e3779b9;
		v0 += ((v1 << 4) + 0xa341316c) ^ (v1 + s0) ^ ((v1 >> 5) + 0xc8013ea4);
		v1 += ((v0 << 4) + 0xad90777d) ^ (v0 + s0) ^ ((v0 >> 5) + 0x7e95761e);
	}
	return v0;
}

// Takes our seed, updates it, and returns a pseudorandom float in [0..1]
float nextRand(inout uint s)
{
	s = (1664525u * s + 1013904223u);
	return float(s & 0x00FFFFFF) / float(0x01000000);
}

// Get a cosine-weighted random vector centered around a specified normal direction.
vec3 getCosHemisphereSample(vec2 randVal, /*inout uint randSeed,*/ vec3 hitNorm)
{
	// Get 2 random numbers to select our sample with
	//vec2 randVal = vec2(nextRand(randSeed), nextRand(randSeed));

	// Cosine weighted hemisphere sample from RNG
	vec3 bitangent = getPerpendicularVector(hitNorm);
	vec3 tangent = cross(bitangent, hitNorm);
	float r = sqrt(randVal.x);
	float phi = 2.0f * 3.14159265f * randVal.y;

	// Get our cosine-weighted hemisphere lobe sample direction
	return tangent * (r * cos(phi).x) + bitangent * (r * sin(phi)) + hitNorm.xyz * sqrt(max(0.0,1.0f - randVal.x));
}
vec3 getCosHemisphereSample2(vec2 randVal, /*vec2 rand,*/ vec3 hitNorm)
{
	// Get 2 random numbers to select our sample with
	//vec2 randVal = rand;

	// Cosine weighted hemisphere sample from RNG
	vec3 bitangent = getPerpendicularVector(hitNorm);
	vec3 tangent = cross(bitangent, hitNorm);
	float r = sqrt(randVal.x);
	float phi = 2.0f * 3.14159265f * randVal.y;

	// Get our cosine-weighted hemisphere lobe sample direction
	return tangent * (r * cos(phi).x) + bitangent * (r * sin(phi)) + hitNorm.xyz * sqrt(max(0.0,1.0f - randVal.x));
}

vec3
sample_sphere(vec2 uv)
{
    float y = 2.0 * uv.x - 1;
    float theta = 2.0 * M_PI * uv.y;
    float r = sqrt(1.0 - y * y);
    return vec3(cos(theta) * r, y, sin(theta) * r);
}


// Get a uniform weighted random vector centered around a specified normal direction.
vec3 getUniformHemisphereSample(vec2 randVal,/*inout uint randSeed,*/ vec3 hitNorm)
{
	// Get 2 random numbers to select our sample with
	//vec2 randVal = vec2(nextRand(randSeed), nextRand(randSeed));

	// Cosine weighted hemisphere sample from RNG
	vec3 bitangent = getPerpendicularVector(hitNorm);
	vec3 tangent = cross(bitangent, hitNorm);
	float r = sqrt(max(0.0f,1.0f - randVal.x*randVal.x));
	float phi = 2.0f * 3.14159265f * randVal.y;

	// Get our cosine-weighted hemisphere lobe sample direction
	return tangent * (r * cos(phi).x) + bitangent * (r * sin(phi)) + hitNorm.xyz * randVal.x;
}

uint packRGBE(vec3 v)
{
    vec3 va = max(vec3(0), v);
    float max_abs = max(va.r, max(va.g, va.b));
    if(max_abs == 0)
        return 0;

    float exponent = floor(log2(max_abs));

    uint result;
    result = uint(clamp(exponent + 20, 0, 31)) << 27;

    float scale = pow(2, -exponent) * 256.0;
    uvec3 vu = min(uvec3(511), uvec3(round(va * scale)));
    result |= vu.r;
    result |= vu.g << 9;
    result |= vu.b << 18;

    return result;
}

vec3 unpackRGBE(uint x)
{
    int exponent = int(x >> 27) - 20;
    float scale = pow(2, exponent) / 256.0;

    vec3 v;
    v.r = float(x & 0x1ff) * scale;
    v.g = float((x >> 9) & 0x1ff) * scale;
    v.b = float((x >> 18) & 0x1ff) * scale;

    return v;
}

vec3 reinhard_extended(vec3 v, float max_white)
{
    vec3 numerator = v * (1.0f + (v / vec3(max_white * max_white)));
    return numerator / (1.0f + v);
}
vec3 reinhard(vec3 v)
{
    return v / (1.0f + v);
}
vec3 reinhard_extended_luminance(vec3 v, float max_white_l)
{
    float l_old = luminance(v);
    float numerator = l_old * (1.0f + (l_old / (max_white_l * max_white_l)));
    float l_new = numerator / (1.0f + l_old);
    return change_luminance(v, l_new);
}