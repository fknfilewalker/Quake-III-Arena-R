/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// tr_init.c -- functions that are not called every frame

#include "tr_local.h"

glconfig_t	glConfig;
glstate_t	glState;

trApi_t		tr_api;

static void GfxInfo_f( void );

cvar_t	*r_flareSize;
cvar_t	*r_flareFade;

cvar_t	*r_railWidth;
cvar_t	*r_railCoreWidth;
cvar_t	*r_railSegmentLength;

cvar_t	*r_ignoreFastPath;

cvar_t	*r_verbose;
cvar_t	*r_ignore;

cvar_t	*r_displayRefresh;

cvar_t	*r_detailTextures;

cvar_t	*r_znear;

cvar_t	*r_smp;
cvar_t	*r_showSmp;
cvar_t	*r_skipBackEnd;

cvar_t	*r_ignorehwgamma;
cvar_t	*r_measureOverdraw;

cvar_t	*r_inGameVideo;
cvar_t	*r_fastsky;
cvar_t	*r_drawSun;
cvar_t	*r_dynamiclight;
cvar_t	*r_dlightBacks;

cvar_t	*r_lodbias;
cvar_t	*r_lodscale;

cvar_t	*r_norefresh;
cvar_t	*r_drawentities;
cvar_t	*r_drawworld;
cvar_t	*r_speeds;
cvar_t	*r_fullbright;
cvar_t	*r_novis;
cvar_t	*r_nocull;
cvar_t	*r_facePlaneCull;
cvar_t	*r_showcluster;
cvar_t	*r_nocurves;

cvar_t	*r_allowExtensions;

cvar_t	*r_ext_compressed_textures;
cvar_t	*r_ext_gamma_control;
cvar_t	*r_ext_multitexture;
cvar_t	*r_ext_compiled_vertex_array;
cvar_t	*r_ext_texture_env_add;

cvar_t	*r_ignoreGLErrors;
cvar_t	*r_logFile;

cvar_t	*r_stencilbits;
cvar_t	*r_depthbits;
cvar_t	*r_colorbits;
cvar_t	*r_stereo;
cvar_t	*r_primitives;
cvar_t	*r_texturebits;

cvar_t	*r_drawBuffer;
cvar_t  *r_glDriver;
cvar_t	*r_lightmap;
cvar_t	*r_vertexLight;
cvar_t	*r_rtx;
cvar_t	*r_uiFullScreen;
cvar_t	*r_shadows;
cvar_t	*r_flares;
cvar_t	*r_mode;
cvar_t	*r_nobind;
cvar_t	*r_singleShader;
cvar_t	*r_roundImagesDown;
cvar_t	*r_colorMipLevels;
cvar_t	*r_picmip;
cvar_t	*r_showtris;
cvar_t	*r_showsky;
cvar_t	*r_shownormals;
cvar_t	*r_finish;
cvar_t	*r_clear;
cvar_t	*r_swapInterval;
cvar_t	*r_textureMode;
cvar_t	*r_offsetFactor;
cvar_t	*r_offsetUnits;
cvar_t	*r_gamma;
cvar_t	*r_intensity;
cvar_t	*r_lockpvs;
cvar_t	*r_noportals;
cvar_t	*r_portalOnly;

cvar_t	*r_subdivisions;
cvar_t	*r_lodCurveError;

cvar_t	*r_fullscreen;

cvar_t	*r_customwidth;
cvar_t	*r_customheight;
cvar_t	*r_customaspect;

cvar_t	*r_overBrightBits;
cvar_t	*r_mapOverBrightBits;

cvar_t	*r_debugSurface;
cvar_t	*r_simpleMipMaps;

cvar_t	*r_showImages;

cvar_t	*r_ambientScale;
cvar_t	*r_directedScale;
cvar_t	*r_debugLight;
cvar_t	*r_debugSort;
cvar_t	*r_printShaders;
cvar_t	*r_saveFontData;

cvar_t	*r_maxpolys;
int		max_polys;
cvar_t	*r_maxpolyverts;
int		max_polyverts;

// PT
cvar_t* pt_showIntermediateResults;
cvar_t* pt_numRandomIL;
cvar_t* rt_illumination;

cvar_t* rt_cullLights;
cvar_t* rt_numRandomDL;
cvar_t* rt_numRandomIL;
cvar_t* rt_numBounces;
cvar_t* rt_printPerformanceStatistic;
cvar_t* rt_accumulate;
cvar_t* rt_pause;
cvar_t* rt_debug_lights;
cvar_t* rt_antialiasing;
cvar_t* rt_softshadows;
cvar_t* rt_numSamples;
cvar_t* rt_maxSamples;
cvar_t* rt_taa;
cvar_t* rt_aperture;
cvar_t* rt_focalLength;
cvar_t* rt_dof;
cvar_t* rt_denoiser;
cvar_t* rt_brightness;
cvar_t* rt_tonemapping_reinhard;

void ( APIENTRY * qglMultiTexCoord2fARB )( GLenum texture, GLfloat s, GLfloat t );
void ( APIENTRY * qglActiveTextureARB )( GLenum texture );
void ( APIENTRY * qglClientActiveTextureARB )( GLenum texture );

void ( APIENTRY * qglLockArraysEXT)( GLint, GLint);
void ( APIENTRY * qglUnlockArraysEXT) ( void );

static void AssertCvarRange( cvar_t *cv, float minVal, float maxVal, qboolean shouldBeIntegral )
{
	if ( shouldBeIntegral )
	{
		if ( ( int ) cv->value != cv->integer )
		{
			ri.Printf( PRINT_WARNING, "WARNING: cvar '%s' must be integral (%f)\n", cv->name, cv->value );
			ri.Cvar_Set( cv->name, va( "%d", cv->integer ) );
		}
	}

	if ( cv->value < minVal )
	{
		ri.Printf( PRINT_WARNING, "WARNING: cvar '%s' out of range (%f < %f)\n", cv->name, cv->value, minVal );
		ri.Cvar_Set( cv->name, va( "%f", minVal ) );
	}
	else if ( cv->value > maxVal )
	{
		ri.Printf( PRINT_WARNING, "WARNING: cvar '%s' out of range (%f > %f)\n", cv->name, cv->value, maxVal );
		ri.Cvar_Set( cv->name, va( "%f", maxVal ) );
	}
}


/*
** InitOpenGL
**
** This function is responsible for initializing a valid OpenGL subsystem.  This
** is done by calling GLimp_Init (which gives us a working OGL subsystem) then
** setting variables, checking GL constants, and reporting the gfx system config
** to the user.
*/
static void InitOpenGL( void )
{
	char renderer_buffer[1024];

	//
	// initialize OS specific portions of the renderer
	//
	// GLimp_Init directly or indirectly references the following cvars:
	//		- r_fullscreen
	//		- r_glDriver
	//		- r_mode
	//		- r_(color|depth|stencil)bits
	//		- r_ignorehwgamma
	//		- r_gamma
	//
	
	glConfig.driverType = OPENGL;

	if ( glConfig.vidWidth == 0 )
	{

		GLint		temp;
		
		R_SetOpenGLApi(&tr_api);
		GLimp_Init();

		strcpy( renderer_buffer, glConfig.renderer_string );
		Q_strlwr( renderer_buffer );

		// OpenGL driver constants
		qglGetIntegerv( GL_MAX_TEXTURE_SIZE, &temp );
		glConfig.maxTextureSize = temp;

		// stubbed or broken drivers may have reported 0...
		if ( glConfig.maxTextureSize <= 0 ) 
		{
			glConfig.maxTextureSize = 0;
		}
	}

	// init command buffers and SMP
	R_InitCommandBuffers();

	// print info
	GfxInfo_f();

	// set default state
	GL_SetDefaultState();
}

