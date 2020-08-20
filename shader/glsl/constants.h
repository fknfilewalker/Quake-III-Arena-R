#ifndef  _CONSTANTS_H_
#define  _CONSTANTS_H_

//
#define BAS_DEFAULT                                 (0)
#define BAS_WORLD_STATIC                            (1)
#define BAS_WORLD_DYNAMIC_DATA                      (2)
#define BAS_WORLD_DYNAMIC_AS                        (3)
#define BAS_ENTITY_STATIC                           (4)
#define BAS_ENTITY_DYNAMIC                          (5)

#define GRAD_DWN (3)

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

#define RAY_GLASS_VISIBLE 	                        0x00000010

#define MATERIAL_KIND_MASK          				0x0000000f
#define MATERIAL_KIND_INVALID       				0x00000000
#define MATERIAL_KIND_REGULAR       				0x00000001
#define MATERIAL_KIND_WATER        					0x00000002
#define MATERIAL_KIND_CHROME						0x00000003
#define MATERIAL_KIND_LAVA        					0x00000004
#define MATERIAL_KIND_SLIME        					0x00000005
#define MATERIAL_KIND_FOG        					0x00000006
#define MATERIAL_KIND_GLASS       					0x00000007
#define MATERIAL_KIND_INVISIBLE						0x00000008
#define MATERIAL_KIND_TRANSPARENT					0x00000009
#define MATERIAL_KIND_SCREEN						0x0000000a
#define MATERIAL_KIND_SKY       				    0x0000000b

#define MATERIAL_FLAG_MASK          				0x000ffff0
#define MATERIAL_FLAG_LIGHT 						0x00000010
#define MATERIAL_FLAG_TRANSPARENT  					0x00000020
#define MATERIAL_FLAG_SEE_THROUGH  					0x00000040
#define MATERIAL_FLAG_MIRROR 						0x00000080
#define MATERIAL_FLAG_NEEDSCOLOR 					0x00000100
#define MATERIAL_FLAG_PARTICLE 						0x00000200
#define MATERIAL_FLAG_PORTAL						0x00000400
#define MATERIAL_FLAG_BULLET_MARK					0x00000800
#define MATERIAL_FLAG_PLAYER_OR_WEAPON			    0x00001000
#define MATERIAL_FLAG_SEE_THROUGH_ADD  				0x00002000

// BINDING OFFSETS
// ubo
#define BINDING_OFFSET_GLOBAL_UBO					        0x00000000
// top AS
#define BINDING_OFFSET_AS							        0x00000001
// environment map
#define BINDING_OFFSET_ENVMAP						        0x00000002
// blue noise textures
#define BINDING_OFFSET_BLUE_NOISE					        0x00000003
// lights
#define BINDING_OFFSET_UBO_LIGHTS					        0x00000004
#define BINDING_OFFSET_VIS_DATA					            0x00000005
#define BINDING_OFFSET_LIGHT_VIS_DATA			            0x00000006
#define BINDING_OFFSET_LIGHT_VIS_DATA2			            0x0000009a
// vertex data
#define BINDING_OFFSET_INSTANCE_DATA				        0x00000010
#define BINDING_OFFSET_XYZ_WORLD_STATIC                     0x00000011
#define BINDING_OFFSET_IDX_WORLD_STATIC                     0x00000012
#define BINDING_OFFSET_XYZ_WORLD_DYNAMIC_DATA		        0x00000013
#define BINDING_OFFSET_IDX_WORLD_DYNAMIC_DATA		        0x00000014
#define BINDING_OFFSET_XYZ_WORLD_DYNAMIC_AS		            0x00000015
#define BINDING_OFFSET_IDX_WORLD_DYNAMIC_AS		            0x00000016
#define BINDING_OFFSET_XYZ_ENTITY_STATIC			        0x00000017
#define BINDING_OFFSET_IDX_ENTITY_STATIC                    0x00000018
#define BINDING_OFFSET_XYZ_ENTITY_DYNAMIC		            0x00000019
#define BINDING_OFFSET_IDX_ENTITY_DYNAMIC	                0x0000001a
// cluster data
#define BINDING_OFFSET_CLUSTER_WORLD_STATIC                 0x0000001b
#define BINDING_OFFSET_CLUSTER_WORLD_DYNAMIC_DATA           0x0000001c
#define BINDING_OFFSET_CLUSTER_WORLD_DYNAMIC_AS             0x0000001d
#define BINDING_OFFSET_CLUSTER_ENTITY_STATIC                0x0000001e

