
#include "tr_local.h"


static void State(unsigned long stateBits)
{
	unsigned long diff = stateBits ^ vk_d.state.blendBits;

	if (!diff)
	{
		//return;
	}

	//
	// check depthFunc bits
	//
	if (diff & GLS_DEPTHFUNC_EQUAL)
	{
		if (stateBits & GLS_DEPTHFUNC_EQUAL)
		{
			//qglDepthFunc( GL_EQUAL );
			vk_d.state.dsBlend.depthCompareOp = VK_COMPARE_OP_EQUAL;
		}
		else
		{
			//qglDepthFunc( GL_LEQUAL );
			vk_d.state.dsBlend.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		}
	}

	//
	// check blend bits
	//
	if (diff & (GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS))
	{
		if (stateBits & (GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS))
		{
			switch (stateBits & GLS_SRCBLEND_BITS)
			{
			case GLS_SRCBLEND_ZERO:
				vk_d.state.colorBlend.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
				//srcFactor = GL_ZERO;
				break;
			case GLS_SRCBLEND_ONE:
				vk_d.state.colorBlend.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
				//srcFactor = GL_ONE;
				break;
			case GLS_SRCBLEND_DST_COLOR:
				vk_d.state.colorBlend.srcColorBlendFactor = VK_BLEND_FACTOR_DST_COLOR;
				//srcFactor = GL_DST_COLOR;
				break;
			case GLS_SRCBLEND_ONE_MINUS_DST_COLOR:
				vk_d.state.colorBlend.srcColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
				//srcFactor = GL_ONE_MINUS_DST_COLOR;
				break;
			case GLS_SRCBLEND_SRC_ALPHA:
				vk_d.state.colorBlend.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
				//srcFactor = GL_SRC_ALPHA;
				break;
			case GLS_SRCBLEND_ONE_MINUS_SRC_ALPHA:
				vk_d.state.colorBlend.srcColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
				//srcFactor = GL_ONE_MINUS_SRC_ALPHA;
				break;
			case GLS_SRCBLEND_DST_ALPHA:
				vk_d.state.colorBlend.srcColorBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
				//srcFactor = GL_DST_ALPHA;
				break;
			case GLS_SRCBLEND_ONE_MINUS_DST_ALPHA:
				vk_d.state.colorBlend.srcColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
				//srcFactor = GL_ONE_MINUS_DST_ALPHA;
				break;
			case GLS_SRCBLEND_ALPHA_SATURATE:
				vk_d.state.colorBlend.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
				//srcFactor = GL_SRC_ALPHA_SATURATE;
				break;
			default:
				vk_d.state.colorBlend.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
				//srcFactor = GL_ONE;        // to get warning to shut up
				ri.Error(ERR_DROP, "GL_State: invalid src blend state bits\n");
				break;
			}

			switch (stateBits & GLS_DSTBLEND_BITS)
			{
			case GLS_DSTBLEND_ZERO:
				vk_d.state.colorBlend.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
				//dstFactor = GL_ZERO;
				break;
			case GLS_DSTBLEND_ONE:
				vk_d.state.colorBlend.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
				//dstFactor = GL_ONE;
				break;
			case GLS_DSTBLEND_SRC_COLOR:
				vk_d.state.colorBlend.dstColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
				//dstFactor = GL_SRC_COLOR;
				break;
			case GLS_DSTBLEND_ONE_MINUS_SRC_COLOR:
				vk_d.state.colorBlend.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
				//dstFactor = GL_ONE_MINUS_SRC_COLOR;
				break;
			case GLS_DSTBLEND_SRC_ALPHA:
				vk_d.state.colorBlend.dstColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
				//dstFactor = GL_SRC_ALPHA;
				break;
			case GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA:
				vk_d.state.colorBlend.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
				//dstFactor = GL_ONE_MINUS_SRC_ALPHA;
				break;
			case GLS_DSTBLEND_DST_ALPHA:
				vk_d.state.colorBlend.dstColorBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
				//dstFactor = GL_DST_ALPHA;
				break;
			case GLS_DSTBLEND_ONE_MINUS_DST_ALPHA:
				vk_d.state.colorBlend.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
				//dstFactor = GL_ONE_MINUS_DST_ALPHA;
				break;
			default:
				vk_d.state.colorBlend.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
				//dstFactor = GL_ONE;        // to get warning to shut up
				ri.Error(ERR_DROP, "GL_State: invalid dst blend state bits\n");
				break;
			}

			vk_d.state.colorBlend.blendEnable = VK_TRUE;
			vk_d.state.colorBlend.colorBlendOp = VK_BLEND_OP_ADD;
			vk_d.state.colorBlend.alphaBlendOp = VK_BLEND_OP_ADD;
			vk_d.state.colorBlend.srcAlphaBlendFactor = vk_d.state.colorBlend.srcColorBlendFactor;
			vk_d.state.colorBlend.dstAlphaBlendFactor = vk_d.state.colorBlend.dstColorBlendFactor;
			//qglEnable( GL_BLEND );
			//qglBlendFunc( srcFactor, dstFactor );
		}
		else
		{
			vk_d.state.colorBlend.blendEnable = VK_FALSE;
			//qglDisable( GL_BLEND );
		}
	}

	//
	// check depthmask
	//
	if (diff & GLS_DEPTHMASK_TRUE)
	{
		if (stateBits & GLS_DEPTHMASK_TRUE)
		{
			vk_d.state.dsBlend.depthWriteEnable = VK_TRUE;
			//qglDepthMask( GL_TRUE );
		}
		else
		{
			vk_d.state.dsBlend.depthWriteEnable = VK_FALSE;
			//qglDepthMask( GL_FALSE );
		}
	}

	//
	// fill/line mode
	//
	if (diff & GLS_POLYMODE_LINE)
	{
		if (stateBits & GLS_POLYMODE_LINE)
		{
			vk_d.state.polygonMode = VK_POLYGON_MODE_LINE;
			//qglPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		}
		else
		{
			vk_d.state.polygonMode = VK_POLYGON_MODE_FILL;
			//qglPolygonMode( GL_FRONT_AND_BACK, VK_POLYGON_MODE_FILL );
		}
	}

	//
	// depthtest
	//
	if (diff & GLS_DEPTHTEST_DISABLE)
	{
		if (stateBits & GLS_DEPTHTEST_DISABLE)
		{
			vk_d.state.dsBlend.depthTestEnable = VK_FALSE;
			//qglDisable( GL_DEPTH_TEST );
		}
		else
		{
			vk_d.state.dsBlend.depthTestEnable = VK_TRUE;
			//qglEnable( GL_DEPTH_TEST );
		}
	}

	//
	// alpha test
	//
	if (diff & GLS_ATEST_BITS)
	{
		switch (stateBits & GLS_ATEST_BITS)
		{
		case 0:
			vk_d.discardModeAlpha = 0;
			//vk_d.state.colorBlend.
			//qglDisable( GL_ALPHA_TEST );
			break;
		case GLS_ATEST_GT_0:
			vk_d.discardModeAlpha = 1;
			//qglEnable( GL_ALPHA_TEST );
			//qglAlphaFunc( GL_GREATER, 0.0f );
			break;
		case GLS_ATEST_LT_80:
			vk_d.discardModeAlpha = 2;
			//qglEnable( GL_ALPHA_TEST );
			//qglAlphaFunc( GL_LESS, 0.5f );
			break;
		case GLS_ATEST_GE_80:
			vk_d.discardModeAlpha = 3;
			//qglEnable( GL_ALPHA_TEST );
			//qglAlphaFunc( GL_GEQUAL, 0.5f );
			break;
		default:
			assert(0);
			break;
		}
	}

	vk_d.state.blendBits = stateBits;
}

static void SetViewportAndScissor(void) {
	
	const float* p = backEnd.viewParms.projectionMatrix;

	// update q3's proj matrix (opengl) to vulkan conventions: z - [0, 1] instead of [-1, 1] and invert y direction
	float zNear = r_znear->value;
	float zFar = backEnd.viewParms.zFar;
	float P10 = -zFar / (zFar - zNear);
	float P14 = -zFar * zNear / (zFar - zNear);
	float P5 = -p[5];

	float proj[16] = {
		p[0],  p[1],  p[2], p[3],
		p[4],  P5,    p[6], p[7],
		p[8],  p[9],  P10,  p[11],
		p[12], p[13], P14,  p[15]
	};
	Com_Memcpy(vk_d.projectionMatrix, proj, 64);

	vk_d.viewport.x = backEnd.viewParms.viewportX;
	vk_d.viewport.y = glConfig.vidHeight - (backEnd.viewParms.viewportY + backEnd.viewParms.viewportHeight);
	vk_d.viewport.width = backEnd.viewParms.viewportWidth;
	vk_d.viewport.height = backEnd.viewParms.viewportHeight;
	vk_d.viewport.minDepth = 0;
	vk_d.viewport.maxDepth = 1;

	vk_d.scissor.offset.x = backEnd.viewParms.viewportX;
	vk_d.scissor.offset.y = vk_d.viewport.y < 0 ? 0 : vk_d.viewport.y;
	vk_d.scissor.extent.width = backEnd.viewParms.viewportWidth;
	vk_d.scissor.extent.height = backEnd.viewParms.viewportHeight;
}

/*
============================================================================

RENDER BACK END THREAD FUNCTIONS

============================================================================
*/
static void RB_Set2D(void) {
	backEnd.projection2D = qtrue;

	Com_Memset(vk_d.modelViewMatrix, 0, 64);
	vk_d.modelViewMatrix[0] = vk_d.modelViewMatrix[5] = vk_d.modelViewMatrix[10] = vk_d.modelViewMatrix[15] = 1;

	float top = 0;
	float bottom = glConfig.vidHeight;
	float left = 0;
	float right = glConfig.vidWidth;
	float zNear = 0;
	float zFar = 1;

	float mvp0 = 2.0f / (right - left);
	float mvp5 = 2.0f / (bottom - top);
	float mvp10 = 1.0f / (zNear - zFar);
	float mvp12 = -(right - left) / (right - left);
	float mvp13 = -(bottom + top) / (bottom - top);
	float mvp14 = zNear / (zNear - zFar);

	vk_d.projectionMatrix[0] = mvp0; vk_d.projectionMatrix[1] = 0.0f; vk_d.projectionMatrix[2] = 0.0f; vk_d.projectionMatrix[3] = 0.0f;
	vk_d.projectionMatrix[4] = 0.0f; vk_d.projectionMatrix[5] = mvp5; vk_d.projectionMatrix[6] = 0.0f; vk_d.projectionMatrix[7] = 0.0f;
	vk_d.projectionMatrix[8] = 0.0f; vk_d.projectionMatrix[9] = 0.0f; vk_d.projectionMatrix[10] = 1.0f; vk_d.projectionMatrix[11] = 0.0f;
	vk_d.projectionMatrix[12] = mvp12; vk_d.projectionMatrix[13] = mvp13; vk_d.projectionMatrix[14] = mvp14; vk_d.projectionMatrix[15] = 1.0f;

	vk_d.scissor.offset.x = 0;
	vk_d.scissor.offset.y = 0;
	vk_d.scissor.extent.width = glConfig.vidWidth;
	vk_d.scissor.extent.height = glConfig.vidHeight;

	vk_d.viewport.x = 0;
	vk_d.viewport.y = 0;
	vk_d.viewport.width = glConfig.vidWidth;
	vk_d.viewport.height = glConfig.vidHeight;
	vk_d.viewport.minDepth = 0;
	vk_d.viewport.maxDepth = 1;

	State(GLS_DEPTHTEST_DISABLE |
		GLS_SRCBLEND_SRC_ALPHA |
		GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA);

	vk_d.state.cullMode = VK_CULL_MODE_NONE;

	// set time for 2D shaders
	backEnd.refdef.time = ri.Milliseconds();
	backEnd.refdef.floatTime = backEnd.refdef.time * 0.001f;
}

void R_SetVulkanApi( trApi_t *api ) {
	api->State = State;
	api->SetViewportAndScissor = SetViewportAndScissor;
	api->RB_Set2D = RB_Set2D;
}
