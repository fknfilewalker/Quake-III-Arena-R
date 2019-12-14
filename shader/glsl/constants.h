#ifndef  _CONSTANTS_H_
#define  _CONSTANTS_H_

// texture
#define TEXTURE_DEFAULT 							0x00000000
#define TEXTURE_ADD 								0x00000001
#define TEXTURE_MUL 								0x00000002

// cullMask
#define RAY_FIRST_PERSON_OPAQUE_VISIBLE 			0x00000001
#define RAY_MIRROR_OPAQUE_VISIBLE 					0x00000002
#define RAY_FIRST_PERSON_PARTICLE_VISIBLE 			0x00000004
#define RAY_MIRROR_PARTICLE_VISIBLE 				0x00000008
#define RAY_FIRST_PERSON_MIRROR_OPAQUE_VISIBLE 		0x00000003
#define RAY_FIRST_PERSON_MIRROR_PARTICLE_VISIBLE 	0x0000000c

#define MATERIAL_KIND_MASK          				0x0000000f
#define MATERIAL_KIND_INVALID       				0x00000000
#define MATERIAL_KIND_REGULAR       				0x00000001
#define MATERIAL_KIND_CHROME						0x00000002
#define MATERIAL_KIND_LAVA        					0x00000003
#define MATERIAL_KIND_SLIME        					0x00000004
#define MATERIAL_KIND_WATER        					0x00000005
#define MATERIAL_KIND_FOG        					0x00000006
#define MATERIAL_KIND_GLASS       					0x00000007
#define MATERIAL_KIND_INVISIBLE						0x00000008
#define MATERIAL_KIND_TRANSPARENT					0x00000009
#define MATERIAL_KIND_SCREEN						0x0000000a

#define MATERIAL_FLAG_MASK          				0x0000fff0
#define MATERIAL_FLAG_LIGHT 						0x00000010
#define MATERIAL_FLAG_TRANSPARENT  					0x00000020
#define MATERIAL_FLAG_SEE_THROUGH  					0x00000040
#define MATERIAL_FLAG_MIRROR 						0x00000080
#define MATERIAL_FLAG_NEEDSCOLOR 					0x00000100
#define MATERIAL_FLAG_PARTICLE 						0x00000200
#define MATERIAL_FLAG_PORTAL						0x00000400
#define MATERIAL_FLAG_BULLET_MARK					0x00000800

// binding offset
#define BINDING_OFFSET_AS							0x00000000
#define BINDING_OFFSET_XYZ_STATIC					0x00000002
#define BINDING_OFFSET_IDX_STATIC					0x00000003
#define BINDING_OFFSET_ENVMAP						0x00000005
#define BINDING_OFFSET_UBO_LIGHTS					0x00000007
#define BINDING_OFFSET_XYZ_DYNAMIC					0x00000009
#define BINDING_OFFSET_IDX_DYNAMIC					0x0000000a
#define BINDING_OFFSET_INSTANCE_DATA				0x00000004
#define BINDING_OFFSET_BLUE_NOISE					0x00000008

// shader offset
#define SBT_RGEN_PRIMARY_RAYS						0x00000000
#define SBT_RMISS_PATH_TRACER						0x00000001
#define SBT_RCHIT_OPAQUE							0x00000002
#define SBT_RAHIT_PARTICLE							0x00000003

#define NUM_BOUNCES									(2)
#define RTX_MAX_LIGHTS								(128)

// blue noise
#define NUM_BLUE_NOISE_TEX							(32)
#define BLUE_NOISE_RES								(256)

// shared structures between GLSL and C
#ifdef GLSL
    #define STRUCT(content, name) struct name { content };
    #define BOOL(n) bool n;
    #define UINT(n) uint n;
    #define FLOAT(n) float n;
    #define VEC2(n) vec2 n;
    #define VEC3(n) vec3 n;
    #define VEC4(n) vec4 n;
    #define MAT4(n) mat4 n;
#else
    #define STRUCT(content, name) typedef struct { content } name;
    #define BOOL(n) unsigned int n;
    #define UINT(n) unsigned int n;
    #define FLOAT(n) float n;
    #define VEC2(n) float n[2];
    #define VEC3(n) float n[3];
    #define VEC4(n) float n[4];
    #define MAT4(n) float n[16];
#endif

// holds material/offset/etc data for each AS Instance
STRUCT (
    BOOL    (world)
	BOOL    (dynamic)
	UINT    (offsetIDX)
	UINT    (offsetXYZ)
	UINT    (texIdx)
	UINT    (material)
	UINT    (blendfunc)
	FLOAT   (opaque)
	UINT    (type)
,ASInstanceData)

// holds all vertex data
STRUCT (
    VEC4    (pos)
    VEC4    (uv)
    VEC4    (color)
,VertexBuffer)

// global ubo
STRUCT (
    MAT4    (inverseViewMat)
    MAT4    (inverseViewMatPortal)
    MAT4    (inverseProjMat)
    MAT4    (inverseProjMatPortal)
    MAT4    (viewMat)
    MAT4    (projMat)
	BOOL    (hasPortal)
	UINT    (frameIndex)
,RTUbo)

#undef STRUCT
#undef BOOL
#undef UINT
#undef FLOAT
#undef VEC2
#undef VEC3
#undef VEC4

#endif /*_CONSTANTS_H_*/