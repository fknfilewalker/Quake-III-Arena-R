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
#define MATERIAL_FLAG_PLAYER_OR_WEAPON			    0x00001000

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
#define RTX_MAX_LIGHTS								(256)

// blue noise
#define NUM_BLUE_NOISE_TEX							(32)
#define BLUE_NOISE_RES								(256)

#define UINT_MAX                                    0xffffffff
#define UINT_TOP_16BITS_MASK                        0xffff0000
#define UINT_BOTTOM_16BITS_MASK                     0x0000ffff

#define TEX_SHIFT_BITS                              10
#define TEX0_IDX_MASK                               0x000001ff
#define TEX1_IDX_MASK                               0x0007FC00
#define TEX2_IDX_MASK                               0x1ff00000

#define TEX0_BLEND_MASK                             0x00000200
#define TEX1_BLEND_MASK                             0x00080000
#define TEX2_BLEND_MASK                             0x20000000

// shared structures between GLSL and C
#ifdef GLSL
    #define STRUCT(content, name) struct name { content };
    #define BOOL(n) bool n;
    #define INT(n) int n;
    #define UINT(n) uint n;
    #define FLOAT(n) float n;
    #define VEC2(n) vec2 n;
    #define VEC3(n) vec3 n;
    #define VEC4(n) vec4 n;
    #define MAT4(n) mat4 n;
    #define UVEC2(n) uvec2 n;
#else
    #define STRUCT(content, name) typedef struct { content } name;
    #define BOOL(n) unsigned int n;
    #define INT(n) int n;
    #define UINT(n) unsigned int n;
    #define FLOAT(n) float n;
    #define VEC2(n) float n[2];
    #define VEC3(n) float n[3];
    #define VEC4(n) float n[4];
    #define MAT4(n) float n[16];
    #define UVEC2(n) unsigned int[2] n;
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
    UINT     (texIdx0)
    UINT     (texIdx1)
    UINT     (texIdx2)
    UINT    (material)
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

// holds a light
STRUCT (
    VEC4    (pos)
,Light)

STRUCT (
    Light   lights[RTX_MAX_LIGHTS];
    UINT    (numLights)
,LightList_s)

#undef STRUCT
#undef BOOL
#undef UINT
#undef FLOAT
#undef VEC2
#undef VEC3
#undef VEC4

#endif /*_CONSTANTS_H_*/