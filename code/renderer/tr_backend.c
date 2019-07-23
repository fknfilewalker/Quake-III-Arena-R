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
#include "tr_local.h"

backEndData_t	*backEndData[SMP_FRAMES];
backEndState_t	backEnd;


static float	s_flipMatrix[16] = {
	// convert from our coordinate system (looking down X)
	// to OpenGL's coordinate system (looking down -Z)
	0, 0, -1, 0,
	-1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 0, 1
};


/*
** GL_Bind
*/
void GL_Bind( image_t *image ) {
	int texnum;

	if ( !image ) {
		ri.Printf( PRINT_WARNING, "GL_Bind: NULL image\n" );
		texnum = tr.defaultImage->texnum;
	} else {
		texnum = image->texnum;
	}

	if ( r_nobind->integer && tr.dlightImage ) {		// performance evaluation option
		texnum = tr.dlightImage->texnum;
	}

	if ( glState.currenttextures[glState.currenttmu] != texnum ) {
		image->frameUsed = tr.frameCount;
		glState.currenttextures[glState.currenttmu] = texnum;
		qglBindTexture (GL_TEXTURE_2D, texnum);
	}
}

void VK_Bind( image_t *image ) {
    int index;
    
    if ( !image ) {
        ri.Printf( PRINT_WARNING, "VK_Bind: NULL image\n" );
        index = tr.defaultImage->index;
    } else {
        index = image->index;
    }
    
    if ( r_nobind->integer && tr.dlightImage ) {        // performance evaluation option
        index = tr.dlightImage->index;
    }
    
    if ( vk_d.currentTexture[0] != index ) {
        image->frameUsed = tr.frameCount;
        vk_d.currentTexture[0] = index;
    }
//    if ( glState.currenttextures[glState.currenttmu] != texnum ) {
//        image->frameUsed = tr.frameCount;
//        glState.currenttextures[glState.currenttmu] = texnum;
//        qglBindTexture (GL_TEXTURE_2D, texnum);
//    }
}

/*
** GL_SelectTexture
*/
void GL_SelectTexture( int unit )
{
	if ( glState.currenttmu == unit )
	{
		return;
	}

	if ( unit == 0 )
	{
		qglActiveTextureARB( GL_TEXTURE0_ARB );
		GLimp_LogComment( "glActiveTextureARB( GL_TEXTURE0_ARB )\n" );
		qglClientActiveTextureARB( GL_TEXTURE0_ARB );
		GLimp_LogComment( "glClientActiveTextureARB( GL_TEXTURE0_ARB )\n" );
	}
	else if ( unit == 1 )
	{
		qglActiveTextureARB( GL_TEXTURE1_ARB );
		GLimp_LogComment( "glActiveTextureARB( GL_TEXTURE1_ARB )\n" );
		qglClientActiveTextureARB( GL_TEXTURE1_ARB );
		GLimp_LogComment( "glClientActiveTextureARB( GL_TEXTURE1_ARB )\n" );
	} else {
		ri.Error( ERR_DROP, "GL_SelectTexture: unit = %i", unit );
	}

	glState.currenttmu = unit;
}


/*
** GL_BindMultitexture
*/
void GL_BindMultitexture( image_t *image0, GLuint env0, image_t *image1, GLuint env1 ) {
	int		texnum0, texnum1;

	texnum0 = image0->texnum;
	texnum1 = image1->texnum;

	if ( r_nobind->integer && tr.dlightImage ) {		// performance evaluation option
		texnum0 = texnum1 = tr.dlightImage->texnum;
	}

	if ( glState.currenttextures[1] != texnum1 ) {
		GL_SelectTexture( 1 );
		image1->frameUsed = tr.frameCount;
		glState.currenttextures[1] = texnum1;
		qglBindTexture( GL_TEXTURE_2D, texnum1 );
	}
	if ( glState.currenttextures[0] != texnum0 ) {
		GL_SelectTexture( 0 );
		image0->frameUsed = tr.frameCount;
		glState.currenttextures[0] = texnum0;
		qglBindTexture( GL_TEXTURE_2D, texnum0 );
	}
}


/*
** GL_Cull
*/
void GL_Cull( int cullType ) {
	if ( glState.faceCulling == cullType ) {
		return;
	}

	glState.faceCulling = cullType;

	if ( cullType == CT_TWO_SIDED ) 
	{
		qglDisable( GL_CULL_FACE );
	} 
	else 
	{
		qglEnable( GL_CULL_FACE );

		if ( cullType == CT_BACK_SIDED )
		{
			if ( backEnd.viewParms.isMirror )
			{
				qglCullFace( GL_FRONT );
			}
			else
			{
				qglCullFace( GL_BACK );
			}
		}
		else
		{
			if ( backEnd.viewParms.isMirror )
			{
				qglCullFace( GL_BACK );
			}
			else
			{
				qglCullFace( GL_FRONT );
			}
		}
	}
}

