
#include "tr_local.h"

/*
** GL_State
**
** This routine is responsible for setting the most commonly changed state
** in Q3.
*/
static void GL_State(unsigned long stateBits)
{
	unsigned long diff = stateBits ^ glState.glStateBits;

	if (!diff)
	{
		return;
	}

	//
	// check depthFunc bits
	//
	if (diff & GLS_DEPTHFUNC_EQUAL)
	{
		if (stateBits & GLS_DEPTHFUNC_EQUAL)
		{
			qglDepthFunc(GL_EQUAL);
		}
		else
		{
			qglDepthFunc(GL_LEQUAL);
		}
	}

	//
	// check blend bits
	//
	if (diff & (GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS))
	{
		GLenum srcFactor, dstFactor;

		if (stateBits & (GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS))
		{
			switch (stateBits & GLS_SRCBLEND_BITS)
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
				ri.Error(ERR_DROP, "GL_State: invalid src blend state bits\n");
				break;
			}

			switch (stateBits & GLS_DSTBLEND_BITS)
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
				ri.Error(ERR_DROP, "GL_State: invalid dst blend state bits\n");
				break;
			}

			qglEnable(GL_BLEND);
			qglBlendFunc(srcFactor, dstFactor);
		}
		else
		{
			qglDisable(GL_BLEND);
		}
	}

	//
	// check depthmask
	//
	if (diff & GLS_DEPTHMASK_TRUE)
	{
		if (stateBits & GLS_DEPTHMASK_TRUE)
		{
			qglDepthMask(GL_TRUE);
		}
		else
		{
			qglDepthMask(GL_FALSE);
		}
	}

	//
	// fill/line mode
	//
	if (diff & GLS_POLYMODE_LINE)
	{
		if (stateBits & GLS_POLYMODE_LINE)
		{
			qglPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else
		{
			qglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}

	//
	// depthtest
	//
	if (diff & GLS_DEPTHTEST_DISABLE)
	{
		if (stateBits & GLS_DEPTHTEST_DISABLE)
		{
			qglDisable(GL_DEPTH_TEST);
		}
		else
		{
			qglEnable(GL_DEPTH_TEST);
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
			qglDisable(GL_ALPHA_TEST);
			break;
		case GLS_ATEST_GT_0:
			qglEnable(GL_ALPHA_TEST);
			qglAlphaFunc(GL_GREATER, 0.0f);
			break;
		case GLS_ATEST_LT_80:
			qglEnable(GL_ALPHA_TEST);
			qglAlphaFunc(GL_LESS, 0.5f);
			break;
		case GLS_ATEST_GE_80:
			qglEnable(GL_ALPHA_TEST);
			qglAlphaFunc(GL_GEQUAL, 0.5f);
			break;
		default:
			assert(0);
			break;
		}
	}

	glState.glStateBits = stateBits;
}

/*
** GL_Cull
*/
static void GL_Cull(int cullType) {
	if (glState.faceCulling == cullType) {
		return;
	}

	glState.faceCulling = cullType;

	if (cullType == CT_TWO_SIDED)
	{
		qglDisable(GL_CULL_FACE);
	}
	else
	{
		qglEnable(GL_CULL_FACE);

		if (cullType == CT_BACK_SIDED)
		{
			if (backEnd.viewParms.isMirror)
			{
				qglCullFace(GL_FRONT);
			}
			else
			{
				qglCullFace(GL_BACK);
			}
		}
		else
		{
			if (backEnd.viewParms.isMirror)
			{
				qglCullFace(GL_BACK);
			}
			else
			{
				qglCullFace(GL_FRONT);
			}
		}
	}
}

static void SetViewportAndScissor(void) {
	qglMatrixMode(GL_PROJECTION);
	qglLoadMatrixf(backEnd.viewParms.projectionMatrix);
	qglMatrixMode(GL_MODELVIEW);

	// set the window clipping
	qglViewport(backEnd.viewParms.viewportX, backEnd.viewParms.viewportY,
		backEnd.viewParms.viewportWidth, backEnd.viewParms.viewportHeight);
	qglScissor(backEnd.viewParms.viewportX, backEnd.viewParms.viewportY,
		backEnd.viewParms.viewportWidth, backEnd.viewParms.viewportHeight);
}

/*
 ================
 R_ArrayElementDiscrete
 
 This is just for OpenGL conformance testing, it should never be the fastest
 ================
 */
static void APIENTRY R_ArrayElementDiscrete( GLint index ) {
    qglColor4ubv( tess.svars.colors[ index ] );
    if ( glState.currenttmu ) {
        qglMultiTexCoord2fARB( 0, tess.svars.texcoords[ 0 ][ index ][0], tess.svars.texcoords[ 0 ][ index ][1] );
        qglMultiTexCoord2fARB( 1, tess.svars.texcoords[ 1 ][ index ][0], tess.svars.texcoords[ 1 ][ index ][1] );
    } else {
        qglTexCoord2fv( tess.svars.texcoords[ 0 ][ index ] );
    }
    qglVertex3fv( tess.xyz[ index ] );
}

/*
 ===================
 R_DrawStripElements
 
 ===================
 */
static int        c_vertexes;        // for seeing how long our average strips are
static int        c_begins;
static void R_DrawStripElements( int numIndexes, const glIndex_t *indexes, void ( APIENTRY *element )(GLint) ) {
    int i;
    int last[3] = { -1, -1, -1 };
    qboolean even;
    
    c_begins++;
    
    if ( numIndexes <= 0 ) {
        return;
    }
    
    qglBegin( GL_TRIANGLE_STRIP );
    
    // prime the strip
    element( indexes[0] );
    element( indexes[1] );
    element( indexes[2] );
    c_vertexes += 3;
    
    last[0] = indexes[0];
    last[1] = indexes[1];
    last[2] = indexes[2];
    
    even = qfalse;
    
    for ( i = 3; i < numIndexes; i += 3 )
    {
        // odd numbered triangle in potential strip
        if ( !even )
        {
            // check previous triangle to see if we're continuing a strip
            if ( ( indexes[i+0] == last[2] ) && ( indexes[i+1] == last[1] ) )
            {
                element( indexes[i+2] );
                c_vertexes++;
                assert( indexes[i+2] < tess.numVertexes );
                even = qtrue;
            }
            // otherwise we're done with this strip so finish it and start
            // a new one
            else
            {
                qglEnd();
                
                qglBegin( GL_TRIANGLE_STRIP );
                c_begins++;
                
                element( indexes[i+0] );
                element( indexes[i+1] );
                element( indexes[i+2] );
                
                c_vertexes += 3;
                
                even = qfalse;
            }
        }
        else
        {
            // check previous triangle to see if we're continuing a strip
            if ( ( last[2] == indexes[i+1] ) && ( last[0] == indexes[i+0] ) )
            {
                element( indexes[i+2] );
                c_vertexes++;
                
                even = qfalse;
            }
            // otherwise we're done with this strip so finish it and start
            // a new one
            else
            {
                qglEnd();
                
                qglBegin( GL_TRIANGLE_STRIP );
                c_begins++;
                
                element( indexes[i+0] );
                element( indexes[i+1] );
                element( indexes[i+2] );
                c_vertexes += 3;
                
                even = qfalse;
            }
        }
        
        // cache the last three vertices
        last[0] = indexes[i+0];
        last[1] = indexes[i+1];
        last[2] = indexes[i+2];
    }
    
    qglEnd();
}

/*
 ==================
 R_DrawElements
 
 Optionally performs our own glDrawElements that looks for strip conditions
 instead of using the single glDrawElements call that may be inefficient
 without compiled vertex arrays.
 ==================
 */
static void R_DrawElements( int numIndexes, const glIndex_t *indexes ) {
    int        primitives;
    
    primitives = r_primitives->integer;

    // default is to use triangles if compiled vertex arrays are present
    if (primitives == 0) {
        if (qglLockArraysEXT) {
            primitives = 2;
        }
        else {
            primitives = 1;
        }
    }
    
    
    if (primitives == 2) {
        qglDrawElements(GL_TRIANGLES,
                        numIndexes,
                        GL_INDEX_TYPE,
                        indexes);
        return;
    }
    
    if (primitives == 1) {
        R_DrawStripElements(numIndexes, indexes, qglArrayElement);
        return;
    }
    
    if (primitives == 3) {
        R_DrawStripElements(numIndexes, indexes, R_ArrayElementDiscrete);
        return;
    }
    
    // anything else will cause no drawing
}

/*
============================================================================

RENDER BACK END THREAD FUNCTIONS

============================================================================
*/
static void RB_Set2D(void) {
	backEnd.projection2D = qtrue;

	// set 2D virtual screen size
	qglViewport(0, 0, glConfig.vidWidth, glConfig.vidHeight);
	qglScissor(0, 0, glConfig.vidWidth, glConfig.vidHeight);
	qglMatrixMode(GL_PROJECTION);
	qglLoadIdentity();
	qglOrtho(0, glConfig.vidWidth, glConfig.vidHeight, 0, 0, 1);
	qglMatrixMode(GL_MODELVIEW);
	qglLoadIdentity();

	GL_State(GLS_DEPTHTEST_DISABLE |
		GLS_SRCBLEND_SRC_ALPHA |
		GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA);

	qglDisable(GL_CULL_FACE);
	qglDisable(GL_CLIP_PLANE0);

	// set time for 2D shaders
	backEnd.refdef.time = ri.Milliseconds();
	backEnd.refdef.floatTime = backEnd.refdef.time * 0.001f;
}

void R_SetOpenGLApi(trApi_t* api) {
	api->Cull = GL_Cull;
	api->State = GL_State;
	api->SetViewportAndScissor = SetViewportAndScissor;
	api->RB_Set2D = RB_Set2D;
    api->R_DrawElements = R_DrawElements;
}