static void InitVulkan(void)
{
	glConfig.driverType = VULKAN;
	
	if (glConfig.vidWidth == 0)
	{
		R_SetVulkanApi(&tr_api);

		VKimp_Init();

		VK_InitPipelines();

		VK_CreateIndexBuffer(&vk_d.indexbuffer, vk.swapchain.imageCount * VK_INDEX_DATA_SIZE * sizeof(uint32_t));
		VK_CreateVertexBuffer(&vk_d.vertexbuffer, vk.swapchain.imageCount * VK_VERTEX_ATTRIBUTE_DATA_SIZE * sizeof(vec4_t));
		VK_CreateVertexBuffer(&vk_d.normalbuffer, vk.swapchain.imageCount * VK_VERTEX_ATTRIBUTE_DATA_SIZE * sizeof(vec4_t));
		VK_CreateVertexBuffer(&vk_d.uvbuffer1, vk.swapchain.imageCount * VK_VERTEX_ATTRIBUTE_DATA_SIZE * sizeof(vec2_t));
        VK_CreateVertexBuffer(&vk_d.uvbuffer2, vk.swapchain.imageCount * VK_VERTEX_ATTRIBUTE_DATA_SIZE * sizeof(vec2_t));
		VK_CreateVertexBuffer(&vk_d.colorbuffer, vk.swapchain.imageCount * VK_VERTEX_ATTRIBUTE_DATA_SIZE * sizeof(color4ub_t));

		vk_d.mipmapLevel = 1 + floor(log2(max(vk.swapchain.extent.width, vk.swapchain.extent.height)));

		// <RTX>
		if (glConfig.driverType == VULKAN && r_vertexLight->value == 2) {
			for (int i = 0; i < vk.swapchain.imageCount; i++) {
				// gbuffer
				VK_CreateImage(&vk_d.gBuffer[i].position, vk.swapchain.extent.width, vk.swapchain.extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1);
				VK_CreateSampler(&vk_d.gBuffer[i].position, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
				VK_TransitionImage(&vk_d.gBuffer[i].position, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

				VK_CreateImage(&vk_d.gBuffer[i].albedo, vk.swapchain.extent.width, vk.swapchain.extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1);
				VK_CreateSampler(&vk_d.gBuffer[i].albedo, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
				VK_TransitionImage(&vk_d.gBuffer[i].albedo, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

				/*VK_CreateImage(&vk_d.gBuffer[i].color, vk.swapchain.extent.width, vk.swapchain.extent.height, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1);
				VK_CreateSampler(&vk_d.gBuffer[i].color, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
				VK_TransitionImage(&vk_d.gBuffer[i].color, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);*/

				VK_CreateImage(&vk_d.gBuffer[i].normals, vk.swapchain.extent.width, vk.swapchain.extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1);
				VK_CreateSampler(&vk_d.gBuffer[i].normals, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
				VK_TransitionImage(&vk_d.gBuffer[i].normals, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

				VK_CreateImage(&vk_d.gBuffer[i].viewDir, vk.swapchain.extent.width, vk.swapchain.extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1);
				VK_CreateSampler(&vk_d.gBuffer[i].viewDir, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
				VK_TransitionImage(&vk_d.gBuffer[i].viewDir, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

				VK_CreateImage(&vk_d.gBuffer[i].transparent, vk.swapchain.extent.width, vk.swapchain.extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1);
				VK_CreateSampler(&vk_d.gBuffer[i].transparent, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
				VK_TransitionImage(&vk_d.gBuffer[i].transparent, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

				VK_CreateImage(&vk_d.gBuffer[i].reflection, vk.swapchain.extent.width, vk.swapchain.extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1);
				VK_CreateSampler(&vk_d.gBuffer[i].reflection, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
				VK_TransitionImage(&vk_d.gBuffer[i].reflection, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

				VK_CreateImage(&vk_d.gBuffer[i].directIllumination, vk.swapchain.extent.width, vk.swapchain.extent.height, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1);
				VK_CreateSampler(&vk_d.gBuffer[i].directIllumination, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
				VK_TransitionImage(&vk_d.gBuffer[i].directIllumination, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

				VK_CreateImage(&vk_d.gBuffer[i].indirectIllumination, vk.swapchain.extent.width, vk.swapchain.extent.height, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1);
				VK_CreateSampler(&vk_d.gBuffer[i].indirectIllumination, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
				VK_TransitionImage(&vk_d.gBuffer[i].indirectIllumination, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

				VK_CreateImage(&vk_d.gBuffer[i].objectInfo, vk.swapchain.extent.width, vk.swapchain.extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1);
				VK_CreateSampler(&vk_d.gBuffer[i].objectInfo, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
				VK_TransitionImage(&vk_d.gBuffer[i].objectInfo, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

				VK_CreateImage(&vk_d.gBuffer[i].depthNormal, vk.swapchain.extent.width, vk.swapchain.extent.height, VK_FORMAT_R32G32_SFLOAT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1);
				VK_CreateSampler(&vk_d.gBuffer[i].depthNormal, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
				VK_TransitionImage(&vk_d.gBuffer[i].depthNormal, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

				VK_CreateImage(&vk_d.gBuffer[i].motion, vk.swapchain.extent.width, vk.swapchain.extent.height, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1);
				VK_CreateSampler(&vk_d.gBuffer[i].motion, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
				VK_TransitionImage(&vk_d.gBuffer[i].motion, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

				VK_CreateImageArray(&vk_d.gBuffer[i].maxmipmap, vk.swapchain.extent.width, vk.swapchain.extent.height, VK_FORMAT_R16_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1, vk_d.mipmapLevel);
				//VK_CreateImage(&vk_d.gBuffer[i].maxmipmap, vk.swapchain.extent.width, vk.swapchain.extent.height, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1);
				VK_CreateSampler(&vk_d.gBuffer[i].maxmipmap, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
				VK_TransitionImage(&vk_d.gBuffer[i].maxmipmap, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

				VK_CreateImage(&vk_d.gBuffer[i].result, vk.swapchain.extent.width, vk.swapchain.extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1);
				VK_CreateSampler(&vk_d.gBuffer[i].result, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
				VK_TransitionImage(&vk_d.gBuffer[i].result, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

				// asvgf
				VK_CreateImage(&vk_d.asvgf[i].debug, vk.swapchain.extent.width, vk.swapchain.extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1);
				VK_CreateSampler(&vk_d.asvgf[i].debug, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
				VK_TransitionImage(&vk_d.asvgf[i].debug, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

				VK_CreateImage(&vk_d.asvgf[i].rngSeed, vk.swapchain.extent.width, vk.swapchain.extent.height, VK_FORMAT_R32_UINT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1);
				VK_CreateSampler(&vk_d.asvgf[i].rngSeed, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
				VK_TransitionImage(&vk_d.asvgf[i].rngSeed, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

				VK_CreateImage(&vk_d.asvgf[i].objectFwd, (vk.swapchain.extent.width + GRAD_DWN - 1) / GRAD_DWN, (vk.swapchain.extent.height + GRAD_DWN - 1) / GRAD_DWN, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1);
				VK_CreateSampler(&vk_d.asvgf[i].objectFwd, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
				VK_TransitionImage(&vk_d.asvgf[i].objectFwd, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

				VK_CreateImage(&vk_d.asvgf[i].positionFwd, (vk.swapchain.extent.width + GRAD_DWN - 1) / GRAD_DWN, (vk.swapchain.extent.height + GRAD_DWN - 1) / GRAD_DWN, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1);
				VK_CreateSampler(&vk_d.asvgf[i].positionFwd, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
				VK_TransitionImage(&vk_d.asvgf[i].positionFwd, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

				VK_CreateImage(&vk_d.asvgf[i].color, vk.swapchain.extent.width, vk.swapchain.extent.height, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1);
				VK_CreateSampler(&vk_d.asvgf[i].color, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
				VK_TransitionImage(&vk_d.asvgf[i].color, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

				VK_CreateImage(&vk_d.asvgf[i].gradSamplePos, (vk.swapchain.extent.width + GRAD_DWN - 1) / GRAD_DWN, (vk.swapchain.extent.height + GRAD_DWN - 1) / GRAD_DWN, VK_FORMAT_R32_UINT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1);
				VK_CreateSampler(&vk_d.asvgf[i].gradSamplePos, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
				VK_TransitionImage(&vk_d.asvgf[i].gradSamplePos, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

				VK_CreateImage(&vk_d.asvgf[i].gradA, (vk.swapchain.extent.width + GRAD_DWN - 1) / GRAD_DWN, (vk.swapchain.extent.height + GRAD_DWN - 1) / GRAD_DWN, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT, 1);
				VK_CreateSampler(&vk_d.asvgf[i].gradA, VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT);
				VK_TransitionImage(&vk_d.asvgf[i].gradA, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
				
				VK_CreateImage(&vk_d.asvgf[i].gradB, (vk.swapchain.extent.width + GRAD_DWN - 1) / GRAD_DWN, (vk.swapchain.extent.height + GRAD_DWN - 1) / GRAD_DWN, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT, 1);
				VK_CreateSampler(&vk_d.asvgf[i].gradB, VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT);
				VK_TransitionImage(&vk_d.asvgf[i].gradB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

				VK_CreateImage(&vk_d.asvgf[i].atrousA, (vk.swapchain.extent.width), vk.swapchain.extent.height, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT, 1);
				VK_CreateSampler(&vk_d.asvgf[i].atrousA, VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT);
				VK_TransitionImage(&vk_d.asvgf[i].atrousA, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

				VK_CreateImage(&vk_d.asvgf[i].atrousB, (vk.swapchain.extent.width), vk.swapchain.extent.height, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT, 1);
				VK_CreateSampler(&vk_d.asvgf[i].atrousB, VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT);
				VK_TransitionImage(&vk_d.asvgf[i].atrousB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

				VK_CreateImage(&vk_d.asvgf[i].histColor, vk.swapchain.extent.width, vk.swapchain.extent.height, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT, 1);
				VK_CreateSampler(&vk_d.asvgf[i].histColor, VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT);
				VK_TransitionImage(&vk_d.asvgf[i].histColor, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

				VK_CreateImage(&vk_d.asvgf[i].histMoments, vk.swapchain.extent.width, vk.swapchain.extent.height, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1);
				VK_CreateSampler(&vk_d.asvgf[i].histMoments, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
				VK_TransitionImage(&vk_d.asvgf[i].histMoments, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

				VK_CreateImage(&vk_d.asvgf[i].taa, vk.swapchain.extent.width, vk.swapchain.extent.height, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT, 1);
				VK_CreateSampler(&vk_d.asvgf[i].taa, VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT);
				VK_TransitionImage(&vk_d.asvgf[i].taa, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
			}

			vk_d.bottomASList = calloc(VK_MAX_BOTTOM_AS, sizeof(vkbottomAS_t));
			vk_d.bottomASCount = 0;
			vk_d.bottomASTraceList = calloc(VK_MAX_BOTTOM_AS_INSTANCES, sizeof(vkbottomAS_t));
			vk_d.bottomASTraceListCount = 0;

			// world offsets
			vk_d.geometry.idx_world_static_offset = 0;
			vk_d.geometry.xyz_world_static_offset = 0;
			vk_d.geometry.idx_world_dynamic_data_offset = 0;
			vk_d.geometry.xyz_world_dynamic_data_offset = 0;
			for (int i = 0; i < vk.swapchain.imageCount; i++) {
				vk_d.geometry.idx_world_dynamic_as_offset[i] = 0;
				vk_d.geometry.xyz_world_dynamic_as_offset[i] = 0;
			}
			vk_d.geometry.cluster_world_static_offset = 0;
			vk_d.geometry.cluster_world_dynamic_data_offset = 0;
			vk_d.geometry.cluster_world_dynamic_as_offset = 0;
			vk_d.geometry.cluster_entity_static_offset = 0;
			// entity offsets
			vk_d.geometry.idx_entity_static_offset = 0;
			vk_d.geometry.xyz_entity_static_offset = 0;
			vk_d.geometry.idx_entity_dynamic_offset = 0;
			vk_d.geometry.xyz_entity_dynamic_offset = 0;

			// world buffers
			// static
			VK_CreateRayTracingASBuffer(&vk_d.basBufferStaticWorld, 50 * VK_AS_MEMORY_ALLIGNMENT_SIZE * sizeof(byte));
			VK_CreateAttributeBuffer(&vk_d.geometry.idx_world_static, RTX_WORLD_STATIC_IDX_SIZE * sizeof(uint32_t), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
			VK_CreateAttributeBuffer(&vk_d.geometry.xyz_world_static, RTX_WORLD_STATIC_XYZ_SIZE * sizeof(VertexBuffer), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
			VK_CreateAttributeBuffer(&vk_d.geometry.cluster_world_static, RTX_WORLD_STATIC_IDX_SIZE/3 * sizeof(uint32_t), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

			

			// dynamic data
			for (int i = 0; i < vk.swapchain.imageCount; i++) {
				VK_CreateAttributeBuffer(&vk_d.prevToCurrInstanceBuffer[i], sizeof(vk_d.prevToCurrInstance), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

				VK_CreateAttributeBuffer(&vk_d.geometry.idx_world_dynamic_data[i], RTX_WORLD_DYNAMIC_DATA_IDX_SIZE * sizeof(uint32_t), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
				VK_CreateAttributeBuffer(&vk_d.geometry.xyz_world_dynamic_data[i], RTX_WORLD_DYNAMIC_DATA_XYZ_SIZE * sizeof(VertexBuffer), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
			}
			VK_CreateAttributeBuffer(&vk_d.geometry.cluster_world_dynamic_data, RTX_WORLD_DYNAMIC_DATA_IDX_SIZE / 3 * sizeof(uint32_t), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

			VK_CreateRayTracingASBuffer(&vk_d.basBufferWorldDynamicData, 10 * VK_AS_MEMORY_ALLIGNMENT_SIZE * sizeof(byte));
			// dynamic AS
			vk_d.basBufferEntityDynamicOffset = 0;
			for (int i = 0; i < vk.swapchain.imageCount; i++) {
				VK_CreateAttributeBuffer(&vk_d.geometry.idx_world_dynamic_as[i], RTX_WORLD_DYNAMIC_AS_IDX_SIZE * sizeof(uint32_t), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
				VK_CreateAttributeBuffer(&vk_d.geometry.xyz_world_dynamic_as[i], RTX_WORLD_DYNAMIC_AS_XYZ_SIZE * sizeof(VertexBuffer), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
				VK_CreateRayTracingASBuffer(&vk_d.basBufferWorldDynamicAS[i], 10 * VK_AS_MEMORY_ALLIGNMENT_SIZE * sizeof(byte));
			}
			VK_CreateAttributeBuffer(&vk_d.geometry.cluster_world_dynamic_as, RTX_WORLD_DYNAMIC_AS_IDX_SIZE / 3 * sizeof(uint32_t), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

			// entity
			VK_CreateAttributeBuffer(&vk_d.geometry.idx_entity_static, RTX_ENTITY_STATIC_INDEX_SIZE * sizeof(uint32_t), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
			VK_CreateAttributeBuffer(&vk_d.geometry.xyz_entity_static, RTX_ENTITY_STATIC_XYZ_SIZE * sizeof(VertexBuffer), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
			VK_CreateAttributeBuffer(&vk_d.geometry.cluster_entity_static, RTX_ENTITY_STATIC_INDEX_SIZE / 3 * sizeof(uint32_t), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

			VK_CreateRayTracingASBuffer(&vk_d.basBufferEntityStatic, VK_MAX_BOTTOM_AS * VK_AS_MEMORY_ALLIGNMENT_SIZE * sizeof(byte));
			vk_d.basBufferEntityStaticOffset = 0;
			vk_d.basBufferEntityDynamicOffset = 0;
			for (int i = 0; i < vk.swapchain.imageCount; i++) {
				VK_CreateAttributeBuffer(&vk_d.geometry.idx_entity_dynamic[i], RTX_ENTITY_DYNAMIC_IDX_SIZE * sizeof(uint32_t), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
				VK_CreateAttributeBuffer(&vk_d.geometry.xyz_entity_dynamic[i], RTX_ENTITY_DYNAMIC_XYZ_SIZE * sizeof(VertexBuffer), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
				VK_CreateRayTracingASBuffer(&vk_d.basBufferEntityDynamic[i], 100 * VK_AS_MEMORY_ALLIGNMENT_SIZE * sizeof(byte));
				// Per Frame Dynamic AS List
				vk_d.bottomASDynamicList[i] = calloc(VK_MAX_DYNAMIC_BOTTOM_AS_INSTANCES, sizeof(vkbottomAS_t));
				vk_d.bottomASDynamicCount[i] = 0;
			}

			// stuff we need for each swapchain image
			for (int i = 0; i < vk.swapchain.imageCount; i++) {
				// Top AS Buffer
				VK_CreateRayTracingASBuffer(&vk_d.topASBuffer[i], 20 * VK_AS_MEMORY_ALLIGNMENT_SIZE);
				// Per Instance Buffers
				VK_CreateRayTracingBuffer(&vk_d.instanceBuffer[i], VK_MAX_BOTTOM_AS_INSTANCES * sizeof(VkGeometryInstanceNV));
				VK_CreateAttributeBuffer(&vk_d.instanceDataBuffer[i], VK_MAX_BOTTOM_AS_INSTANCES * sizeof(ASInstanceData), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
				// UBOs
				VK_CreateUniformBuffer(&vk_d.uboBuffer[i], sizeof(GlobalUbo));
				//VK_CreateUniformBuffer(&vk_d.uboLightList[i], sizeof(LightList_s));
				VK_CreateAttributeBuffer(&vk_d.uboLightList[i], sizeof(LightList_s), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

				// End Result Image
				VK_CreateImage(&vk_d.accelerationStructures.resultImage[i], vk.swapchain.extent.width, vk.swapchain.extent.height, vk.swapchain.imageFormat, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1);
				VK_CreateSampler(&vk_d.accelerationStructures.resultImage[i], VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
				VK_TransitionImage(&vk_d.accelerationStructures.resultImage[i], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

				// create descriptor
				VK_AddSampler(&vk_d.accelerationStructures.resultImage[i].descriptor_set, 0, VK_SHADER_STAGE_FRAGMENT_BIT);
				vk_d.accelerationStructures.resultImage[i].descriptor_set.data[0].descImageInfo->imageLayout = VK_IMAGE_LAYOUT_GENERAL;
				VK_SetSampler(&vk_d.accelerationStructures.resultImage[i].descriptor_set, 0, VK_SHADER_STAGE_FRAGMENT_BIT, vk_d.accelerationStructures.resultImage[i].sampler, vk_d.accelerationStructures.resultImage[i].view);
				VK_FinishDescriptor(&vk_d.accelerationStructures.resultImage[i].descriptor_set);
				
				VK_CreateImage(&vk_d.accelerationStructures.accumulationImage[i], vk.swapchain.extent.width, vk.swapchain.extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1);
				VK_CreateSampler(&vk_d.accelerationStructures.accumulationImage[i], VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
				VK_TransitionImage(&vk_d.accelerationStructures.accumulationImage[i], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
			}
			// Scratch buffer for AS build
			VK_CreateRayTracingScratchBuffer(&vk_d.scratchBuffer, VK_MAX_DYNAMIC_BOTTOM_AS_INSTANCES * VK_AS_MEMORY_ALLIGNMENT_SIZE * sizeof(byte));
			vk_d.scratchBufferOffset = 0;
			/*
			// new
			VK_CreateImage(&vk_d.accelerationStructures.rngImage, vk.swapchain.extent.width, vk.swapchain.extent.height, vk.swapchain.imageFormat, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1);
			VK_CreateSampler(&vk_d.accelerationStructures.rngImage, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
			
			for (int i = 0; i < vk.swapchain.imageCount; i++) {
				VK_CreateImage(&vk_d.randomSeedTex[i], vk.swapchain.extent.width, vk.swapchain.extent.height, VK_FORMAT_R32_UINT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1);
				VK_CreateSampler(&vk_d.randomSeedTex[i], VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
				VK_TransitionImage(&vk_d.randomSeedTex[i], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
				
				VK_AddUniformBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_GLOBAL_UBO, VK_SHADER_STAGE_COMPUTE_BIT);
				VK_SetUniformBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_GLOBAL_UBO, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.uboBuffer[i].buffer);
				VK_AddStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_RANDOM_SEED, VK_SHADER_STAGE_COMPUTE_BIT);
				VK_SetStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_RANDOM_SEED, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.randomSeedTex[i].view);
				VK_FinishDescriptor(&vk_d.computeDescriptor[i]);
			}
			vkshader_t rngShader = { 0 };
			VK_RngCompShader(&rngShader);
			VK_SetComputeShader(&vk_d.accelerationStructures.rngPipeline, &rngShader);
			VK_SetComputeDescriptorSet(&vk_d.accelerationStructures.rngPipeline, &vk_d.computeDescriptor[0]);
			VK_FinishComputePipeline(&vk_d.accelerationStructures.rngPipeline);*/
		
		
			// load blue noise
			const int num_blue_noise_images = NUM_BLUE_NOISE_TEX;
			const int resolution = BLUE_NOISE_RES;
			const size_t img_size = (size_t)resolution * (size_t)resolution;
			const size_t total_size = img_size * sizeof(uint16_t);

			int		width, height;
			int		bytes_per_channel = 2;
			byte* pic;
			VK_CreateImageArray(&vk_d.blueNoiseTex, BLUE_NOISE_RES, BLUE_NOISE_RES, VK_FORMAT_R16_UNORM, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1, NUM_BLUE_NOISE_TEX);
			for (int i = 0; i < NUM_BLUE_NOISE_TEX/4; i++) {
				char buf[1024];
				//snprintf(buf, sizeof buf, "blue_noise/LDR_RGBA_%04d.tga", i);
				snprintf(buf, sizeof buf, "blue_noise_textures/%d_%d/HDR_RGBA_%04d.png", BLUE_NOISE_RES, BLUE_NOISE_RES, i);
				//snprintf(buf, sizeof buf, "blue_noise_textures/%d_%d/HDR_RGBA_%01d.png", BLUE_NOISE_RES, BLUE_NOISE_RES, i);
				//snprintf(buf, sizeof buf, "blue_noise_textures/256_256/HDR_RGBA_%04d.png", i);
				R_LoadImage16(buf, &pic, &width, &height);

				// HDR is RGBA
				for (int channel = 0; channel < 4; channel++) {
					uint8_t img[2 * BLUE_NOISE_RES * BLUE_NOISE_RES];
					for (int j = 0; j < img_size; j++) {
						img[(j * bytes_per_channel) + 0] = *(pic + ((j * 8) + ((channel * bytes_per_channel) + 0)));
						img[(j * bytes_per_channel) + 1] = *(pic + ((j * 8) + ((channel * bytes_per_channel) + 1)));
					}
					VK_UploadImageData(&vk_d.blueNoiseTex, width, height, &img, bytes_per_channel, 0, (i*4) + channel);
				}
				ri.Free(pic);
			}
			VK_CreateSampler(&vk_d.blueNoiseTex, VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT);
		}
		// </RTX>

		// descriptor for all texture array
		vk_d.imageDescriptor.lastBindingVariableSizeExt = qtrue;
		VK_AddSamplerCount(&vk_d.imageDescriptor, 0, VK_GLOBAL_IMAGEARRAY_SHADER_STAGE_FLAGS, MAX_DRAWIMAGES);
		VK_FinishDescriptorWithoutUpdate(&vk_d.imageDescriptor);

		// device infos
		VkPhysicalDeviceProperties devProperties;
		vkGetPhysicalDeviceProperties(vk.physicalDevice, &devProperties);

		// device limits
		VkPhysicalDeviceLimits limits = devProperties.limits;
		glConfig.maxTextureSize = limits.maxImageDimension2D;

		// stubbed or broken drivers may have reported 0...
		if (glConfig.maxTextureSize <= 0)
		{
			glConfig.maxTextureSize = 0;
		}

		// init command buffers and SMP
		R_InitCommandBuffers();

		// print info
		GfxInfo_f();

		// set default state
		VK_SetDefaultState();
	}
}

/*
==================
GL_CheckErrors
==================
*/
void GL_CheckErrors( void ) {
    int		err;
    char	s[64];

    err = qglGetError();
    if ( err == GL_NO_ERROR ) {
        return;
    }
    if ( r_ignoreGLErrors->integer ) {
        return;
    }
    switch( err ) {
        case GL_INVALID_ENUM:
            strcpy( s, "GL_INVALID_ENUM" );
            break;
        case GL_INVALID_VALUE:
            strcpy( s, "GL_INVALID_VALUE" );
            break;
        case GL_INVALID_OPERATION:
            strcpy( s, "GL_INVALID_OPERATION" );
            break;
        case GL_STACK_OVERFLOW:
            strcpy( s, "GL_STACK_OVERFLOW" );
            break;
        case GL_STACK_UNDERFLOW:
            strcpy( s, "GL_STACK_UNDERFLOW" );
            break;
        case GL_OUT_OF_MEMORY:
            strcpy( s, "GL_OUT_OF_MEMORY" );
            break;
        default:
            Com_sprintf( s, sizeof(s), "%i", err);
            break;
    }

    ri.Error( ERR_FATAL, "GL_CheckErrors: %s", s );
}


/*
** R_GetModeInfo
*/
typedef struct vidmode_s
{
    const char *description;
    int         width, height;
	float		pixelAspect;		// pixel width / height
} vidmode_t;

vidmode_t r_vidModes[] =
{
    { "Mode  0: 320x240",        320,    240,    1 },
    { "Mode  1: 400x300",        400,    300,    1 },
    { "Mode  2: 512x384",        512,    384,    1 },
    { "Mode  3: 640x480",        640,    480,    1 },
    { "Mode  4: 800x600",        800,    600,    1 },
    { "Mode  5: 960x720",        960,    720,    1 },
    { "Mode  6: 1024x768",        1024,    768,    1 },
    { "Mode  7: 1152x864",        1152,    864,    1 },
    { "Mode  8: 1280x1024",        1280,    1024,    1 },
    { "Mode  9: 1600x1200",        1600,    1200,    1 },
    { "Mode 10: 2048x1536",        2048,    1536,    1 },
    { "Mode 11: 856x480 (wide)",    856,    480,    1 },
	{ "Mode 11: 856x480 (wide)",    1280,    720,    1 },
	{ "Mode 11: 856x480 (wide)",    1600,    900,    1 },
	{ "Mode 11: 856x480 (wide)",    1920,    1080,    1 },
	{ "Mode 11: 856x480 (wide)",    2560,    1440,    1 }
};
static int	s_numVidModes = ( sizeof( r_vidModes ) / sizeof( r_vidModes[0] ) );

qboolean R_GetModeInfo( int *width, int *height, float *windowAspect, int mode ) {
	vidmode_t	*vm;

    if ( mode < -1 ) {
        return qfalse;
	}
	if ( mode >= s_numVidModes ) {
		return qfalse;
	}

	if ( mode == -1 ) {
		*width = r_customwidth->integer;
		*height = r_customheight->integer;
		*windowAspect = r_customaspect->value;
		return qtrue;
	}

	vm = &r_vidModes[mode];

    *width  = vm->width;
    *height = vm->height;
    *windowAspect = (float)vm->width / ( vm->height * vm->pixelAspect );

    return qtrue;
}

/*
** R_ModeList_f
*/
static void R_ModeList_f( void )
{
	int i;

	ri.Printf( PRINT_ALL, "\n" );
	for ( i = 0; i < s_numVidModes; i++ )
	{
		ri.Printf( PRINT_ALL, "%s\n", r_vidModes[i].description );
	}
	ri.Printf( PRINT_ALL, "\n" );
}


/* 
============================================================================== 
 
						SCREEN SHOTS 

NOTE TTimo
some thoughts about the screenshots system:
screenshots get written in fs_homepath + fs_gamedir
vanilla q3 .. baseq3/screenshots/ *.tga
team arena .. missionpack/screenshots/ *.tga

two commands: "screenshot" and "screenshotJPEG"
we use statics to store a count and start writing the first screenshot/screenshot????.tga (.jpg) available
(with FS_FileExists / FS_FOpenFileWrite calls)
FIXME: the statics don't get a reinit between fs_game changes

============================================================================== 
*/ 

/* 
================== 
RB_TakeScreenshot
================== 
*/  
void RB_TakeScreenshot( int x, int y, int width, int height, char *fileName ) {
	byte		*buffer;
	int			i, c, temp;
		
	buffer = ri.Hunk_AllocateTempMemory(glConfig.vidWidth*glConfig.vidHeight*3+18);

	Com_Memset (buffer, 0, 18);
	buffer[2] = 2;		// uncompressed type
	buffer[12] = width & 255;
	buffer[13] = width >> 8;
	buffer[14] = height & 255;
	buffer[15] = height >> 8;
	buffer[16] = 24;	// pixel size

	if(glConfig.driverType == OPENGL) qglReadPixels( x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer+18 ); 
	else if (glConfig.driverType == VULKAN)	VK_ReadPixelsScreen(qfalse, buffer + 18);


	// swap rgb to bgr
	c = 18 + width * height * 3;
	for (i=18 ; i<c ; i+=3) {
		temp = buffer[i];
		buffer[i] = buffer[i+2];
		buffer[i+2] = temp;
	}

	// gamma correct
	if ( ( tr.overbrightBits > 0 ) && glConfig.deviceSupportsGamma ) {
		R_GammaCorrect( buffer + 18, glConfig.vidWidth * glConfig.vidHeight * 3 );
	}

	ri.FS_WriteFile( fileName, buffer, c );

	ri.Hunk_FreeTempMemory( buffer );
}

/* 
================== 
RB_TakeScreenshotJPEG
================== 
*/  
void RB_TakeScreenshotJPEG( int x, int y, int width, int height, char *fileName ) {
	byte		*buffer;

	buffer = ri.Hunk_AllocateTempMemory(glConfig.vidWidth*glConfig.vidHeight*4);

	if (glConfig.driverType == OPENGL) qglReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	else if (glConfig.driverType == VULKAN)	VK_ReadPixelsScreen(qtrue, buffer);

	// gamma correct
	if ( ( tr.overbrightBits > 0 ) && glConfig.deviceSupportsGamma ) {
		R_GammaCorrect( buffer, glConfig.vidWidth * glConfig.vidHeight * 4 );
	}

	ri.FS_WriteFile( fileName, buffer, 1 );		// create path
	SaveJPG( fileName, 95, glConfig.vidWidth, glConfig.vidHeight, buffer);

	ri.Hunk_FreeTempMemory( buffer );
}

/*
==================
RB_TakeScreenshotCmd
==================
*/
const void *RB_TakeScreenshotCmd( const void *data ) {
	const screenshotCommand_t	*cmd;
	
	cmd = (const screenshotCommand_t *)data;
	
	if (cmd->jpeg)
		RB_TakeScreenshotJPEG( cmd->x, cmd->y, cmd->width, cmd->height, cmd->fileName);
	else
		RB_TakeScreenshot( cmd->x, cmd->y, cmd->width, cmd->height, cmd->fileName);
	
	return (const void *)(cmd + 1);	
}

/*
==================
R_TakeScreenshot
==================
*/
void R_TakeScreenshot( int x, int y, int width, int height, char *name, qboolean jpeg ) {
	static char	fileName[MAX_OSPATH]; // bad things if two screenshots per frame?
	screenshotCommand_t	*cmd;

	cmd = R_GetCommandBuffer( sizeof( *cmd ) );
	if ( !cmd ) {
		return;
	}
	cmd->commandId = RC_SCREENSHOT;

	cmd->x = x;
	cmd->y = y;
	cmd->width = width;
	cmd->height = height;
	Q_strncpyz( fileName, name, sizeof(fileName) );
	cmd->fileName = fileName;
	cmd->jpeg = jpeg;
}

/* 
================== 
R_ScreenshotFilename
================== 
*/  
void R_ScreenshotFilename( int lastNumber, char *fileName ) {
	int		a,b,c,d;

	if ( lastNumber < 0 || lastNumber > 9999 ) {
		Com_sprintf( fileName, MAX_OSPATH, "screenshots/shot9999.tga" );
		return;
	}

	a = lastNumber / 1000;
	lastNumber -= a*1000;
	b = lastNumber / 100;
	lastNumber -= b*100;
	c = lastNumber / 10;
	lastNumber -= c*10;
	d = lastNumber;

	Com_sprintf( fileName, MAX_OSPATH, "screenshots/shot%i%i%i%i.tga"
		, a, b, c, d );
}

/* 
================== 
R_ScreenshotFilename
================== 
*/  
void R_ScreenshotFilenameJPEG( int lastNumber, char *fileName ) {
	int		a,b,c,d;

	if ( lastNumber < 0 || lastNumber > 9999 ) {
		Com_sprintf( fileName, MAX_OSPATH, "screenshots/shot9999.jpg" );
		return;
	}

	a = lastNumber / 1000;
	lastNumber -= a*1000;
	b = lastNumber / 100;
	lastNumber -= b*100;
	c = lastNumber / 10;
	lastNumber -= c*10;
	d = lastNumber;

	Com_sprintf( fileName, MAX_OSPATH, "screenshots/shot%i%i%i%i.jpg"
		, a, b, c, d );
}

/*
====================
R_LevelShot

levelshots are specialized 128*128 thumbnails for
the menu system, sampled down from full screen distorted images
====================
*/
void R_LevelShot( void ) {
	char		checkname[MAX_OSPATH];
	byte		*buffer;
	byte		*source;
	byte		*src, *dst;
	int			x, y;
	int			r, g, b;
	float		xScale, yScale;
	int			xx, yy;

	sprintf( checkname, "levelshots/%s.tga", tr.world->baseName );

	source = ri.Hunk_AllocateTempMemory( glConfig.vidWidth * glConfig.vidHeight * 3 );

	buffer = ri.Hunk_AllocateTempMemory( 128 * 128*3 + 18);
	Com_Memset (buffer, 0, 18);
	buffer[2] = 2;		// uncompressed type
	buffer[12] = 128;
	buffer[14] = 128;
	buffer[16] = 24;	// pixel size

	qglReadPixels( 0, 0, glConfig.vidWidth, glConfig.vidHeight, GL_RGB, GL_UNSIGNED_BYTE, source ); 

	// resample from source
	xScale = glConfig.vidWidth / 512.0f;
	yScale = glConfig.vidHeight / 384.0f;
	for ( y = 0 ; y < 128 ; y++ ) {
		for ( x = 0 ; x < 128 ; x++ ) {
			r = g = b = 0;
			for ( yy = 0 ; yy < 3 ; yy++ ) {
				for ( xx = 0 ; xx < 4 ; xx++ ) {
					src = source + 3 * ( glConfig.vidWidth * (int)( (y*3+yy)*yScale ) + (int)( (x*4+xx)*xScale ) );
					r += src[0];
					g += src[1];
					b += src[2];
				}
			}
			dst = buffer + 18 + 3 * ( y * 128 + x );
			dst[0] = b / 12;
			dst[1] = g / 12;
			dst[2] = r / 12;
		}
	}

	// gamma correct
	if ( ( tr.overbrightBits > 0 ) && glConfig.deviceSupportsGamma ) {
		R_GammaCorrect( buffer + 18, 128 * 128 * 3 );
	}

	ri.FS_WriteFile( checkname, buffer, 128 * 128*3 + 18 );

	ri.Hunk_FreeTempMemory( buffer );
	ri.Hunk_FreeTempMemory( source );

	ri.Printf( PRINT_ALL, "Wrote %s\n", checkname );
}

/* 
================== 
R_ScreenShot_f

screenshot
screenshot [silent]
screenshot [levelshot]
screenshot [filename]

Doesn't print the pacifier message if there is a second arg
================== 
*/  
void R_ScreenShot_f (void) {
	char	checkname[MAX_OSPATH];
	static	int	lastNumber = -1;
	qboolean	silent;

	if ( !strcmp( ri.Cmd_Argv(1), "levelshot" ) ) {
		R_LevelShot();
		return;
	}

	if ( !strcmp( ri.Cmd_Argv(1), "silent" ) ) {
		silent = qtrue;
	} else {
		silent = qfalse;
	}

	if ( ri.Cmd_Argc() == 2 && !silent ) {
		// explicit filename
		Com_sprintf( checkname, MAX_OSPATH, "screenshots/%s.tga", ri.Cmd_Argv( 1 ) );
	} else {
		// scan for a free filename

		// if we have saved a previous screenshot, don't scan
		// again, because recording demo avis can involve
		// thousands of shots
		if ( lastNumber == -1 ) {
			lastNumber = 0;
		}
		// scan for a free number
		for ( ; lastNumber <= 9999 ; lastNumber++ ) {
			R_ScreenshotFilename( lastNumber, checkname );

      if (!ri.FS_FileExists( checkname ))
      {
        break; // file doesn't exist
      }
		}

		if ( lastNumber >= 9999 ) {
			ri.Printf (PRINT_ALL, "ScreenShot: Couldn't create a file\n"); 
			return;
 		}

		lastNumber++;
	}

	R_TakeScreenshot( 0, 0, glConfig.vidWidth, glConfig.vidHeight, checkname, qfalse );

	if ( !silent ) {
		ri.Printf (PRINT_ALL, "Wrote %s\n", checkname);
	}
} 

void R_ScreenShotJPEG_f (void) {
	char		checkname[MAX_OSPATH];
	static	int	lastNumber = -1;
	qboolean	silent;

	if ( !strcmp( ri.Cmd_Argv(1), "levelshot" ) ) {
		R_LevelShot();
		return;
	}

	if ( !strcmp( ri.Cmd_Argv(1), "silent" ) ) {
		silent = qtrue;
	} else {
		silent = qfalse;
	}

	if ( ri.Cmd_Argc() == 2 && !silent ) {
		// explicit filename
		Com_sprintf( checkname, MAX_OSPATH, "screenshots/%s.jpg", ri.Cmd_Argv( 1 ) );
	} else {
		// scan for a free filename

		// if we have saved a previous screenshot, don't scan
		// again, because recording demo avis can involve
		// thousands of shots
		if ( lastNumber == -1 ) {
			lastNumber = 0;
		}
		// scan for a free number
		for ( ; lastNumber <= 9999 ; lastNumber++ ) {
			R_ScreenshotFilenameJPEG( lastNumber, checkname );

      if (!ri.FS_FileExists( checkname ))
      {
        break; // file doesn't exist
      }
		}

		if ( lastNumber == 10000 ) {
			ri.Printf (PRINT_ALL, "ScreenShot: Couldn't create a file\n"); 
			return;
 		}

		lastNumber++;
	}

	R_TakeScreenshot( 0, 0, glConfig.vidWidth, glConfig.vidHeight, checkname, qtrue );

	if ( !silent ) {
		ri.Printf (PRINT_ALL, "Wrote %s\n", checkname);
	}
} 

//============================================================================

/*
** GL_SetDefaultState
*/
void GL_SetDefaultState( void )
{
	qglClearDepth( 1.0f );

	qglCullFace(GL_FRONT);

	qglColor4f (1,1,1,1);

	// initialize downstream texture unit if we're running
	// in a multitexture environment
	if ( qglActiveTextureARB ) {
		GL_SelectTexture( 1 );
		GL_TextureMode( r_textureMode->string );
		GL_TexEnv( GL_MODULATE );
		qglDisable( GL_TEXTURE_2D );
		GL_SelectTexture( 0 );
	}

	qglEnable(GL_TEXTURE_2D);
	GL_TextureMode( r_textureMode->string );
	GL_TexEnv( GL_MODULATE );

	qglShadeModel( GL_SMOOTH );
	qglDepthFunc( GL_LEQUAL );

	// the vertex array is always enabled, but the color and texture
	// arrays are enabled and disabled around the compiled vertex array call
	qglEnableClientState (GL_VERTEX_ARRAY);

	//
	// make sure our GL state vector is set correctly
	//
	glState.glStateBits = GLS_DEPTHTEST_DISABLE | GLS_DEPTHMASK_TRUE;

	qglPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	qglDepthMask( GL_TRUE );
	qglDisable( GL_DEPTH_TEST );
	qglEnable( GL_SCISSOR_TEST );
	qglDisable( GL_CULL_FACE );
	qglDisable( GL_BLEND );
}

void VK_SetDefaultState(void)
{
	vk_d.state.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	//qglClearDepth(1.0f);
	vk_d.clearDepth = 1.0f;

	vk_d.state.cullMode = VK_CULL_MODE_FRONT_BIT;
	//qglCullFace(GL_FRONT);

	//qglColor4f(1, 1, 1, 1);

	//// initialize downstream texture unit if we're running
	//// in a multitexture environment
	//if (qglActiveTextureARB) {
	//	GL_SelectTexture(1);
	//	GL_TextureMode(r_textureMode->string);
	//	GL_TexEnv(GL_MODULATE);
	//	qglDisable(GL_TEXTURE_2D);
	//	GL_SelectTexture(0);
	//}

	//qglEnable(GL_TEXTURE_2D);
	//GL_TextureMode(r_textureMode->string);
	//GL_TexEnv(GL_MODULATE);

	//qglShadeModel(GL_SMOOTH);
	//qglDepthFunc(GL_LEQUAL);

	vk_d.state.dsBlend.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

	//// the vertex array is always enabled, but the color and texture
	//// arrays are enabled and disabled around the compiled vertex array call
	//qglEnableClientState(GL_VERTEX_ARRAY);

	//
	// make sure our VK state vector is set correctly
	//
	//glState.glStateBits = GLS_DEPTHTEST_DISABLE | GLS_DEPTHMASK_TRUE;
	//vk_d.state.blendBits = GLS_DEPTHTEST_DISABLE | GLS_DEPTHMASK_TRUE;

	
	//qglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//qglDepthMask(GL_TRUE);
	//qglDisable(GL_DEPTH_TEST);
	//qglEnable(GL_SCISSOR_TEST);
	//qglDisable(GL_CULL_FACE);
	//qglDisable(GL_BLEND);

	vk_d.state.polygonMode = VK_POLYGON_MODE_FILL;
	vk_d.state.dsBlend.depthWriteEnable = VK_TRUE;
	vk_d.state.dsBlend.depthTestEnable = VK_FALSE;

	vk_d.state.cullMode = VK_CULL_MODE_NONE;
	vk_d.state.colorBlend.blendEnable = VK_FALSE;
}


/*
================
GfxInfo_f
================
*/
void GfxInfo_f( void ) 
{
	cvar_t *sys_cpustring = ri.Cvar_Get( "sys_cpustring", "", 0 );
	const char *enablestrings[] =
	{
		"disabled",
		"enabled"
	};
	const char *fsstrings[] =
	{
		"windowed",
		"fullscreen"
	};

	ri.Printf( PRINT_ALL, "\nVENDOR: %s\n", glConfig.vendor_string );
	ri.Printf( PRINT_ALL, "RENDERER: %s\n", glConfig.renderer_string );
	ri.Printf( PRINT_ALL, "API_VERSION: %s\n", glConfig.version_string );
	ri.Printf( PRINT_ALL, "EXTENSIONS: %s\n", glConfig.extensions_string );
	ri.Printf( PRINT_ALL, "MAX_TEXTURE_SIZE: %d\n", glConfig.maxTextureSize );
	ri.Printf( PRINT_ALL, "GL_MAX_ACTIVE_TEXTURES_ARB: %d\n", glConfig.maxActiveTextures );
	ri.Printf( PRINT_ALL, "\nPIXELFORMAT: color(%d-bits) Z(%d-bit) stencil(%d-bits)\n", glConfig.colorBits, glConfig.depthBits, glConfig.stencilBits );
	ri.Printf( PRINT_ALL, "MODE: %d, %d x %d %s hz:", r_mode->integer, glConfig.vidWidth, glConfig.vidHeight, fsstrings[r_fullscreen->integer == 1] );
	if ( glConfig.displayFrequency )
	{
		ri.Printf( PRINT_ALL, "%d\n", glConfig.displayFrequency );
	}
	else
	{
		ri.Printf( PRINT_ALL, "N/A\n" );
	}
	if ( glConfig.deviceSupportsGamma )
	{
		ri.Printf( PRINT_ALL, "GAMMA: hardware w/ %d overbright bits\n", tr.overbrightBits );
	}
	else
	{
		ri.Printf( PRINT_ALL, "GAMMA: software w/ %d overbright bits\n", tr.overbrightBits );
	}
	ri.Printf( PRINT_ALL, "CPU: %s\n", sys_cpustring->string );

	// rendering primitives
	{
		int		primitives;

		// default is to use triangles if compiled vertex arrays are present
		ri.Printf( PRINT_ALL, "rendering primitives: " );
		primitives = r_primitives->integer;
		if ( primitives == 0 ) {
			if (glConfig.driverType == OPENGL && qglLockArraysEXT ) {
				primitives = 2;
			} else {
				primitives = 1;
			}
		}
		if ( primitives == -1 ) {
			ri.Printf( PRINT_ALL, "none\n" );
		} else if ( primitives == 2 ) {
			ri.Printf( PRINT_ALL, "single glDrawElements\n" );
		} else if ( primitives == 1 ) {
			ri.Printf( PRINT_ALL, "multiple glArrayElement\n" );
		} else if ( primitives == 3 ) {
			ri.Printf( PRINT_ALL, "multiple glColor4ubv + glTexCoord2fv + glVertex3fv\n" );
		}
	}

	ri.Printf( PRINT_ALL, "texturemode: %s\n", r_textureMode->string );
	ri.Printf( PRINT_ALL, "picmip: %d\n", r_picmip->integer );
	ri.Printf( PRINT_ALL, "texture bits: %d\n", r_texturebits->integer );
	ri.Printf( PRINT_ALL, "multitexture: %s\n", enablestrings[qglActiveTextureARB != 0] );
	ri.Printf( PRINT_ALL, "compiled vertex arrays: %s\n", enablestrings[qglLockArraysEXT != 0 ] );
	ri.Printf( PRINT_ALL, "texenv add: %s\n", enablestrings[glConfig.textureEnvAddAvailable != 0] );
	ri.Printf( PRINT_ALL, "compressed textures: %s\n", enablestrings[glConfig.textureCompression!=TC_NONE] );
	if ( r_vertexLight->integer)
	{
		ri.Printf( PRINT_ALL, "HACK: using vertex lightmap approximation\n" );
	}
	if ( glConfig.smpActive ) {
		ri.Printf( PRINT_ALL, "Using dual processor acceleration\n" );
	}
	if ( r_finish->integer ) {
		ri.Printf( PRINT_ALL, "Forcing glFinish\n" );
	}
}

/*
===============
R_Register
===============
*/
void R_Register( void ) 
{
	//
	// latched and archived variables
	//
	r_glDriver = ri.Cvar_Get( "r_glDriver", OPENGL_DRIVER_NAME, CVAR_ARCHIVE | CVAR_LATCH );
	r_allowExtensions = ri.Cvar_Get( "r_allowExtensions", "1", CVAR_ARCHIVE | CVAR_LATCH );
	r_ext_compressed_textures = ri.Cvar_Get( "r_ext_compressed_textures", "0", CVAR_ARCHIVE | CVAR_LATCH );
	r_ext_gamma_control = ri.Cvar_Get( "r_ext_gamma_control", "1", CVAR_ARCHIVE | CVAR_LATCH );
	r_ext_multitexture = ri.Cvar_Get( "r_ext_multitexture", "1", CVAR_ARCHIVE | CVAR_LATCH );
	r_ext_compiled_vertex_array = ri.Cvar_Get( "r_ext_compiled_vertex_array", "1", CVAR_ARCHIVE | CVAR_LATCH);
#ifdef __linux__ // broken on linux
	r_ext_texture_env_add = ri.Cvar_Get( "r_ext_texture_env_add", "0", CVAR_ARCHIVE | CVAR_LATCH);
#else
	r_ext_texture_env_add = ri.Cvar_Get( "r_ext_texture_env_add", "1", CVAR_ARCHIVE | CVAR_LATCH);
#endif

	r_picmip = ri.Cvar_Get ("r_picmip", "1", CVAR_ARCHIVE | CVAR_LATCH );
	r_roundImagesDown = ri.Cvar_Get ("r_roundImagesDown", "1", CVAR_ARCHIVE | CVAR_LATCH );
	r_colorMipLevels = ri.Cvar_Get ("r_colorMipLevels", "0", CVAR_LATCH );
	AssertCvarRange( r_picmip, 0, 16, qtrue );
	r_detailTextures = ri.Cvar_Get( "r_detailtextures", "1", CVAR_ARCHIVE | CVAR_LATCH );
	r_texturebits = ri.Cvar_Get( "r_texturebits", "0", CVAR_ARCHIVE | CVAR_LATCH );
	r_colorbits = ri.Cvar_Get( "r_colorbits", "32", CVAR_ARCHIVE | CVAR_LATCH );
	r_stereo = ri.Cvar_Get( "r_stereo", "0", CVAR_ARCHIVE | CVAR_LATCH );
	r_stencilbits = ri.Cvar_Get( "r_stencilbits", "8", CVAR_ARCHIVE | CVAR_LATCH );
	r_depthbits = ri.Cvar_Get( "r_depthbits", "24", CVAR_ARCHIVE | CVAR_LATCH );
	r_overBrightBits = ri.Cvar_Get ("r_overBrightBits", "1", CVAR_ARCHIVE | CVAR_LATCH );
	r_ignorehwgamma = ri.Cvar_Get( "r_ignorehwgamma", "0", CVAR_ARCHIVE | CVAR_LATCH);
	r_mode = ri.Cvar_Get( "r_mode", "3", CVAR_ARCHIVE | CVAR_LATCH );
	r_fullscreen = ri.Cvar_Get( "r_fullscreen", "1", CVAR_ARCHIVE | CVAR_LATCH );
	r_customwidth = ri.Cvar_Get( "r_customwidth", "1600", CVAR_ARCHIVE | CVAR_LATCH );
	r_customheight = ri.Cvar_Get( "r_customheight", "1024", CVAR_ARCHIVE | CVAR_LATCH );
	r_customaspect = ri.Cvar_Get( "r_customaspect", "1", CVAR_ARCHIVE | CVAR_LATCH );
	r_simpleMipMaps = ri.Cvar_Get( "r_simpleMipMaps", "1", CVAR_ARCHIVE | CVAR_LATCH );
	r_vertexLight = ri.Cvar_Get( "r_vertexLight", "0", CVAR_ARCHIVE | CVAR_LATCH );
	r_rtx = ri.Cvar_Get("r_rtx", "0", CVAR_ARCHIVE | CVAR_LATCH);
	r_uiFullScreen = ri.Cvar_Get( "r_uifullscreen", "0", 0);
	r_subdivisions = ri.Cvar_Get ("r_subdivisions", "4", CVAR_ARCHIVE | CVAR_LATCH);
	
#if (defined(__APPLE__) || defined(__linux__)) && defined(SMP)
  // Default to using SMP on Mac OS X or Linux if we have multiple processors
	r_smp = ri.Cvar_Get( "r_smp", Sys_ProcessorCount() > 1 ? "1" : "0", CVAR_ARCHIVE | CVAR_LATCH);
#else        
	r_smp = ri.Cvar_Get( "r_smp", "0", CVAR_ARCHIVE | CVAR_LATCH);
#endif
	r_ignoreFastPath = ri.Cvar_Get( "r_ignoreFastPath", "1", CVAR_ARCHIVE | CVAR_LATCH );

	//
	// temporary latched variables that can only change over a restart
	//
	r_displayRefresh = ri.Cvar_Get( "r_displayRefresh", "0", CVAR_LATCH );
	AssertCvarRange( r_displayRefresh, 0, 200, qtrue );
	r_fullbright = ri.Cvar_Get ("r_fullbright", "0", CVAR_LATCH|CVAR_CHEAT );
	r_mapOverBrightBits = ri.Cvar_Get ("r_mapOverBrightBits", "2", CVAR_LATCH );
	r_intensity = ri.Cvar_Get ("r_intensity", "1", CVAR_LATCH );
	r_singleShader = ri.Cvar_Get ("r_singleShader", "0", CVAR_CHEAT | CVAR_LATCH );

	//
	// archived variables that can change at any time
	//
	r_lodCurveError = ri.Cvar_Get( "r_lodCurveError", "250", CVAR_ARCHIVE|CVAR_CHEAT );
	r_lodbias = ri.Cvar_Get( "r_lodbias", "0", CVAR_ARCHIVE );
	r_flares = ri.Cvar_Get ("r_flares", "0", CVAR_ARCHIVE );
	r_znear = ri.Cvar_Get( "r_znear", "4", CVAR_CHEAT );
	AssertCvarRange( r_znear, 0.001f, 200, qtrue );
	r_ignoreGLErrors = ri.Cvar_Get( "r_ignoreGLErrors", "1", CVAR_ARCHIVE );
	r_fastsky = ri.Cvar_Get( "r_fastsky", "0", CVAR_ARCHIVE );
	r_inGameVideo = ri.Cvar_Get( "r_inGameVideo", "1", CVAR_ARCHIVE );
	r_drawSun = ri.Cvar_Get( "r_drawSun", "0", CVAR_ARCHIVE );
	r_dynamiclight = ri.Cvar_Get( "r_dynamiclight", "1", CVAR_ARCHIVE );
	r_dlightBacks = ri.Cvar_Get( "r_dlightBacks", "1", CVAR_ARCHIVE );
	r_finish = ri.Cvar_Get ("r_finish", "0", CVAR_ARCHIVE);
	r_textureMode = ri.Cvar_Get( "r_textureMode", "GL_LINEAR_MIPMAP_NEAREST", CVAR_ARCHIVE );
	r_swapInterval = ri.Cvar_Get( "r_swapInterval", "0", CVAR_ARCHIVE );

	r_gamma = ri.Cvar_Get( "r_gamma", "1", CVAR_ARCHIVE );

	r_facePlaneCull = ri.Cvar_Get ("r_facePlaneCull", "1", CVAR_ARCHIVE );

	r_railWidth = ri.Cvar_Get( "r_railWidth", "16", CVAR_ARCHIVE );
	r_railCoreWidth = ri.Cvar_Get( "r_railCoreWidth", "6", CVAR_ARCHIVE );
	r_railSegmentLength = ri.Cvar_Get( "r_railSegmentLength", "32", CVAR_ARCHIVE );

	r_primitives = ri.Cvar_Get( "r_primitives", "0", CVAR_ARCHIVE );

	r_ambientScale = ri.Cvar_Get( "r_ambientScale", "0.6", CVAR_CHEAT );
	r_directedScale = ri.Cvar_Get( "r_directedScale", "1", CVAR_CHEAT );

	//
	// temporary variables that can change at any time
	//
	r_showImages = ri.Cvar_Get( "r_showImages", "0", CVAR_TEMP );

	r_debugLight = ri.Cvar_Get( "r_debuglight", "0", CVAR_TEMP );
	r_debugSort = ri.Cvar_Get( "r_debugSort", "0", CVAR_CHEAT );
	r_printShaders = ri.Cvar_Get( "r_printShaders", "0", 0 );
	r_saveFontData = ri.Cvar_Get( "r_saveFontData", "0", 0 );

	r_nocurves = ri.Cvar_Get ("r_nocurves", "0", CVAR_CHEAT );
	r_drawworld = ri.Cvar_Get ("r_drawworld", "1", CVAR_CHEAT );
	r_lightmap = ri.Cvar_Get ("r_lightmap", "0", 0 );
	r_portalOnly = ri.Cvar_Get ("r_portalOnly", "0", CVAR_CHEAT );

	r_flareSize = ri.Cvar_Get ("r_flareSize", "40", CVAR_CHEAT);
	r_flareFade = ri.Cvar_Get ("r_flareFade", "7", CVAR_CHEAT);

	r_showSmp = ri.Cvar_Get ("r_showSmp", "0", CVAR_CHEAT);
	r_skipBackEnd = ri.Cvar_Get ("r_skipBackEnd", "0", CVAR_CHEAT);

	r_measureOverdraw = ri.Cvar_Get( "r_measureOverdraw", "0", CVAR_CHEAT );
	r_lodscale = ri.Cvar_Get( "r_lodscale", "5", CVAR_CHEAT );
	r_norefresh = ri.Cvar_Get ("r_norefresh", "0", CVAR_CHEAT);
	r_drawentities = ri.Cvar_Get ("r_drawentities", "1", CVAR_CHEAT );
	r_ignore = ri.Cvar_Get( "r_ignore", "1", CVAR_CHEAT );
	r_nocull = ri.Cvar_Get ("r_nocull", "0", CVAR_CHEAT);
	r_novis = ri.Cvar_Get ("r_novis", "0", CVAR_CHEAT);
	r_showcluster = ri.Cvar_Get ("r_showcluster", "0", CVAR_CHEAT);
	r_speeds = ri.Cvar_Get ("r_speeds", "0", CVAR_CHEAT);
	r_verbose = ri.Cvar_Get( "r_verbose", "0", CVAR_CHEAT );
	r_logFile = ri.Cvar_Get( "r_logFile", "0", CVAR_CHEAT );
	r_debugSurface = ri.Cvar_Get ("r_debugSurface", "0", CVAR_CHEAT);
	r_nobind = ri.Cvar_Get ("r_nobind", "0", CVAR_CHEAT);
	r_showtris = ri.Cvar_Get ("r_showtris", "0", CVAR_CHEAT);
	r_showsky = ri.Cvar_Get ("r_showsky", "0", CVAR_CHEAT);
	r_shownormals = ri.Cvar_Get ("r_shownormals", "0", CVAR_CHEAT);
	r_clear = ri.Cvar_Get ("r_clear", "0", CVAR_CHEAT);
	r_offsetFactor = ri.Cvar_Get( "r_offsetfactor", "-1", CVAR_CHEAT );
	r_offsetUnits = ri.Cvar_Get( "r_offsetunits", "-2", CVAR_CHEAT );
	r_drawBuffer = ri.Cvar_Get( "r_drawBuffer", "GL_BACK", CVAR_CHEAT );
	r_lockpvs = ri.Cvar_Get ("r_lockpvs", "0", CVAR_CHEAT);
	r_noportals = ri.Cvar_Get ("r_noportals", "0", CVAR_CHEAT);
	r_shadows = ri.Cvar_Get( "cg_shadows", "1", 0 );

	r_maxpolys = ri.Cvar_Get( "r_maxpolys", va("%d", MAX_POLYS), 0);
	r_maxpolyverts = ri.Cvar_Get( "r_maxpolyverts", va("%d", MAX_POLYVERTS), 0);

	// RTX
	pt_showIntermediateResults = ri.Cvar_Get("pt_showIntermediateResults", "1", 0);
	pt_numRandomIL = ri.Cvar_Get("pt_numRandomIL", "1", 0);
	rt_illumination = ri.Cvar_Get("rt_illumination", "1", 0);

	rt_printPerformanceStatistic = ri.Cvar_Get("rt_printPerfStats", "0", CVAR_TEMP);
	rt_cullLights = ri.Cvar_Get("rt_cullLights", "1", 0);
	rt_numRandomDL = ri.Cvar_Get("rt_numRandomDL", "1", 0);
	rt_numRandomIL = ri.Cvar_Get("rt_numRandomIL", "1", 0);
	rt_numBounces = ri.Cvar_Get("rt_numBounces", "0", 0);
	rt_accumulate = ri.Cvar_Get("rt_accumulate", "0", 0);
	rt_pause = ri.Cvar_Get("rt_pause", "0", 0);
	rt_antialiasing = ri.Cvar_Get("rt_antialiasing", "1", 0);
	rt_debug_lights = ri.Cvar_Get("rt_debug_lights", "0", 0);
	rt_numSamples = ri.Cvar_Get("rt_numSamples", "1", 0);
	rt_maxSamples = ri.Cvar_Get("rt_maxSamples", "0", 0);
	rt_softshadows = ri.Cvar_Get("rt_softshadows", "1", 0);
	rt_taa = ri.Cvar_Get("rt_taa", "1", 0);
	rt_aperture = ri.Cvar_Get("rt_aperture", "0.05", 0);
	rt_focalLength = ri.Cvar_Get("rt_focallength", "15", 0);
	rt_dof = ri.Cvar_Get("rt_dof", "0", 0);
	rt_denoiser = ri.Cvar_Get("rt_denoiser", "1", 0);
	rt_brightness = ri.Cvar_Get("rt_brightness", "0", 0);
	rt_tonemapping_reinhard = ri.Cvar_Get("rt_tonemapping_reinhard", "1", 0);

	// make sure all the commands added here are also
	// removed in R_Shutdown
	ri.Cmd_AddCommand( "imagelist", R_ImageList_f );
	ri.Cmd_AddCommand( "shaderlist", R_ShaderList_f );
	ri.Cmd_AddCommand( "skinlist", R_SkinList_f );
	ri.Cmd_AddCommand( "modellist", R_Modellist_f );
	ri.Cmd_AddCommand( "modelist", R_ModeList_f );
	ri.Cmd_AddCommand( "screenshot", R_ScreenShot_f );
	ri.Cmd_AddCommand( "screenshotJPEG", R_ScreenShotJPEG_f );
	ri.Cmd_AddCommand( "gfxinfo", GfxInfo_f );
}

/*
===============
R_Init
===============
*/
void R_Init( void ) {	
	int	err;
	int i;
	byte *ptr;

	ri.Printf( PRINT_ALL, "----- R_Init -----\n" );

	// clear all our internal state
	Com_Memset( &tr, 0, sizeof( tr ) );
	Com_Memset( &backEnd, 0, sizeof( backEnd ) );
	Com_Memset( &tess, 0, sizeof( tess ) );
    
	if ( (int)tess.xyz & 15 ) {
		Com_Printf( "WARNING: tess.xyz not 16 byte aligned\n" );
	}
	Com_Memset( tess.constantColor255, 255, sizeof( tess.constantColor255 ) );

	//
	// init function tables
	//
	for ( i = 0; i < FUNCTABLE_SIZE; i++ )
	{
		tr.sinTable[i]		= sin( DEG2RAD( i * 360.0f / ( ( float ) ( FUNCTABLE_SIZE - 1 ) ) ) );
		tr.squareTable[i]	= ( i < FUNCTABLE_SIZE/2 ) ? 1.0f : -1.0f;
		tr.sawToothTable[i] = (float)i / FUNCTABLE_SIZE;
		tr.inverseSawToothTable[i] = 1.0f - tr.sawToothTable[i];

		if ( i < FUNCTABLE_SIZE / 2 )
		{
			if ( i < FUNCTABLE_SIZE / 4 )
			{
				tr.triangleTable[i] = ( float ) i / ( FUNCTABLE_SIZE / 4 );
			}
			else
			{
				tr.triangleTable[i] = 1.0f - tr.triangleTable[i-FUNCTABLE_SIZE / 4];
			}
		}
		else
		{
			tr.triangleTable[i] = -tr.triangleTable[i-FUNCTABLE_SIZE/2];
		}
	}

	R_InitFogTable();

	R_NoiseInit();

	R_Register();

	max_polys = r_maxpolys->integer;
	if (max_polys < MAX_POLYS)
		max_polys = MAX_POLYS;

	max_polyverts = r_maxpolyverts->integer;
	if (max_polyverts < MAX_POLYVERTS)
		max_polyverts = MAX_POLYVERTS;

	ptr = ri.Hunk_Alloc( sizeof( *backEndData[0] ) + sizeof(srfPoly_t) * max_polys + sizeof(polyVert_t) * max_polyverts, h_low);
	backEndData[0] = (backEndData_t *) ptr;
	backEndData[0]->polys = (srfPoly_t *) ((char *) ptr + sizeof( *backEndData[0] ));
	backEndData[0]->polyVerts = (polyVert_t *) ((char *) ptr + sizeof( *backEndData[0] ) + sizeof(srfPoly_t) * max_polys);
	if ( r_smp->integer ) {
		ptr = ri.Hunk_Alloc( sizeof( *backEndData[1] ) + sizeof(srfPoly_t) * max_polys + sizeof(polyVert_t) * max_polyverts, h_low);
		backEndData[1] = (backEndData_t *) ptr;
		backEndData[1]->polys = (srfPoly_t *) ((char *) ptr + sizeof( *backEndData[1] ));
		backEndData[1]->polyVerts = (polyVert_t *) ((char *) ptr + sizeof( *backEndData[1] ) + sizeof(srfPoly_t) * max_polys);
	} else {
		backEndData[1] = NULL;
	}
	R_ToggleSmpFrame();

	if (!Q_stricmp(r_glDriver->string, OPENGL_DRIVER_NAME)) InitOpenGL();
	else if (!Q_stricmp(r_glDriver->string, VULKAN_DRIVER_NAME)) InitVulkan();

	R_InitImages();

	R_InitShaders();

	R_InitSkins();

	R_ModelInit();

	R_InitFreeType();

	if (glConfig.driverType == OPENGL) {
		err = qglGetError();
		if (err != GL_NO_ERROR)
			ri.Printf(PRINT_ALL, "glGetError() = 0x%x\n", err);
	}

	ri.Printf( PRINT_ALL, "----- finished R_Init -----\n" );
    
}

/*
===============
RE_Shutdown
===============
*/
void RE_Shutdown( qboolean destroyWindow ) {	
	
	ri.Printf(PRINT_ALL, "RE_Shutdown( %i )\n", destroyWindow);

	ri.Cmd_RemoveCommand("modellist");
	ri.Cmd_RemoveCommand("screenshotJPEG");
	ri.Cmd_RemoveCommand("screenshot");
	ri.Cmd_RemoveCommand("imagelist");
	ri.Cmd_RemoveCommand("shaderlist");
	ri.Cmd_RemoveCommand("skinlist");
	ri.Cmd_RemoveCommand("gfxinfo");
	ri.Cmd_RemoveCommand("modelist");
	ri.Cmd_RemoveCommand("shaderstate");


	if (tr.registered) {
		R_SyncRenderThread();
		R_ShutdownCommandBuffers();
		R_DeleteTextures();

		if (glConfig.driverType == VULKAN) {
			if (vk_d.clusterList != NULL) {
				free(vk_d.clusterList);
				vk_d.clusterList = NULL;
			}
			if (vk_d.vis != NULL) {
				free(vk_d.vis);
				vk_d.vis = NULL;
			}
			vk_d.lightList.numLights = 0;
			// <RTX>
			vk_d.worldASInit = qfalse;
			VK_DestroyRayTracingPipeline(&vk_d.primaryRaysPipeline);
			VK_DestroyRayTracingPipeline(&vk_d.reflectRaysPipeline);
			VK_DestroyRayTracingPipeline(&vk_d.directIlluminationPipeline);
			VK_DestroyCPipeline(&vk_d.accelerationStructures.compositingPipeline);

			VK_DestroyCPipeline(&vk_d.accelerationStructures.asvgfAtrousPipeline);
			VK_DestroyCPipeline(&vk_d.accelerationStructures.asvgfFwdPipeline);
			VK_DestroyCPipeline(&vk_d.accelerationStructures.asvgfGradAtrousPipeline);
			VK_DestroyCPipeline(&vk_d.accelerationStructures.asvgfGradImgPipeline);
			VK_DestroyCPipeline(&vk_d.accelerationStructures.asvgfRngPipeline);
			VK_DestroyCPipeline(&vk_d.accelerationStructures.asvgfTaaPipeline);
			VK_DestroyCPipeline(&vk_d.accelerationStructures.asvgfTemporalPipeline);

			VK_DestroyCPipeline(&vk_d.accelerationStructures.maxmipmapPipeline);
			VK_DestroyCPipeline(&vk_d.accelerationStructures.tonemappingPipeline);

			VK_DestroyAllAccelerationStructures();
			VK_DestroyImage(&vk_d.accelerationStructures.envmap);
			VK_DestroyImage(&vk_d.accelerationStructures.visData);
			VK_DestroyImage(&vk_d.accelerationStructures.lightVisData);

			VK_DestroyBottomAccelerationStructure(&vk_d.bottomASWorldStatic);
			VK_DestroyBottomAccelerationStructure(&vk_d.bottomASWorldStaticTrans);
			VK_DestroyBottomAccelerationStructure(&vk_d.bottomASWorldDynamicData);
			VK_DestroyBottomAccelerationStructure(&vk_d.bottomASWorldDynamicDataTrans);
			for (int i = 0; i < vk.swapchain.imageCount; i++) {
				VK_DestroyBottomAccelerationStructure(&vk_d.bottomASWorldDynamicAS[i]);
				VK_DestroyBottomAccelerationStructure(&vk_d.bottomASWorldDynamicASTrans[i]);
				for (int j = 0; j < vk_d.bottomASDynamicCount[i]; j++) {
					VK_DestroyBottomAccelerationStructure(&vk_d.bottomASDynamicList[i][j]);
				}
				VK_DestroyDescriptor(&vk_d.rtxDescriptor[i]);
				VK_DestroyDescriptor(&vk_d.computeDescriptor[i]);
				vk_d.bottomASDynamicCount[i] = 0;
			}
	
			vk_d.bottomASTraceListCount = 0;

			vk_d.geometry.cluster_world_static_offset = 0;
			vk_d.geometry.cluster_world_dynamic_data_offset = 0;
			vk_d.geometry.cluster_world_dynamic_as_offset = 0;
			vk_d.geometry.cluster_entity_static_offset = 0;

			vk_d.geometry.idx_world_static_offset = 0;
			vk_d.geometry.xyz_world_static_offset = 0;
			vk_d.geometry.idx_world_dynamic_data_offset = 0;
			vk_d.geometry.xyz_world_dynamic_data_offset = 0;
			for (int i = 0; i < vk.swapchain.imageCount; i++) {
				vk_d.geometry.idx_world_dynamic_as_offset[i] = 0;
				vk_d.geometry.xyz_world_dynamic_as_offset[i] = 0;
			}
			vk_d.geometry.idx_entity_static_offset = 0;
			vk_d.geometry.xyz_entity_static_offset = 0;
			vk_d.geometry.idx_entity_dynamic_offset = 0;
			vk_d.geometry.xyz_entity_dynamic_offset = 0;

			vk_d.basBufferEntityStaticOffset = 0;
			for (int i = 0; i < vk.swapchain.imageCount; i++) {
				vk_d.basBufferEntityDynamicOffset = 0;
			}

			vk_d.updateDataOffsetXYZCount = 0;
			vk_d.updateASOffsetXYZCount = 0;
			vk_d.scratchBufferOffset = 0;
			// </RTX>

			vk_d.offset = 0;
			vk_d.offsetIdx = 0;
		}
	}

	R_DoneFreeType();

	// shut down platform specific OpenGL/Vulkan stuff
	if (destroyWindow) {
		if(glConfig.driverType == OPENGL) GLimp_Shutdown();
		else if (glConfig.driverType == VULKAN) {
			VK_DestroyBuffer(&vk_d.indexbuffer);
			VK_DestroyBuffer(&vk_d.vertexbuffer);
			VK_DestroyBuffer(&vk_d.normalbuffer);
			VK_DestroyBuffer(&vk_d.uvbuffer1);
            VK_DestroyBuffer(&vk_d.uvbuffer2);
			VK_DestroyBuffer(&vk_d.colorbuffer);
			VK_DestroyDescriptor(&vk_d.imageDescriptor);

			// <RTX>
			// geometry buffer
			VK_DestroyBuffer(&vk_d.geometry.xyz_world_static);
			VK_DestroyBuffer(&vk_d.geometry.idx_world_static);
			for (int i = 0; i < vk.swapchain.imageCount; i++) {
				VK_DestroyBuffer(&vk_d.geometry.xyz_world_dynamic_data[i]);
				VK_DestroyBuffer(&vk_d.geometry.idx_world_dynamic_data[i]);
				VK_DestroyBuffer(&vk_d.geometry.xyz_world_dynamic_as[i]);
				VK_DestroyBuffer(&vk_d.geometry.idx_world_dynamic_as[i]);
			}
			VK_DestroyBuffer(&vk_d.geometry.xyz_entity_static);
			VK_DestroyBuffer(&vk_d.geometry.idx_entity_static);
			for (int i = 0; i < vk.swapchain.imageCount; i++) {
				VK_DestroyBuffer(&vk_d.geometry.xyz_entity_dynamic[i]);
				VK_DestroyBuffer(&vk_d.geometry.idx_entity_dynamic[i]);
			}
			for (int i = 0; i < vk.swapchain.imageCount; i++) {
				VK_DestroyBuffer(&vk_d.prevToCurrInstanceBuffer[i]);
			}

			// cluster buffer
			VK_DestroyBuffer(&vk_d.geometry.cluster_world_static);
			VK_DestroyBuffer(&vk_d.geometry.cluster_world_dynamic_data);
			VK_DestroyBuffer(&vk_d.geometry.cluster_world_dynamic_as);
			VK_DestroyBuffer(&vk_d.geometry.cluster_entity_static);
			
			// as buffer
			VK_DestroyBuffer(&vk_d.basBufferStaticWorld);
			VK_DestroyBuffer(&vk_d.basBufferWorldDynamicData);
			for (int i = 0; i < vk.swapchain.imageCount; i++) {
				VK_DestroyBuffer(&vk_d.basBufferWorldDynamicAS[i]);
			}
			VK_DestroyBuffer(&vk_d.basBufferEntityStatic);
			for (int i = 0; i < vk.swapchain.imageCount; i++) {
				VK_DestroyBuffer(&vk_d.basBufferEntityDynamic[i]);
			}
			
			for (int i = 0; i < vk.swapchain.imageCount; i++) {
				VK_DestroyBuffer(&vk_d.topASBuffer[i]);
				VK_DestroyBuffer(&vk_d.instanceBuffer[i]);
				VK_DestroyBuffer(&vk_d.instanceDataBuffer[i]);

				VK_DestroyBuffer(&vk_d.uboBuffer[i]);
				VK_DestroyBuffer(&vk_d.uboLightList[i]);
				VK_DestroyImage(&vk_d.accelerationStructures.resultImage[i]);
				VK_DestroyImage(&vk_d.accelerationStructures.accumulationImage[i]);
			}
			VK_DestroyBuffer(&vk_d.scratchBuffer);
			VK_DestroyImage(&vk_d.blueNoiseTex);

			// g buffer and asvgf
			for (int i = 0; i < vk.swapchain.imageCount; i++) {
				VK_DestroyImage(&vk_d.gBuffer[i].position);
				VK_DestroyImage(&vk_d.gBuffer[i].albedo);
				VK_DestroyImage(&vk_d.gBuffer[i].normals);
				VK_DestroyImage(&vk_d.gBuffer[i].viewDir);
				VK_DestroyImage(&vk_d.gBuffer[i].transparent);
				VK_DestroyImage(&vk_d.gBuffer[i].reflection);
				VK_DestroyImage(&vk_d.gBuffer[i].objectInfo);
				VK_DestroyImage(&vk_d.gBuffer[i].motion);
				VK_DestroyImage(&vk_d.gBuffer[i].directIllumination);
				VK_DestroyImage(&vk_d.gBuffer[i].indirectIllumination);
				VK_DestroyImage(&vk_d.gBuffer[i].depthNormal);
				VK_DestroyImage(&vk_d.gBuffer[i].result);
				VK_DestroyImage(&vk_d.gBuffer[i].maxmipmap);

				VK_DestroyImage(&vk_d.asvgf[i].debug);
				VK_DestroyImage(&vk_d.asvgf[i].positionFwd);
				VK_DestroyImage(&vk_d.asvgf[i].objectFwd);
				VK_DestroyImage(&vk_d.asvgf[i].rngSeed);
				VK_DestroyImage(&vk_d.asvgf[i].gradSamplePos);
				VK_DestroyImage(&vk_d.asvgf[i].gradA);
				VK_DestroyImage(&vk_d.asvgf[i].gradB);
				VK_DestroyImage(&vk_d.asvgf[i].color);
				VK_DestroyImage(&vk_d.asvgf[i].histColor);
				VK_DestroyImage(&vk_d.asvgf[i].histMoments);
				VK_DestroyImage(&vk_d.asvgf[i].atrousA);
				VK_DestroyImage(&vk_d.asvgf[i].atrousB);
				VK_DestroyImage(&vk_d.asvgf[i].taa);
			}

			// lists
			if(vk_d.bottomASList == NULL) free(vk_d.bottomASList);
			if (vk_d.bottomASTraceList == NULL) free(vk_d.bottomASTraceList);
			for (int i = 0; i < vk.swapchain.imageCount; i++) {
				if (vk_d.bottomASDynamicList[i] == NULL) free(vk_d.bottomASDynamicList[i]);
			}
			// </RTX>
			
			VK_DestroyAllPipelines();
			VK_DestroyAllShaders();
			VK_Destroy();

			VKimp_Shutdown();
		}
	}

	tr.registered = qfalse;
}


/*
=============
RE_EndRegistration

Touch all images to make sure they are resident
=============
*/
void RE_EndRegistration( void ) {
	R_SyncRenderThread();

	if (glConfig.driverType == OPENGL) {
		if (!Sys_LowPhysicalMemory()) {
			RB_ShowImages();
		}
	}
}


/*
@@@@@@@@@@@@@@@@@@@@@
GetRefAPI

@@@@@@@@@@@@@@@@@@@@@
*/
refexport_t *GetRefAPI ( int apiVersion, refimport_t *rimp ) {
	static refexport_t	re;

	ri = *rimp;

	Com_Memset( &re, 0, sizeof( re ) );

	if ( apiVersion != REF_API_VERSION ) {
		ri.Printf(PRINT_ALL, "Mismatched REF_API_VERSION: expected %i, got %i\n", 
			REF_API_VERSION, apiVersion );
		return NULL;
	}

	// the RE_ functions are Renderer Entry points

	re.Shutdown = RE_Shutdown;

	re.BeginRegistration = RE_BeginRegistration;
	re.RegisterModel = RE_RegisterModel;
	re.RegisterSkin = RE_RegisterSkin;
	re.RegisterShader = RE_RegisterShader;
	re.RegisterShaderNoMip = RE_RegisterShaderNoMip;
	re.LoadWorld = RE_LoadWorldMap;
	re.SetWorldVisData = RE_SetWorldVisData;
	re.EndRegistration = RE_EndRegistration;

	re.BeginFrame = RE_BeginFrame;
	re.EndFrame = RE_EndFrame;

	re.MarkFragments = R_MarkFragments;
	re.LerpTag = R_LerpTag;
	re.ModelBounds = R_ModelBounds;

	re.ClearScene = RE_ClearScene;
	re.AddRefEntityToScene = RE_AddRefEntityToScene;
	re.AddPolyToScene = RE_AddPolyToScene;
	re.LightForPoint = R_LightForPoint;
	re.AddLightToScene = RE_AddLightToScene;
	re.AddAdditiveLightToScene = RE_AddAdditiveLightToScene;
	re.RenderScene = RE_RenderScene;

	re.SetColor = RE_SetColor;
	re.DrawStretchPic = RE_StretchPic;
	re.DrawStretchRaw = RE_StretchRaw;
	re.UploadCinematic = RE_UploadCinematic;

	re.RegisterFont = RE_RegisterFont;
	re.RemapShader = R_RemapShader;
	re.GetEntityToken = R_GetEntityToken;
	re.inPVS = R_inPVS;

	return &re;
}