void VK_Cull( int cullType ) {
    if ( vk_d.state.faceCulling == cullType ) {
        return;
    }
    
    vk_d.state.faceCulling = cullType;
    
    if ( cullType == CT_TWO_SIDED )
    {
        vk_d.state.cullMode = VK_CULL_MODE_NONE;
        //qglDisable( GL_CULL_FACE );
    }
    else
    {
        //qglEnable( GL_CULL_FACE );
        
        if ( cullType == CT_FRONT_SIDED )
        {
            if ( backEnd.viewParms.isMirror )
            {
                vk_d.state.cullMode = VK_CULL_MODE_FRONT_BIT;
                //qglCullFace( GL_FRONT );
            }
            else
            {
                vk_d.state.cullMode = VK_CULL_MODE_BACK_BIT;
                //qglCullFace( GL_BACK );
            }
        }
        else
        {
            if ( backEnd.viewParms.isMirror )
            {
                vk_d.state.cullMode = VK_CULL_MODE_BACK_BIT;
                //qglCullFace( GL_BACK );
            }
            else
            {
                vk_d.state.cullMode = VK_CULL_MODE_FRONT_BIT;
                //qglCullFace( GL_FRONT );
            }
        }
    }
}

/*
** GL_TexEnv
*/
void GL_TexEnv( int env )
{
	if ( env == glState.texEnv[glState.currenttmu] )
	{
		return;
	}

	glState.texEnv[glState.currenttmu] = env;


	switch ( env )
	{
	case GL_MODULATE:
		qglTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
		break;
	case GL_REPLACE:
		qglTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
		break;
	case GL_DECAL:
		qglTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );
		break;
	case GL_ADD:
		qglTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD );
		break;
	default:
		ri.Error( ERR_DROP, "GL_TexEnv: invalid env '%d' passed\n", env );
		break;
	}
}

/*
** GL_State
**
** This routine is responsible for setting the most commonly changed state
** in Q3.
*/
void GL_State( unsigned long stateBits )
{
	unsigned long diff = stateBits ^ glState.glStateBits;

	if ( !diff )
	{
		return;
	}

	//
	// check depthFunc bits
	//
	if ( diff & GLS_DEPTHFUNC_EQUAL )
	{
		if ( stateBits & GLS_DEPTHFUNC_EQUAL )
		{
			qglDepthFunc( GL_EQUAL );
		}
		else
		{
			qglDepthFunc( GL_LEQUAL );
		}
	}

	//
	// check blend bits
	//
	if ( diff & ( GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS ) )
	{
		GLenum srcFactor, dstFactor;

		if ( stateBits & ( GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS ) )
		{
			switch ( stateBits & GLS_SRCBLEND_BITS )
			{
			case GLS_SRCBLEND_ZERO:
				srcFactor = GL_ZERO;
				break;
			case GLS_SRCBLEND_ONE:
				srcFactor = GL_ONE;
				break;
			case GLS_SRCBLEND_DST_COLOR:
				srcFactor = GL_DST_COLOR;
				break;
			case GLS_SRCBLEND_ONE_MINUS_DST_COLOR:
				srcFactor = GL_ONE_MINUS_DST_COLOR;
				break;
			case GLS_SRCBLEND_SRC_ALPHA:
				srcFactor = GL_SRC_ALPHA;
				break;
			case GLS_SRCBLEND_ONE_MINUS_SRC_ALPHA:
				srcFactor = GL_ONE_MINUS_SRC_ALPHA;
				break;
			case GLS_SRCBLEND_DST_ALPHA:
				srcFactor = GL_DST_ALPHA;
				break;
			case GLS_SRCBLEND_ONE_MINUS_DST_ALPHA:
				srcFactor = GL_ONE_MINUS_DST_ALPHA;
				break;
			case GLS_SRCBLEND_ALPHA_SATURATE:
				srcFactor = GL_SRC_ALPHA_SATURATE;
				break;
			default:
				srcFactor = GL_ONE;		// to get warning to shut up
				ri.Error( ERR_DROP, "GL_State: invalid src blend state bits\n" );
				break;
			}

			switch ( stateBits & GLS_DSTBLEND_BITS )
			{
			case GLS_DSTBLEND_ZERO:
				dstFactor = GL_ZERO;
				break;
			case GLS_DSTBLEND_ONE:
				dstFactor = GL_ONE;
				break;
			case GLS_DSTBLEND_SRC_COLOR:
				dstFactor = GL_SRC_COLOR;
				break;
			case GLS_DSTBLEND_ONE_MINUS_SRC_COLOR:
				dstFactor = GL_ONE_MINUS_SRC_COLOR;
				break;
			case GLS_DSTBLEND_SRC_ALPHA:
				dstFactor = GL_SRC_ALPHA;
				break;
			case GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA:
				dstFactor = GL_ONE_MINUS_SRC_ALPHA;
				break;
			case GLS_DSTBLEND_DST_ALPHA:
				dstFactor = GL_DST_ALPHA;
				break;
			case GLS_DSTBLEND_ONE_MINUS_DST_ALPHA:
				dstFactor = GL_ONE_MINUS_DST_ALPHA;
				break;
			default:
				dstFactor = GL_ONE;		// to get warning to shut up
				ri.Error( ERR_DROP, "GL_State: invalid dst blend state bits\n" );
				break;
			}

			qglEnable( GL_BLEND );
			qglBlendFunc( srcFactor, dstFactor );
		}
		else
		{
			qglDisable( GL_BLEND );
		}
	}

	//
	// check depthmask
	//
	if ( diff & GLS_DEPTHMASK_TRUE )
	{
		if ( stateBits & GLS_DEPTHMASK_TRUE )
		{
			qglDepthMask( GL_TRUE );
		}
		else
		{
			qglDepthMask( GL_FALSE );
		}
	}

	//
	// fill/line mode
	//
	if ( diff & GLS_POLYMODE_LINE )
	{
		if ( stateBits & GLS_POLYMODE_LINE )
		{
			qglPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		}
		else
		{
			qglPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		}
	}

	//
	// depthtest
	//
	if ( diff & GLS_DEPTHTEST_DISABLE )
	{
		if ( stateBits & GLS_DEPTHTEST_DISABLE )
		{
			qglDisable( GL_DEPTH_TEST );
		}
		else
		{
			qglEnable( GL_DEPTH_TEST );
		}
	}

	//
	// alpha test
	//
	if ( diff & GLS_ATEST_BITS )
	{
		switch ( stateBits & GLS_ATEST_BITS )
		{
		case 0:
			qglDisable( GL_ALPHA_TEST );
			break;
		case GLS_ATEST_GT_0:
			qglEnable( GL_ALPHA_TEST );
			qglAlphaFunc( GL_GREATER, 0.0f );
			break;
		case GLS_ATEST_LT_80:
			qglEnable( GL_ALPHA_TEST );
			qglAlphaFunc( GL_LESS, 0.5f );
			break;
		case GLS_ATEST_GE_80:
			qglEnable( GL_ALPHA_TEST );
			qglAlphaFunc( GL_GEQUAL, 0.5f );
			break;
		default:
			assert( 0 );
			break;
		}
	}

	glState.glStateBits = stateBits;
}

