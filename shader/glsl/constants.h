#ifndef  _CONSTANTS_H_
#define  _CONSTANTS_H_

//
#define BAS_DEFAULT                                 (0)
#define BAS_WORLD_STATIC                            (1)
#define BAS_WORLD_DYNAMIC_DATA                      (2)
#define BAS_WORLD_DYNAMIC_AS                        (3)
#define BAS_ENTITY_STATIC                           (4)
#define BAS_ENTITY_DYNAMIC                          (5)

// texture
#define TEXTURE_DEFAULT 							0x00000000
#define TEXTURE_ADD 								0x00000001
#define TEXTURE_MUL 								0x00000002

// cullMask
#define RAY_FIRST_PERSON_OPAQUE_VISIBLE 			0x00000001
#define RAY_MIRROR_OPAQUE_VISIBLE 					0x00000002
#define RAY_FIRST_PERSON_MIRROR_OPAQUE_VISIBLE 		0x00000003

#define RAY_FIRST_PERSON_PARTICLE_VISIBLE 			0x00000004
#define RAY_MIRROR_PARTICLE_VISIBLE 				0x00000008
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
#define MATERIAL_KIND_SKY       				    0x0000000b

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
#define BINDING_OFFSET_GLOBAL_UBO					0x00000000
#define BINDING_OFFSET_AS							0x00000001

#define BINDING_OFFSET_ENVMAP						0x00000002
#define BINDING_OFFSET_UBO_LIGHTS					0x00000003
#define BINDING_OFFSET_BLUE_NOISE					0x00000004
#define BINDING_OFFSET_VIS_DATA					    0x00000005
#define BINDING_OFFSET_LIGHT_VIS_DATA			    0x00000006


#define BINDING_OFFSET_INSTANCE_DATA				0x00000010
#define BINDING_OFFSET_XYZ_WORLD_STATIC             0x00000011
#define BINDING_OFFSET_IDX_WORLD_STATIC             0x00000012
#define BINDING_OFFSET_XYZ_WORLD_DYNAMIC_DATA		0x00000013
#define BINDING_OFFSET_IDX_WORLD_DYNAMIC_DATA		0x00000014
#define BINDING_OFFSET_XYZ_WORLD_DYNAMIC_AS		    0x00000015
#define BINDING_OFFSET_IDX_WORLD_DYNAMIC_AS		    0x00000016
#define BINDING_OFFSET_XYZ_ENTITY_STATIC			0x00000017
#define BINDING_OFFSET_IDX_ENTITY_STATIC            0x00000018
#define BINDING_OFFSET_XYZ_ENTITY_DYNAMIC		    0x00000019
#define BINDING_OFFSET_IDX_ENTITY_DYNAMIC	        0x0000001a

#define BINDING_OFFSET_CLUSTER_WORLD_STATIC         0x0000001b
#define BINDING_OFFSET_CLUSTER_WORLD_DYNAMIC_DATA   0x0000001c
#define BINDING_OFFSET_CLUSTER_WORLD_DYNAMIC_AS     0x0000001d
#define BINDING_OFFSET_CLUSTER_ENTITY_STATIC        0x0000001e


#define BINDING_OFFSET_RESULT_OUTPUT                0x00000020

#define BINDING_OFFSET_RESULT_ACCUMULATION          0x0000002a
#define BINDING_OFFSET_RESULT_ACCUMULATION_PREV     0x0000002b
// g buffer
#define BINDING_OFFSET_GBUFFER_POS                  0x00000021
#define BINDING_OFFSET_GBUFFER_ALBEDO               0x00000022
#define BINDING_OFFSET_GBUFFER_NORMAL               0x00000023
#define BINDING_OFFSET_GBUFFER_REFLECTION           0x00000024
#define BINDING_OFFSET_GBUFFER_OBJECT               0x00000025
#define BINDING_OFFSET_GBUFFER_DEPTH                0x00000026
#define BINDING_OFFSET_GBUFFER_MOTION               0x00000027
#define BINDING_OFFSET_GBUFFER_VIEW_DIR             0x00000028

// prev frame
#define BINDING_OFFSET_XYZ_WORLD_DYNAMIC_DATA_PREV  0x00000040
#define BINDING_OFFSET_IDX_WORLD_DYNAMIC_DATA_PREV  0x00000041
#define BINDING_OFFSET_XYZ_WORLD_DYNAMIC_AS_PREV    0x00000042
#define BINDING_OFFSET_IDX_WORLD_DYNAMIC_AS_PREV    0x00000043
#define BINDING_OFFSET_XYZ_ENTITY_DYNAMIC_PREV      0x00000044
#define BINDING_OFFSET_IDX_ENTITY_DYNAMIC_PREV      0x00000045
#define BINDING_OFFSET_INSTANCE_DATA_PREV		    0x00000046
#define BINDING_OFFSET_GLOBAL_UBO_PREV			    0x00000050