// result image
#define BINDING_OFFSET_RESULT_OUTPUT                        0x00000023
// accumulation
#define BINDING_OFFSET_RESULT_ACCUMULATION                  0x00000024
#define BINDING_OFFSET_RESULT_ACCUMULATION_PREV             0x00000025

// g buffer
#define BINDING_OFFSET_GBUFFER_POS                          0x00000030
#define BINDING_OFFSET_GBUFFER_ALBEDO                       0x00000031
#define BINDING_OFFSET_GBUFFER_NORMAL                       0x00000032
#define BINDING_OFFSET_GBUFFER_REFLECTION                   0x00000033
#define BINDING_OFFSET_GBUFFER_OBJECT                       0x00000034
#define BINDING_OFFSET_GBUFFER_DEPTH                        0x00000035
#define BINDING_OFFSET_GBUFFER_MOTION                       0x00000036
#define BINDING_OFFSET_GBUFFER_VIEW_DIR                     0x00000037
#define BINDING_OFFSET_GBUFFER_TRANSPARENT                  0x00000038
#define BINDING_OFFSET_GBUFFER_DEPTH_NORMAL                 0x0000005c
#define BINDING_OFFSET_GBUFFER_DIRECT_ILLUMINATION          0x00000039
#define BINDING_OFFSET_GBUFFER_INDIRECT_ILLUMINATION        0x00000071
// forward projection
#define BINDING_OFFSET_INSTANCE_PREV_TO_CURR                0x00000044
#define BINDING_OFFSET_POS_FWD                              0x0000004e
#define BINDING_OFFSET_OBJECT_FWD                           0x0000004f
// a-svgf
#define BINDING_OFFSET_ASVGF_DEBUG                          0x00000045
#define BINDING_OFFSET_ASVGF_GRAD_A                         0x00000046
#define BINDING_OFFSET_ASVGF_GRAD_B                         0x00000047
#define BINDING_OFFSET_ASVGF_GRAD_A_SAMPLER                 0x00000048
#define BINDING_OFFSET_ASVGF_GRAD_B_SAMPLER                 0x00000049
#define BINDING_OFFSET_ASVGF_GRAD_SMPL_POS                  0x0000004a
#define BINDING_OFFSET_ASVGF_RNG                            0x0000004c
#define BINDING_OFFSET_ASVGF_HIST_COLOR                     0x00000050
#define BINDING_OFFSET_ASVGF_HIST_COLOR_S                   0x00000051
#define BINDING_OFFSET_ASVGF_HIST_MOMENTS                   0x00000052
#define BINDING_OFFSET_ASVGF_ATROUS_A                       0x00000054
#define BINDING_OFFSET_ASVGF_ATROUS_B                       0x00000055
#define BINDING_OFFSET_ASVGF_ATROUS_A_SAMPLER               0x00000056
#define BINDING_OFFSET_ASVGF_ATROUS_B_SAMPLER               0x00000057
#define BINDING_OFFSET_ASVGF_TAA                            0x00000059
#define BINDING_OFFSET_ASVGF_TAA_SAMPLER                    0x00000058
#define BINDING_OFFSET_ASVGF_COLOR                          0x0000005b

