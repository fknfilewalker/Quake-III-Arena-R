
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



bool
found_intersection(RayPayload rp)
{
	return rp.instanceID != ~0u;
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