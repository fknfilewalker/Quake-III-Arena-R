#version 460
#extension GL_NV_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable

layout(set = 1, binding = 0) uniform sampler2D tex[];

struct RayPayload {
  vec4 color;
  vec4 normal;
  float distance;
};
layout(location = 0) rayPayloadInNV RayPayload rp;
hitAttributeNV vec2 attribs;

struct v_s
{
  vec4 pos;
  vec4 uv;
  vec4 color;
};
layout(binding = 2, set = 0) buffer Vertices { v_s v[]; } vertices;

layout(binding = 3, set = 0) buffer Indices { uint i[]; } indices;


void main()
{

	ivec3 index = ivec3(indices.i[3 * gl_PrimitiveID], indices.i[3 * gl_PrimitiveID + 1], indices.i[3 * gl_PrimitiveID + 2]);


  	const vec3 barycentricCoords = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);

	//uint vertId = 3 * gl_PrimitiveID;
	vec4 uv = 		vertices.v[index.x].uv * barycentricCoords.x +
                  	vertices.v[index.y].uv * barycentricCoords.y +
                  	vertices.v[index.z].uv * barycentricCoords.z;

  vec4 c =     vertices.v[index.x].color * barycentricCoords.x +
                    vertices.v[index.y].color * barycentricCoords.y +
                    vertices.v[index.z].color * barycentricCoords.z;
    //float texturePos = (vertices.v[index.x].pos.w); 
    float texturePos = (vertices.v[index.x].pos.w); 

    vec4 color;
    //if(gl_PrimitiveID  > 10) color =  vec4(1,0,0,1);
  	
    color = (c/255) * texture(tex[uint(texturePos)], uv.xy);
  	rp.color = color;//barycentricCoords;
    rp.distance = gl_RayTmaxNV;

    vec3 AB = vertices.v[index.y].pos.xyz - vertices.v[index.x].pos.xyz;
    vec3 AC = vertices.v[index.z].pos.xyz - vertices.v[index.x].pos.xyz;

    rp.normal = vec4(normalize(cross(AB, AC)),1);//vertices.v[index.x].normal;
}