// PREV FRAME
// ubo
#define BINDING_OFFSET_GLOBAL_UBO_PREV			            0x00000070
// g buffer
#define BINDING_OFFSET_GBUFFER_OBJECT_PREV                  0x00000040
#define BINDING_OFFSET_GBUFFER_DIRECT_ILLUMINATION_PREV     0x00000041
#define BINDING_OFFSET_GBUFFER_NORMAL_PREV                  0x00000042
#define BINDING_OFFSET_GBUFFER_VIEW_DIR_PREV                0x00000043
// asvgf
#define BINDING_OFFSET_ASVGF_RNG_PREV                       0x0000004d
#define BINDING_OFFSET_ASVGF_GRAD_SMPL_POS_PREV             0x0000004b
#define BINDING_OFFSET_ASVGF_HIST_COLOR_PREV                0x00000090
#define BINDING_OFFSET_ASVGF_HIST_MOMENTS_PREV              0x00000053
#define BINDING_OFFSET_ASVGF_TAA_PREV                       0x0000005a
// vertex data
#define BINDING_OFFSET_INSTANCE_DATA_PREV		            0x00000066
#define BINDING_OFFSET_XYZ_WORLD_DYNAMIC_DATA_PREV          0x00000060
#define BINDING_OFFSET_IDX_WORLD_DYNAMIC_DATA_PREV          0x00000061
#define BINDING_OFFSET_XYZ_WORLD_DYNAMIC_AS_PREV            0x00000062
#define BINDING_OFFSET_IDX_WORLD_DYNAMIC_AS_PREV            0x00000063
#define BINDING_OFFSET_XYZ_ENTITY_DYNAMIC_PREV              0x00000064
#define BINDING_OFFSET_IDX_ENTITY_DYNAMIC_PREV              0x00000065

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

#define RTX_MAX_LIGHTS								(512)

// blue noise
#define NUM_BLUE_NOISE_TEX							(128)
#define BLUE_NOISE_RES								(256)

#define UINT_MAX                                    0xffffffff
#define UINT_TOP_16BITS_MASK                        0xffff0000
#define UINT_BOTTOM_16BITS_MASK                     0x0000ffff

#define TEX_SHIFT_BITS                              (16)
#define TEX0_IDX_MASK                               0x000001ff
#define TEX1_IDX_MASK                               0x01ff0000

#define TEX0_COLOR_MASK                             0x00000200
#define TEX1_COLOR_MASK                             0x02000000
#define TEX0_NORMAL_BLEND_MASK                      0x00000400
#define TEX0_MUL_BLEND_MASK                         0x00000800
#define TEX0_ADD_BLEND_MASK                         0x00001000
#define TEX1_NORMAL_BLEND_MASK                      0x04000000
#define TEX1_MUL_BLEND_MASK                         0x08000000
#define TEX1_ADD_BLEND_MASK                         0x10000000
#define TEX0_BLEND_MASK                             0x00001C00
#define TEX1_BLEND_MASK                             0x1C000000

#ifdef GLSL
    #define M_PI 3.14159265358979323846264338327950288f
#endif

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
    UINT    (currInstanceID)
    UINT    (prevInstanceID)
    UINT    (type)
	UINT    (offsetIDX)
	UINT    (offsetXYZ)
    UINT    (texIdx0)
    UINT    (texIdx1)
	BOOL    (isBrushModel)
	BOOL    (isPlayer)
	UINT    (cluster)
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
    UINT    (maxSamples)
    UINT    (width)
    UINT    (height)
    // settings
    UINT    (showIntermediateResults)
    BOOL    (cullLights)
    UINT    (numRandomDL)
    UINT    (numRandomIL)
    UINT    (numBounces)
    BOOL    (illumination)
    BOOL    (randSampleLight)
    UINT    (accumulate)
    UINT    (taa)
    BOOL    (debugLights)
    // dof
    FLOAT   (aperture)
    FLOAT   (focalLength)
    UINT    (dof)
    // anti aliasing
    BOOL    (randomPixelOffset)
    // asvgf
    BOOL    (denoiser)
,GlobalUbo)

// holds a light
STRUCT (
    VEC4    (pos)
    UINT    (offsetIDX)
    UINT    (offsetXYZ)
    UINT    (type)
    INT     (cluster)
    VEC4    (color)
    VEC3    (normal)
    FLOAT   (size)
    VEC4    (AB)
    VEC4    (AC)
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