layout(binding = 0, set = 1) uniform sampler2D texure_array[];

vec4
global_textureLod(uint idx, vec2 tex_coord, uint lod)
{
	return textureLod(texure_array[idx], tex_coord, lod);
}

vec4
global_textureGrad(uint idx, vec2 tex_coord, vec2 d_x, vec2 d_y)
{
	return textureGrad(texure_array[idx], tex_coord, d_x, d_y);
}