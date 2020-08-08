// Catmull-Rom filtering code from http://vec3.ca/bicubic-filtering-in-fewer-taps/
void
BicubicCatmullRom(vec2 UV, vec2 texSize, out vec2 Sample[3], out vec2 Weight[3])
{
    const vec2 invTexSize = 1.0 / texSize;

    vec2 tc = floor( UV - 0.5 ) + 0.5;
    vec2 f = UV - tc;
    vec2 f2 = f * f;
    vec2 f3 = f2 * f;

    vec2 w0 = f2 - 0.5 * (f3 + f);
    vec2 w1 = 1.5 * f3 - 2.5 * f2 + 1;
    vec2 w3 = 0.5 * (f3 - f2);
    vec2 w2 = 1 - w0 - w1 - w3;

    Weight[0] = w0;
    Weight[1] = w1 + w2;
    Weight[2] = w3;

    Sample[0] = tc - 1;
    Sample[1] = tc + w2 / Weight[1];
    Sample[2] = tc + 2;

    Sample[0] *= invTexSize;
    Sample[1] *= invTexSize;
    Sample[2] *= invTexSize;
}
/* uv is in pixel coordinates */
vec4
sample_texture_catmull_rom(sampler2D tex, vec2 uv)
{//taaASVGF_Prev
    vec4 sum = vec4(0);
    vec2 sampleLoc[3], sampleWeight[3];
    BicubicCatmullRom(uv, vec2(textureSize(tex, 0)), sampleLoc, sampleWeight);
    for(int i = 0; i < 3; i++) {
        for(int j = 0; j < 3; j++) {
            vec2 uv = vec2(sampleLoc[j].x, sampleLoc[i].y);
            vec4 c = textureLod(tex, uv, 0);
            sum += c * vec4(sampleWeight[j].x * sampleWeight[i].y);        
        }
    }
    return sum;
}

vec4
temporal_filter()
{
	ivec2 ipos = ivec2(gl_GlobalInvocationID);

    vec3 mom1 = vec3(0.0);
    vec3 mom2 = vec3(0.0);
    const int r = 1;
    vec4 color_center = vec4(0);
    for(int yy = -r; yy <= r; yy++) {
        for(int xx = -r; xx <= r; xx++) {
            ivec2 p = ipos + ivec2(xx, yy);
            vec4 c = imageLoad(IMG_ASVGF_COLOR, p);

            if(xx == 0 && yy == 0) {
                color_center = c;
			}
			else {
				mom1 += c.rgb;
				mom2 += c.rgb * c.rgb;
			}
        }
    }
    int num_pix = (2 * r + 1) * (2 * r + 1);

	{
		vec3 mom1_c = mom1 / (num_pix - 1);
		vec3 mom2_c = mom2 / (num_pix - 1);

		vec3 sigma = sqrt(max(vec3(0), mom2_c - mom1_c * mom1_c));
		vec3 mi = mom1_c - sigma * 3.0;
		vec3 ma = mom1_c + sigma * 3.0;
		if(any(lessThan(color_center.rgb, mi))
		|| any(greaterThan(color_center.rgb, ma))) {
			color_center.rgb = mom1_c;
		}
	}

    mom1 = (mom1 + color_center.rgb) / float(num_pix);
    mom2 = (mom2 + color_center.rgb * color_center.rgb) / float(num_pix);

    vec2 motion;
    {
        float len = -1;
        const int r = 1;
        for(int yy = -r; yy <= r; yy++) {
            for(int xx = -r; xx <= r; xx++) {
                ivec2 p = ipos + ivec2(xx, yy);
                vec2 m = imageLoad(motionGBuffer, p).xy;
                float l = dot(m, m);
                if(l > len) {
                    len = l;
                    motion = m;
                }
            }
        }
    }

	motion *= vec2(ubo.width, ubo.height) * 0.5;
	vec2 pos_prev = vec2(ipos) + vec2(0.5) + motion.xy;
	if(any(lessThan(ivec2(pos_prev), ivec2(1)))
	|| any(greaterThanEqual(ivec2(pos_prev), ivec2(ubo.width, ubo.height) - 1))) {
		return color_center;
	}

    //vec3 color_prev = textureLod(tex_color_prev, pos_prev / vec2(ubo.width, ubo.height), 0).rgb;
    vec3 color_prev = sample_texture_catmull_rom(taaASVGF_Prev, pos_prev).rgb;


    vec3 sigma = sqrt(max(vec3(0), mom2 - mom1 * mom1));
    vec3 mi = mom1 - sigma;
    vec3 ma = mom1 + sigma;

    if(any(isnan(color_prev)))
        color_prev = vec3(0);

    color_prev = clamp(color_prev, mi, ma);

    return vec4(mix(color_prev, color_center.rgb, 0.1), color_center.a);
}