void VK_State( unsigned long stateBits )
{
    unsigned long diff = stateBits ^ vk_d.state.blendBits;
    
    if ( !diff )
    {
        return;
    }
    
//    VkPipelineDepthStencilStateCreateInfo depthStencil = { 0 };
//    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
//    depthStencil.depthTestEnable = VK_TRUE;
//    depthStencil.depthWriteEnable = VK_TRUE;
//    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

//
//    VkPipelineColorBlendAttachmentState colorBlend = {0};
//    colorBlend.colorWriteMask = 0xF;
//    colorBlend.blendEnable = VK_TRUE;
//    colorBlend.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
//    colorBlend.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
//    colorBlend.colorBlendOp = VK_BLEND_OP_ADD;
//    colorBlend.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
//    colorBlend.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
//    colorBlend.alphaBlendOp = VK_BLEND_OP_ADD;
    
    //
    // check depthFunc bits
    //
    if ( diff & GLS_DEPTHFUNC_EQUAL )
    {
        if ( stateBits & GLS_DEPTHFUNC_EQUAL )
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
    if ( diff & ( GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS ) )
    {
        if ( stateBits & ( GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS ) )
        {
            switch ( stateBits & GLS_SRCBLEND_BITS )
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
                    ri.Error( ERR_DROP, "GL_State: invalid src blend state bits\n" );
                    break;
            }
            
            switch ( stateBits & GLS_DSTBLEND_BITS )
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
                    ri.Error( ERR_DROP, "GL_State: invalid dst blend state bits\n" );
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
    if ( diff & GLS_DEPTHMASK_TRUE )
    {
        if ( stateBits & GLS_DEPTHMASK_TRUE )
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
    if ( diff & GLS_POLYMODE_LINE )
    {
        if ( stateBits & GLS_POLYMODE_LINE )
        {
            vk_d.state.polygonMode = VK_POLYGON_MODE_LINE;
            //qglPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        }
        else
        {
            vk_d.state.polygonMode = VK_POLYGON_MODE_LINE;
            //qglPolygonMode( GL_FRONT_AND_BACK, VK_POLYGON_MODE_FILL );
        }
    }
    
    //
    // depthtest
    //
    if ( diff & GLS_DEPTHTEST_DISABLE )
    {
        if ( stateBits & GLS_DEPTHTEST_DISABLE )
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
    if ( diff & GLS_ATEST_BITS )
    {
        switch ( stateBits & GLS_ATEST_BITS )
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
                assert( 0 );
                break;
        }
    }
    
    vk_d.state.blendBits = stateBits;
}


/*
================
RB_Hyperspace

A player has predicted a teleport, but hasn't arrived yet
================
*/
static void RB_Hyperspace( void ) {
	float		c;

	if ( !backEnd.isHyperspace ) {
		// do initialization shit
	}

	c = ( backEnd.refdef.time & 255 ) / 255.0f;
    if ( !Q_stricmp(r_glDriver->string, OPENGL_DRIVER_NAME) ) {
        qglClearColor( c, c, c, 1 );
        qglClear( GL_COLOR_BUFFER_BIT );
    } else if ( !Q_stricmp(r_glDriver->string, VULKAN_DRIVER_NAME) ) {
        VK_ClearAttachments(qfalse, qfalse, qtrue, (vec4_t){c, c, c, 1});
    }

	backEnd.isHyperspace = qtrue;
}


static void SetViewportAndScissor( void ) {
    if ( !Q_stricmp(r_glDriver->string, OPENGL_DRIVER_NAME) ) {
        qglMatrixMode(GL_PROJECTION);
        qglLoadMatrixf( backEnd.viewParms.projectionMatrix );
        qglMatrixMode(GL_MODELVIEW);
        
        // set the window clipping
        qglViewport( backEnd.viewParms.viewportX, backEnd.viewParms.viewportY,
                    backEnd.viewParms.viewportWidth, backEnd.viewParms.viewportHeight );
        qglScissor( backEnd.viewParms.viewportX, backEnd.viewParms.viewportY,
                   backEnd.viewParms.viewportWidth, backEnd.viewParms.viewportHeight );
    } else if ( !Q_stricmp(r_glDriver->string, VULKAN_DRIVER_NAME) ) {
        
        const float* p = backEnd.viewParms.projectionMatrix;
        
        // update q3's proj matrix (opengl) to vulkan conventions: z - [0, 1] instead of [-1, 1] and invert y direction
        float zNear    = r_znear->value;
        float zFar = backEnd.viewParms.zFar;
        float P10 = -zFar / (zFar - zNear);
        float P14 = -zFar*zNear / (zFar - zNear);
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
}

/*
=================
RB_BeginDrawingView

Any mirrored or portaled views have already been drawn, so prepare
to actually render the visible surfaces for this view
=================
*/
void RB_BeginDrawingView (void) {
	int clearBits = 0;
    vec4_t clearColor = {0, 0, 0, 0};

	// sync with gl if needed
	if ( r_finish->integer == 1 && !glState.finishCalled ) {
		qglFinish ();
		glState.finishCalled = qtrue;
	}
	if ( r_finish->integer == 0 ) {
		glState.finishCalled = qtrue;
	}

	// we will need to change the projection matrix before drawing
	// 2D images again
	backEnd.projection2D = qfalse;

	//
	// set the modelview matrix for the viewer
	//
	SetViewportAndScissor();

	// ensures that depth writes are enabled for the depth clear
    if ( !Q_stricmp(r_glDriver->string, OPENGL_DRIVER_NAME) ) {
        GL_State( GLS_DEFAULT );
    } else if ( !Q_stricmp(r_glDriver->string, VULKAN_DRIVER_NAME) ) {
        VK_State( GLS_DEFAULT );
    }
	// clear relevant buffers
	clearBits = GL_DEPTH_BUFFER_BIT;

	if ( r_measureOverdraw->integer || r_shadows->integer == 2 )
	{
		clearBits |= GL_STENCIL_BUFFER_BIT;
	}
	if ( r_fastsky->integer && !( backEnd.refdef.rdflags & RDF_NOWORLDMODEL ) )
	{
		clearBits |= GL_COLOR_BUFFER_BIT;	// FIXME: only if sky shaders have been used
#ifdef _DEBUG
        clearColor[0] = 0.8f; clearColor[1] = 0.7f; clearColor[2] = 0.4f; clearColor[3] = 1.0f;
#else
        clearColor[0] = clearColor[1] = clearColor[2] = 0.0f; clearColor[3] = 1.0f;
#endif
        if ( !Q_stricmp(r_glDriver->string, OPENGL_DRIVER_NAME) ) {
            qglClearColor( clearColor[0], clearColor[1], clearColor[2], clearColor[3] );    // FIXME: get color of sky
        }
	}
    if ( !Q_stricmp(r_glDriver->string, OPENGL_DRIVER_NAME) ) {
        qglClear( clearBits );
    } else if ( !Q_stricmp(r_glDriver->string, VULKAN_DRIVER_NAME) ) {
        VK_ClearAttachments(clearBits & GL_DEPTH_BUFFER_BIT, clearBits & GL_STENCIL_BUFFER_BIT, clearBits & GL_COLOR_BUFFER_BIT, clearColor);
    }
    
	if ( ( backEnd.refdef.rdflags & RDF_HYPERSPACE ) )
	{
		RB_Hyperspace();
		return;
	}
	else
	{
		backEnd.isHyperspace = qfalse;
	}

	glState.faceCulling = -1;		// force face culling to set next time

	// we will only draw a sun if there was sky rendered in this view
	backEnd.skyRenderedThisView = qfalse;

	// clip to the plane of the portal
	if ( backEnd.viewParms.isPortal ) {
		float	plane[4];
		double	plane2[4];

		plane[0] = backEnd.viewParms.portalPlane.normal[0];
		plane[1] = backEnd.viewParms.portalPlane.normal[1];
		plane[2] = backEnd.viewParms.portalPlane.normal[2];
		plane[3] = backEnd.viewParms.portalPlane.dist;

		plane2[0] = DotProduct (backEnd.viewParms.or.axis[0], plane);
		plane2[1] = DotProduct (backEnd.viewParms.or.axis[1], plane);
		plane2[2] = DotProduct (backEnd.viewParms.or.axis[2], plane);
		plane2[3] = DotProduct (plane, backEnd.viewParms.or.origin) - plane[3];
		
        if ( !Q_stricmp(r_glDriver->string, OPENGL_DRIVER_NAME) ) {
            qglLoadMatrixf( s_flipMatrix );
            qglClipPlane (GL_CLIP_PLANE0, plane2);
            qglEnable (GL_CLIP_PLANE0);
        } else if ( !Q_stricmp(r_glDriver->string, VULKAN_DRIVER_NAME) ) {
            //Com_Memcpy(vk_d.state.plane, plane2, 16);
            vk_d.state.plane[0] = -plane2[1];
            vk_d.state.plane[1] =  plane2[2];
            vk_d.state.plane[2] = -plane2[0];
            vk_d.state.plane[3] =  plane2[3];
            vk_d.state.clip = qtrue;
        }
	} else {
        if ( !Q_stricmp(r_glDriver->string, OPENGL_DRIVER_NAME) ) {
            qglDisable (GL_CLIP_PLANE0);
        } else if ( !Q_stricmp(r_glDriver->string, VULKAN_DRIVER_NAME) ) {
            vk_d.state.clip = qfalse;
        }
	}
}


#define	MAC_EVENT_PUMP_MSEC		5

/*
==================
RB_RenderDrawSurfList
==================
*/
void RB_RenderDrawSurfList( drawSurf_t *drawSurfs, int numDrawSurfs ) {
	shader_t		*shader, *oldShader;
	int				fogNum, oldFogNum;
	int				entityNum, oldEntityNum;
	int				dlighted, oldDlighted;
	qboolean		depthRange, oldDepthRange;
	int				i;
	drawSurf_t		*drawSurf;
	int				oldSort;
	float			originalTime;


	// save original time for entity shader offsets
	originalTime = backEnd.refdef.floatTime;

	// clear the z buffer, set the modelview, etc
	RB_BeginDrawingView ();

	// draw everything
	oldEntityNum = -1;
	backEnd.currentEntity = &tr.worldEntity;
	oldShader = NULL;
	oldFogNum = -1;
	oldDepthRange = qfalse;
	oldDlighted = qfalse;
	oldSort = -1;
	depthRange = qfalse;

	backEnd.pc.c_surfaces += numDrawSurfs;

	for (i = 0, drawSurf = drawSurfs ; i < numDrawSurfs ; i++, drawSurf++) {
		if ( drawSurf->sort == oldSort ) {
			// fast path, same as previous sort
			rb_surfaceTable[ *drawSurf->surface ]( drawSurf->surface );
			continue;
		}
		oldSort = drawSurf->sort;
		R_DecomposeSort( drawSurf->sort, &entityNum, &shader, &fogNum, &dlighted );

		//
		// change the tess parameters if needed
		// a "entityMergable" shader is a shader that can have surfaces from seperate
		// entities merged into a single batch, like smoke and blood puff sprites
		if (shader != oldShader || fogNum != oldFogNum || dlighted != oldDlighted 
			|| ( entityNum != oldEntityNum && !shader->entityMergable ) ) {
			if (oldShader != NULL) {

				RB_EndSurface();
			}
			RB_BeginSurface( shader, fogNum );
			oldShader = shader;
			oldFogNum = fogNum;
			oldDlighted = dlighted;
		}

		//
		// change the modelview matrix if needed
		//
		if ( entityNum != oldEntityNum ) {
			depthRange = qfalse;

			if ( entityNum != ENTITYNUM_WORLD ) {
				backEnd.currentEntity = &backEnd.refdef.entities[entityNum];
				backEnd.refdef.floatTime = originalTime - backEnd.currentEntity->e.shaderTime;
				// we have to reset the shaderTime as well otherwise image animations start
				// from the wrong frame
				tess.shaderTime = backEnd.refdef.floatTime - tess.shader->timeOffset;

				// set up the transformation matrix
				R_RotateForEntity( backEnd.currentEntity, &backEnd.viewParms, &backEnd.or );

				// set up the dynamic lighting if needed
				if ( backEnd.currentEntity->needDlights ) {
					R_TransformDlights( backEnd.refdef.num_dlights, backEnd.refdef.dlights, &backEnd.or );
				}

				if ( backEnd.currentEntity->e.renderfx & RF_DEPTHHACK ) {
					// hack the depth range to prevent view model from poking into walls
					depthRange = qtrue;
				}
			} else {
				backEnd.currentEntity = &tr.worldEntity;
				backEnd.refdef.floatTime = originalTime;
				backEnd.or = backEnd.viewParms.world;
				// we have to reset the shaderTime as well otherwise image animations on
				// the world (like water) continue with the wrong frame
				tess.shaderTime = backEnd.refdef.floatTime - tess.shader->timeOffset;
				R_TransformDlights( backEnd.refdef.num_dlights, backEnd.refdef.dlights, &backEnd.or );
			}

            if ( !Q_stricmp(r_glDriver->string, OPENGL_DRIVER_NAME) ) {
                qglLoadMatrixf( backEnd.or.modelMatrix );
            } else if ( !Q_stricmp(r_glDriver->string, VULKAN_DRIVER_NAME) ) {
                Com_Memcpy(vk_d.modelViewMatrix, backEnd.or.modelMatrix, 64);
            }

			//
			// change depthrange if needed
			//
			if ( oldDepthRange != depthRange ) {
                if ( !Q_stricmp(r_glDriver->string, OPENGL_DRIVER_NAME) ) {
                    if ( depthRange ) {
                        qglDepthRange (0, 0.3);
                    } else {
                        qglDepthRange (0, 1);
                    }
                } else if ( !Q_stricmp(r_glDriver->string, VULKAN_DRIVER_NAME) ) {
                    if ( depthRange ) {
                        vk_d.viewport.minDepth = 0.0f;
                        vk_d.viewport.maxDepth = 0.3f;
                    } else {
                        vk_d.viewport.minDepth = 0.0f;
                        vk_d.viewport.maxDepth = 1.0f;
                    }
                }
				oldDepthRange = depthRange;
			}

			oldEntityNum = entityNum;
		}

		// add the triangles for this surface
		rb_surfaceTable[ *drawSurf->surface ]( drawSurf->surface );
	}

	backEnd.refdef.floatTime = originalTime;

	// draw the contents of the last shader batch
	if (oldShader != NULL) {
		RB_EndSurface();
	}

	// go back to the world modelview matrix
    if ( !Q_stricmp(r_glDriver->string, OPENGL_DRIVER_NAME) ) {
        qglLoadMatrixf( backEnd.viewParms.world.modelMatrix );
    } else if ( !Q_stricmp(r_glDriver->string, VULKAN_DRIVER_NAME) ) {
        Com_Memcpy(vk_d.modelViewMatrix, backEnd.viewParms.world.modelMatrix, 64);
    }
	if ( depthRange ) {
        if ( !Q_stricmp(r_glDriver->string, OPENGL_DRIVER_NAME) ) {
            qglDepthRange (0, 1);
        } else if ( !Q_stricmp(r_glDriver->string, VULKAN_DRIVER_NAME) ) {
            vk_d.viewport.minDepth = 0.0f;
            vk_d.viewport.maxDepth = 1.0f;
        }
		
	}

#if 0
	RB_DrawSun();
#endif
	// darken down any stencil shadows
	RB_ShadowFinish();		

	// add light flares on lights that aren't obscured
	RB_RenderFlares();
}


/*
============================================================================

RENDER BACK END THREAD FUNCTIONS

============================================================================
*/

/*
================
RB_SetGL2D

================
*/
void	RB_SetGL2D (void) {
	backEnd.projection2D = qtrue;

    if ( !Q_stricmp(r_glDriver->string, OPENGL_DRIVER_NAME) ) {
        // set 2D virtual screen size
        qglViewport( 0, 0, glConfig.vidWidth, glConfig.vidHeight );
        qglScissor( 0, 0, glConfig.vidWidth, glConfig.vidHeight );
        qglMatrixMode(GL_PROJECTION);
        qglLoadIdentity ();
        qglOrtho (0, glConfig.vidWidth, glConfig.vidHeight, 0, 0, 1);
        qglMatrixMode(GL_MODELVIEW);
        qglLoadIdentity ();

        GL_State( GLS_DEPTHTEST_DISABLE |
                  GLS_SRCBLEND_SRC_ALPHA |
                  GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA );

        qglDisable( GL_CULL_FACE );
        qglDisable( GL_CLIP_PLANE0 );
    } else if ( !Q_stricmp(r_glDriver->string, VULKAN_DRIVER_NAME) ) {
        
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
        
        VK_State( GLS_DEPTHTEST_DISABLE |
                 GLS_SRCBLEND_SRC_ALPHA |
                 GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA );
        
        vk_d.state.cullMode = VK_CULL_MODE_NONE;
    }

	// set time for 2D shaders
	backEnd.refdef.time = ri.Milliseconds();
	backEnd.refdef.floatTime = backEnd.refdef.time * 0.001f;
}


/*
=============
RE_StretchRaw

FIXME: not exactly backend
Stretches a raw 32 bit power of 2 bitmap image over the given screen rectangle.
Used for cinematics.
=============
*/
void RE_StretchRaw (int x, int y, int w, int h, int cols, int rows, const byte *data, int client, qboolean dirty) {
	int			i, j;
	int			start, end;

    if ( !tr.registered ) {
        return;
    }
    R_SyncRenderThread();
    
    start = end = 0;
    if ( r_speeds->integer ) {
        start = ri.Milliseconds();
    }
    
    // make sure rows and cols are powers of 2
    for ( i = 0 ; ( 1 << i ) < cols ; i++ ) {
    }
    for ( j = 0 ; ( 1 << j ) < rows ; j++ ) {
    }
    if ( ( 1 << i ) != cols || ( 1 << j ) != rows) {
        ri.Error (ERR_DROP, "Draw_StretchRaw: size not a power of 2: %i by %i", cols, rows);
    }
    
    RE_UploadCinematic(w, h, cols, rows, data, client, dirty);
    
    if ( r_speeds->integer ) {
        end = ri.Milliseconds();
        ri.Printf( PRINT_ALL, "qglTexSubImage2D %i, %i: %i msec\n", cols, rows, end - start );
    }
    
    tr.cinematicShader->stages[0]->bundle[0].image[0] = tr.scratchImage[client];
    RE_StretchPic(x, y, w, h,  0.5f / cols, 0.5f / rows,  1.0f - 0.5f / cols, 1.0f - 0.5 / rows, tr.cinematicShader->index);
}

void RE_UploadCinematic (int w, int h, int cols, int rows, const byte *data, int client, qboolean dirty) {

    if (!Q_stricmp(r_glDriver->string, OPENGL_DRIVER_NAME)) {
        GL_Bind( tr.scratchImage[client] );
        
        // if the scratchImage isn't in the format we want, specify it as a new texture
        if ( cols != tr.scratchImage[client]->width || rows != tr.scratchImage[client]->height ) {
            tr.scratchImage[client]->width = tr.scratchImage[client]->uploadWidth = cols;
            tr.scratchImage[client]->height = tr.scratchImage[client]->uploadHeight = rows;
            qglTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8, cols, rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );
            qglTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
            qglTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
            qglTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
            qglTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
        } else {
            if (dirty) {
                // otherwise, just subimage upload it so that drivers can tell we are going to be changing
                // it and don't try and do a texture compression
                qglTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, cols, rows, GL_RGBA, GL_UNSIGNED_BYTE, data );
            }
        }
    } else if (!Q_stricmp(r_glDriver->string, VULKAN_DRIVER_NAME)) {
        if ( cols != tr.scratchImage[client]->width || rows != tr.scratchImage[client]->height ) {
            tr.scratchImage[client]->width = tr.scratchImage[client]->uploadWidth = cols;
            tr.scratchImage[client]->height = tr.scratchImage[client]->uploadHeight = rows;
            
            vkimage_t *image = &vk_d.images[tr.scratchImage[client]->index];
            VK_DestroyImage(image);
            
            VK_CreateImage(image, cols, rows, VK_FORMAT_R8G8B8A8_UNORM, 1);
            VK_UploadImageData(image, cols, rows, data, 4, 0); // rows wise
            VK_CreateSampler(image, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
            
            VK_AddSampler(&image->descriptor_set, 0, VK_SHADER_STAGE_FRAGMENT_BIT);
            VK_SetSampler(&image->descriptor_set, 0, VK_SHADER_STAGE_FRAGMENT_BIT, image->sampler, image->view);
            VK_FinishDescriptor(&image->descriptor_set);
        }
        else {
            if (dirty) {
                VK_UploadImageData(&vk_d.images[tr.scratchImage[client]->index], cols, rows, data, 4, 0);   
            }
        }
    }
}


/*
=============
RB_SetColor

=============
*/
const void	*RB_SetColor( const void *data ) {
	const setColorCommand_t	*cmd;

	cmd = (const setColorCommand_t *)data;

	backEnd.color2D[0] = cmd->color[0] * 255;
	backEnd.color2D[1] = cmd->color[1] * 255;
	backEnd.color2D[2] = cmd->color[2] * 255;
	backEnd.color2D[3] = cmd->color[3] * 255;

	return (const void *)(cmd + 1);
}

/*
=============
RB_StretchPic
=============
*/
const void *RB_StretchPic ( const void *data ) {
	const stretchPicCommand_t	*cmd;
	shader_t *shader;
	int		numVerts, numIndexes;

	cmd = (const stretchPicCommand_t *)data;

    if ( !backEnd.projection2D ) {
        RB_SetGL2D();
    }
    
	shader = cmd->shader;
	if ( shader != tess.shader ) {
		if ( tess.numIndexes ) {
			RB_EndSurface();
		}
		backEnd.currentEntity = &backEnd.entity2D;
		RB_BeginSurface( shader, 0 );
	}

	RB_CHECKOVERFLOW( 4, 6 );
	numVerts = tess.numVertexes;
	numIndexes = tess.numIndexes;

	tess.numVertexes += 4;
	tess.numIndexes += 6;

	tess.indexes[ numIndexes ] = numVerts + 3;
	tess.indexes[ numIndexes + 1 ] = numVerts + 0;
	tess.indexes[ numIndexes + 2 ] = numVerts + 2;
	tess.indexes[ numIndexes + 3 ] = numVerts + 2;
	tess.indexes[ numIndexes + 4 ] = numVerts + 0;
	tess.indexes[ numIndexes + 5 ] = numVerts + 1;

	*(int *)tess.vertexColors[ numVerts ] =
		*(int *)tess.vertexColors[ numVerts + 1 ] =
		*(int *)tess.vertexColors[ numVerts + 2 ] =
		*(int *)tess.vertexColors[ numVerts + 3 ] = *(int *)backEnd.color2D;

	tess.xyz[ numVerts ][0] = cmd->x;
	tess.xyz[ numVerts ][1] = cmd->y;
	tess.xyz[ numVerts ][2] = 0;

	tess.texCoords[ numVerts ][0][0] = cmd->s1;
	tess.texCoords[ numVerts ][0][1] = cmd->t1;

	tess.xyz[ numVerts + 1 ][0] = cmd->x + cmd->w;
	tess.xyz[ numVerts + 1 ][1] = cmd->y;
	tess.xyz[ numVerts + 1 ][2] = 0;

	tess.texCoords[ numVerts + 1 ][0][0] = cmd->s2;
	tess.texCoords[ numVerts + 1 ][0][1] = cmd->t1;

	tess.xyz[ numVerts + 2 ][0] = cmd->x + cmd->w;
	tess.xyz[ numVerts + 2 ][1] = cmd->y + cmd->h;
	tess.xyz[ numVerts + 2 ][2] = 0;

	tess.texCoords[ numVerts + 2 ][0][0] = cmd->s2;
	tess.texCoords[ numVerts + 2 ][0][1] = cmd->t2;

	tess.xyz[ numVerts + 3 ][0] = cmd->x;
	tess.xyz[ numVerts + 3 ][1] = cmd->y + cmd->h;
	tess.xyz[ numVerts + 3 ][2] = 0;

	tess.texCoords[ numVerts + 3 ][0][0] = cmd->s1;
	tess.texCoords[ numVerts + 3 ][0][1] = cmd->t2;

	return (const void *)(cmd + 1);
}


/*
=============
RB_DrawSurfs

=============
*/
const void	*RB_DrawSurfs( const void *data ) {
	const drawSurfsCommand_t	*cmd;

	// finish any 2D drawing if needed
	if ( tess.numIndexes ) {
		RB_EndSurface();
	}

	cmd = (const drawSurfsCommand_t *)data;

	backEnd.refdef = cmd->refdef;
	backEnd.viewParms = cmd->viewParms;

	RB_RenderDrawSurfList( cmd->drawSurfs, cmd->numDrawSurfs );

	return (const void *)(cmd + 1);
}


/*
=============
RB_DrawBuffer

=============
*/
const void	*RB_DrawBuffer( const void *data ) {
	const drawBufferCommand_t	*cmd;

	cmd = (const drawBufferCommand_t *)data;

    if (!Q_stricmp(r_glDriver->string, OPENGL_DRIVER_NAME)) {
        qglDrawBuffer( cmd->buffer );

        // clear screen for debugging
        if ( r_clear->integer ) {
            qglClearColor( 1, 0, 0.5, 1 );
            qglClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        }
    } else if (!Q_stricmp(r_glDriver->string, VULKAN_DRIVER_NAME)) {
        VK_BeginFrame();
        beginRenderClear();
        if ( r_clear->integer ) {
            VK_ClearAttachments(qfalse, qfalse, qtrue, (vec4_t){1, 0, 0.5, 1});
        }
    }

	return (const void *)(cmd + 1);
}

/*
===============
RB_ShowImages

Draw all the images to the screen, on top of whatever
was there.  This is used to test for texture thrashing.

Also called by RE_EndRegistration
===============
*/
void RB_ShowImages( void ) {
	int		i;
	image_t	*image;
	float	x, y, w, h;
	int		start, end;

	if ( !backEnd.projection2D ) {
		RB_SetGL2D();
	}

	qglClear( GL_COLOR_BUFFER_BIT );

	qglFinish();

	start = ri.Milliseconds();

	for ( i=0 ; i<tr.numImages ; i++ ) {
		image = tr.images[i];

		w = glConfig.vidWidth / 20;
		h = glConfig.vidHeight / 15;
		x = i % 20 * w;
		y = i / 20 * h;

		// show in proportional size in mode 2
		if ( r_showImages->integer == 2 ) {
			w *= image->uploadWidth / 512.0f;
			h *= image->uploadHeight / 512.0f;
		}

		GL_Bind( image );
		qglBegin (GL_QUADS);
		qglTexCoord2f( 0, 0 );
		qglVertex2f( x, y );
		qglTexCoord2f( 1, 0 );
		qglVertex2f( x + w, y );
		qglTexCoord2f( 1, 1 );
		qglVertex2f( x + w, y + h );
		qglTexCoord2f( 0, 1 );
		qglVertex2f( x, y + h );
		qglEnd();
	}

	qglFinish();

	end = ri.Milliseconds();
	ri.Printf( PRINT_ALL, "%i msec to draw all images\n", end - start );

}


/*
=============
RB_SwapBuffers

=============
*/
const void	*RB_SwapBuffers( const void *data ) {
	const swapBuffersCommand_t	*cmd;

	// finish any 2D drawing if needed
	if ( tess.numIndexes ) {
		RB_EndSurface();
	}

	// texture swapping test
	if ( r_showImages->integer ) {
		RB_ShowImages();
	}

	cmd = (const swapBuffersCommand_t *)data;

	// we measure overdraw by reading back the stencil buffer and
	// counting up the number of increments that have happened
	if ( r_measureOverdraw->integer ) {
		int i;
		long sum = 0;
		unsigned char *stencilReadback;

		stencilReadback = ri.Hunk_AllocateTempMemory( glConfig.vidWidth * glConfig.vidHeight );
		qglReadPixels( 0, 0, glConfig.vidWidth, glConfig.vidHeight, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, stencilReadback );

		for ( i = 0; i < glConfig.vidWidth * glConfig.vidHeight; i++ ) {
			sum += stencilReadback[i];
		}

		backEnd.pc.c_overDraw += sum;
		ri.Hunk_FreeTempMemory( stencilReadback );
	}

    if (!Q_stricmp(r_glDriver->string, OPENGL_DRIVER_NAME)) {
        if ( !glState.finishCalled ) {
            qglFinish();
        }

        GLimp_LogComment( "***************** RB_SwapBuffers *****************\n\n\n" );

        GLimp_EndFrame();
    } else if (!Q_stricmp(r_glDriver->string, VULKAN_DRIVER_NAME)) {
        endRender();
        VK_EndFrame();
    }

	backEnd.projection2D = qfalse;

	return (const void *)(cmd + 1);
}

/*
====================
RB_ExecuteRenderCommands

This function will be called synchronously if running without
smp extensions, or asynchronously by another thread.
====================
*/
void RB_ExecuteRenderCommands( const void *data ) {
	int		t1, t2;

	t1 = ri.Milliseconds ();

	if ( !r_smp->integer || data == backEndData[0]->commands.cmds ) {
		backEnd.smpFrame = 0;
	} else {
		backEnd.smpFrame = 1;
	}

	while ( 1 ) {
		switch ( *(const int *)data ) {
		case RC_SET_COLOR:
			data = RB_SetColor( data );
			break;
		case RC_STRETCH_PIC:
			data = RB_StretchPic( data );
			break;
		case RC_DRAW_SURFS:
			data = RB_DrawSurfs( data );
			break;
		case RC_DRAW_BUFFER:
			data = RB_DrawBuffer( data );
			break;
		case RC_SWAP_BUFFERS:
			data = RB_SwapBuffers( data );
			break;
		case RC_SCREENSHOT:
			data = RB_TakeScreenshotCmd( data );
			break;

		case RC_END_OF_LIST:
		default:
            /*if (!Q_stricmp(r_glDriver->string, VULKAN_DRIVER_NAME) && com_errorEntered && vk.swapchain.frameStarted) {
                endRender();
                VK_EndFrame();
            }*/
			// stop rendering on this thread
			t2 = ri.Milliseconds ();
			backEnd.pc.msec = t2 - t1;
			return;
		}
	}

}


/*
================
RB_RenderThread
================
*/
void RB_RenderThread( void ) {
	const void	*data;

	// wait for either a rendering command or a quit command
	while ( 1 ) {
		// sleep until we have work to do
		data = GLimp_RendererSleep();

		if ( !data ) {
			return;	// all done, renderer is shutting down
		}

		renderThreadActive = qtrue;

		RB_ExecuteRenderCommands( data );

		renderThreadActive = qfalse;
	}
}