// shader offset
#define SBT_RGEN_PRIMARY_RAYS						0x00000000
#define SBT_RMISS_PRIMARY_RAYS						0x00000001
#define SBT_RCHIT_PRIMARY_RAYS						0x00000002
#define SBT_RAHIT_PRIMARY_RAYS						0x00000003

#define SBT_RMISS_PATH_TRACER						0x00000001
#define SBT_RCHIT_OPAQUE							0x00000002
#define SBT_RAHIT_PARTICLE							0x00000003

#define SBT_RMISS_SHADOW_RAY						0x00000004
#define SBT_RCHIT_SHADOW_RAY						0x00000005
#define SBT_RAHIT_SHADOW_RAY						0x00000006

#define NUM_BOUNCES									(2)
#define RTX_MAX_LIGHTS								(512)

// blue noise
#define NUM_BLUE_NOISE_TEX							(64)
#define BLUE_NOISE_RES								(256)

#define UINT_MAX                                    0xffffffff
#define UINT_TOP_16BITS_MASK                        0xffff0000
#define UINT_BOTTOM_16BITS_MASK                     0x0000ffff

#define TEX_SHIFT_BITS                              (16)
#define TEX0_IDX_MASK                               0x000001ff
#define TEX1_IDX_MASK                               0x01ff0000

#define TEX0_BLEND_MASK                             0x00000200
#define TEX1_BLEND_MASK                             0x02000000
#define TEX0_COLOR_MASK                             0x00000400
#define TEX1_COLOR_MASK                             0x04000000

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
    #define MAT4X3(n) mat4x3 n;
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
    #define MAT4X3(n) float n[12];
    #define UVEC2(n) unsigned int[2] n;
#endif

// holds material/offset/etc data for each AS Instance
STRUCT (
    MAT4    (modelmat)
    UINT    (world)
	UINT    (offsetIDX)
	UINT    (offsetXYZ)
    UINT    (texIdx0)
    UINT    (texIdx1)
	BOOL    (isBrushModel)
	BOOL    (isPlayer)
	UINT    (cluster)
    UINT    (currentInstance)
    UINT    (prevInstance)
    UINT    (buff0)
    UINT    (buff1)
,ASInstanceData)

// holds all vertex data
STRUCT (
    VEC3    (pos)
    UINT    (material)

    VEC4    (normal)

    UINT    (color0)
    UINT    (color1)
    UINT    (color2)
    UINT    (color3)

    VEC2    (uv0)
    VEC2    (uv1)

    VEC2    (uv2)
    VEC2    (uv3)

    UINT     (texIdx0)
    UINT     (texIdx1)
    INT      (cluster)
    UINT     (buff2)
,VertexBuffer)

// global ubo
STRUCT (
    MAT4    (inverseViewMat)
    MAT4    (inverseViewMatPortal)
    MAT4    (inverseProjMat)
    MAT4    (inverseProjMatPortal)
    MAT4    (viewMat)
    MAT4    (projMat)
    VEC4    (camPos)
	BOOL    (hasPortal)
	UINT    (frameIndex)
    INT     (currentCluster)
    INT		(numClusters)
 	INT		(clusterBytes)
    UINT    (numSamples)
    // settings
    UINT    (showIntermediateResults)
    BOOL    (cullLights)
    UINT    (numRandomDL)
    UINT    (numRandomIL)
    UINT    (numBounces)
    BOOL    (randSampleLight)
    UINT    (accumulate)
    // dof
    FLOAT   (aperture)
    FLOAT   (focalLength)
    UINT    (dof)
    // anti aliasing
    BOOL    (randomPixelOffset)
,GlobalUbo)

// holds a light
STRUCT (
    VEC4    (pos)
    UINT    (offsetIDX)
    UINT    (offsetXYZ)
    UINT    (type)
    INT     (cluster)
    VEC4    (color)
,Light)

STRUCT (
    Light   lights[RTX_MAX_LIGHTS];
    UINT    (numLights)
,LightList_s)

// STRUCT (
//     INT		(numClusters)
// 	INT		(clusterBytes)
// 	BYTE    (vis)
// ,Vis_s)

#undef STRUCT
#undef BOOL
#undef UINT
#undef FLOAT
#undef VEC2
#undef VEC3
#undef VEC4

#endif /*_CONSTANTS_H_*/