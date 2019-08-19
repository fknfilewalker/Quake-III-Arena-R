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


/*

  for a projection shadow:

  point[x] += light vector * ( z - shadow plane )
  point[y] +=
  point[z] = shadow plane

  1 0 light[x] / light[z]

*/

typedef struct {
	int		i2;
	int		facing;
} edgeDef_t;

#define	MAX_EDGE_DEFS	32

static	edgeDef_t	edgeDefs[SHADER_MAX_VERTEXES][MAX_EDGE_DEFS];
static	int			numEdgeDefs[SHADER_MAX_VERTEXES];
static	int			facing[SHADER_MAX_INDEXES/3];
static  vec4_t      extrudedEdges[SHADER_MAX_VERTEXES * 4];
static  int         numExtrudedEdges;

void R_AddEdgeDef( int i1, int i2, int facing ) {
	int		c;

	c = numEdgeDefs[ i1 ];
	if ( c == MAX_EDGE_DEFS ) {
		return;		// overflow
	}
	edgeDefs[ i1 ][ c ].i2 = i2;
	edgeDefs[ i1 ][ c ].facing = facing;

	numEdgeDefs[ i1 ]++;
}

static void R_ExtrudeShadowEdges( void ) {
    int        i;
    int        c, c2;
    int        j, k;
    int        i2;
    
    numExtrudedEdges = 0;
    
    // an edge is NOT a silhouette edge if its face doesn't face the light,
    // or if it has a reverse paired edge that also faces the light.
    // A well behaved polyhedron would have exactly two faces for each edge,
    // but lots of models have dangling edges or overfanned edges
    for ( i = 0 ; i < tess.numVertexes ; i++ ) {
        c = numEdgeDefs[ i ];
        for ( j = 0 ; j < c ; j++ ) {
            if ( !edgeDefs[ i ][ j ].facing ) {
                continue;
            }
            
            qboolean sil_edge = qtrue;
            i2 = edgeDefs[ i ][ j ].i2;
            c2 = numEdgeDefs[ i2 ];
            for ( k = 0 ; k < c2 ; k++ ) {
                if ( edgeDefs[ i2 ][ k ].i2 == i && edgeDefs[ i2 ][ k ].facing) {
                    sil_edge = qfalse;
                    break;
                }
            }
            
            // if it doesn't share the edge with another front facing
            // triangle, it is a sil edge
            if ( sil_edge ) {
                VectorCopy(tess.xyz[ i ],                        extrudedEdges[numExtrudedEdges * 4 + 0]);
                VectorCopy(tess.xyz[ i + tess.numVertexes ],    extrudedEdges[numExtrudedEdges * 4 + 1]);
                VectorCopy(tess.xyz[ i2 ],                        extrudedEdges[numExtrudedEdges * 4 + 2]);
                VectorCopy(tess.xyz[ i2 + tess.numVertexes ],    extrudedEdges[numExtrudedEdges * 4 + 3]);
                numExtrudedEdges++;
            }
        }
    }
}

void R_RenderShadowEdges( void ) {
    if(glConfig.driverType == OPENGL){
        qglBegin( GL_QUADS);
        for (int i = 0; i < numExtrudedEdges; i++) {
            qglVertex3fv(extrudedEdges[i*4 + 0]);
            qglVertex3fv(extrudedEdges[i*4 + 1]);
            qglVertex3fv(extrudedEdges[i*4 + 3]);
            qglVertex3fv(extrudedEdges[i*4 + 2]);
        }
        qglEnd();
    } else if(glConfig.driverType == VULKAN){
        tess.numVertexes = numExtrudedEdges * 4;
		tess.numIndexes = numExtrudedEdges * 6;
        for (int i = 0; i < numExtrudedEdges; i++) {
            tess.indexes[i*6 + 0] = i*4 + 0;
            tess.indexes[i*6 + 1] = i*4 + 2;
            tess.indexes[i*6 + 2] = i*4 + 1;
            tess.indexes[i*6 + 3] = i*4 + 2;
            tess.indexes[i*6 + 4] = i*4 + 3;
            tess.indexes[i*6 + 5] = i*4 + 1;
        }
        
		for (int k = 0; k < tess.numVertexes; k++) {
			VectorSet(tess.svars.colors[k], 51, 51, 51);
			tess.svars.colors[k][3] = 255;
		}

        VK_UploadAttribDataOffset(&vk_d.vertexbuffer, vk_d.offset * sizeof(vec4_t), tess.numVertexes * sizeof(vec4_t), (void*)&extrudedEdges[0]);
        VK_UploadAttribDataOffset(&vk_d.colorbuffer, vk_d.offset * sizeof(color4ub_t), tess.numVertexes * sizeof(color4ub_t), (void *) &tess.svars.colors[0]);
        VK_UploadAttribDataOffset(&vk_d.uvbuffer1, vk_d.offset * sizeof(vec2_t), tess.numVertexes * sizeof(vec2_t), (void *) &tess.svars.texcoords[0]);

        myGlMultMatrix(vk_d.modelViewMatrix, vk_d.projectionMatrix, vk_d.mvp);
        tr_api.R_DrawElements(tess.numIndexes, tess.indexes);

        vk_d.offset += tess.numVertexes;
        vk_d.offsetIdx += tess.numIndexes;
    }
}

/*
=================
RB_ShadowTessEnd

triangleFromEdge[ v1 ][ v2 ]


  set triangle from edge( v1, v2, tri )
  if ( facing[ triangleFromEdge[ v1 ][ v2 ] ] && !facing[ triangleFromEdge[ v2 ][ v1 ] ) {
  }
=================
*/
void RB_ShadowTessEnd( void ) {
	int		i;
	int		numTris;
	vec3_t	lightDir;

	// we can only do this if we have enough space in the vertex buffers
	if ( tess.numVertexes >= SHADER_MAX_VERTEXES / 2 ) {
		return;
	}

	if ( glConfig.stencilBits < 4 ) {
		return;
	}

	VectorCopy( backEnd.currentEntity->lightDir, lightDir );

	// project vertexes away from light direction
	for ( i = 0 ; i < tess.numVertexes ; i++ ) {
		VectorMA( tess.xyz[i], -512, lightDir, tess.xyz[i+tess.numVertexes] );
	}

	// decide which triangles face the light
	Com_Memset( numEdgeDefs, 0, 4 * tess.numVertexes );

	numTris = tess.numIndexes / 3;
	for ( i = 0 ; i < numTris ; i++ ) {
		int		i1, i2, i3;
		vec3_t	d1, d2, normal;
		float	*v1, *v2, *v3;
		float	d;

		i1 = tess.indexes[ i*3 + 0 ];
		i2 = tess.indexes[ i*3 + 1 ];
		i3 = tess.indexes[ i*3 + 2 ];

		v1 = tess.xyz[ i1 ];
		v2 = tess.xyz[ i2 ];
		v3 = tess.xyz[ i3 ];

		VectorSubtract( v2, v1, d1 );
		VectorSubtract( v3, v1, d2 );
		CrossProduct( d1, d2, normal );

		d = DotProduct( normal, lightDir );
		if ( d > 0 ) {
			facing[ i ] = 1;
		} else {
			facing[ i ] = 0;
		}

		// create the edges
		R_AddEdgeDef( i1, i2, facing[ i ] );
		R_AddEdgeDef( i2, i3, facing[ i ] );
		R_AddEdgeDef( i3, i1, facing[ i ] );
	}

    R_ExtrudeShadowEdges();
    if(glConfig.driverType == OPENGL){
        // draw the silhouette edges
        GL_Bind( tr.whiteImage );
        qglEnable( GL_CULL_FACE );
        tr_api.State( GLS_SRCBLEND_ONE | GLS_DSTBLEND_ZERO );
        qglColor3f( 0.2f, 0.2f, 0.2f );

        // don't write to the color buffer
        qglColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );

        qglEnable( GL_STENCIL_TEST );
        qglStencilFunc( GL_ALWAYS, 1, 255 );

        // mirrors have the culling order reversed
        if ( backEnd.viewParms.isMirror ) {
            qglCullFace( GL_FRONT );
            qglStencilOp( GL_KEEP, GL_KEEP, GL_INCR );

            R_RenderShadowEdges();

            qglCullFace( GL_BACK );
            qglStencilOp( GL_KEEP, GL_KEEP, GL_DECR );

            R_RenderShadowEdges();
        } else {
            qglCullFace( GL_BACK );
            qglStencilOp( GL_KEEP, GL_KEEP, GL_INCR );

            R_RenderShadowEdges();

            qglCullFace( GL_FRONT );
            qglStencilOp( GL_KEEP, GL_KEEP, GL_DECR );

            R_RenderShadowEdges();
        }

        // reenable writing to the color buffer
        qglColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
    } else if (glConfig.driverType == VULKAN){
        // draw the silhouette edges
        VK_Bind( tr.whiteImage );
        tr_api.State( GLS_SRCBLEND_ONE | GLS_DSTBLEND_ZERO );
        //qglColor3f( 0.2f, 0.2f, 0.2f );
        
        // don't write to the color buffer
        vk_d.state.colorBlend.blendEnable = VK_TRUE;
        vk_d.state.colorBlend.colorBlendOp = VK_BLEND_OP_ADD;
        vk_d.state.colorBlend.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        vk_d.state.colorBlend.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        vk_d.state.colorBlend.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        vk_d.state.colorBlend.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
        
        vk_d.state.dsBlend.stencilTestEnable = VK_TRUE;
        VkStencilOpState stencil = {0};
        stencil.compareOp = VK_COMPARE_OP_ALWAYS;
        stencil.reference = 1;
        stencil.writeMask = 255;
        stencil.compareMask = 255;
        
        // mirrors have the culling order reversed
        if ( backEnd.viewParms.isMirror ) {
            vk_d.state.cullMode = VK_CULL_MODE_FRONT_BIT;
            stencil.failOp = VK_STENCIL_OP_KEEP;
            stencil.depthFailOp = VK_STENCIL_OP_KEEP;
            stencil.passOp = VK_STENCIL_OP_INCREMENT_AND_CLAMP;
            vk_d.state.dsBlend.front = vk_d.state.dsBlend.back = stencil;
  
            R_RenderShadowEdges();
            
            vk_d.state.cullMode = VK_CULL_MODE_BACK_BIT;
            stencil.failOp = VK_STENCIL_OP_KEEP;
            stencil.depthFailOp = VK_STENCIL_OP_KEEP;
            stencil.passOp = VK_STENCIL_OP_DECREMENT_AND_CLAMP;
            vk_d.state.dsBlend.front = vk_d.state.dsBlend.back = stencil;
    
            R_RenderShadowEdges();
        } else {
            vk_d.state.cullMode = VK_CULL_MODE_BACK_BIT;
            stencil.failOp = VK_STENCIL_OP_KEEP;
            stencil.depthFailOp = VK_STENCIL_OP_KEEP;
            stencil.passOp = VK_STENCIL_OP_INCREMENT_AND_CLAMP;
            vk_d.state.dsBlend.front = vk_d.state.dsBlend.back = stencil;

            R_RenderShadowEdges();
            
            vk_d.state.cullMode = VK_CULL_MODE_FRONT_BIT;
            stencil.failOp = VK_STENCIL_OP_KEEP;
            stencil.depthFailOp = VK_STENCIL_OP_KEEP;
            stencil.passOp = VK_STENCIL_OP_DECREMENT_AND_CLAMP;
            vk_d.state.dsBlend.front = vk_d.state.dsBlend.back = stencil;

            R_RenderShadowEdges();
        }
    }
}


/*
=================
RB_ShadowFinish

Darken everything that is is a shadow volume.
We have to delay this until everything has been shadowed,
because otherwise shadows from different body parts would
overlap and double darken.
=================
*/
void RB_ShadowFinish( void ) {

	if ( r_shadows->integer != 2 ) {
		return;
	}
	if ( glConfig.stencilBits < 4 ) {
		return;
	}
    if(glConfig.driverType == OPENGL){
        
        qglEnable( GL_STENCIL_TEST );
        qglStencilFunc( GL_NOTEQUAL, 0, 255 );

        qglDisable (GL_CLIP_PLANE0);
        qglDisable (GL_CULL_FACE);

        GL_Bind( tr.whiteImage );

        qglLoadIdentity ();

        qglColor3f( 0.6f, 0.6f, 0.6f );
        tr_api.State( GLS_DEPTHMASK_TRUE | GLS_SRCBLEND_DST_COLOR | GLS_DSTBLEND_ZERO );

        qglBegin( GL_QUADS );
        qglVertex3f( -100, 100, -10 );
        qglVertex3f( 100, 100, -10 );
        qglVertex3f( 100, -100, -10 );
        qglVertex3f( -100, -100, -10 );
        qglEnd ();

        qglColor4f(1,1,1,1);
        qglDisable( GL_STENCIL_TEST );
    } else if(glConfig.driverType == VULKAN){
        //return;
        vk_d.state.dsBlend.stencilTestEnable = VK_TRUE;
        VkStencilOpState stencil = {0};
        stencil.compareOp = VK_COMPARE_OP_NOT_EQUAL;
        stencil.reference = 0;
        stencil.writeMask = 255;
        stencil.compareMask = 255;
        vk_d.state.dsBlend.front = vk_d.state.dsBlend.back = stencil;
        
        vk_d.state.cullMode = VK_CULL_MODE_NONE;
        vk_d.state.clip = qfalse;
        
        VK_Bind( tr.whiteImage );
        
        tr_api.State( GLS_DEPTHMASK_TRUE | GLS_SRCBLEND_DST_COLOR | GLS_DSTBLEND_ZERO );

        tess.indexes[0] = 0;
        tess.indexes[1] = 1;
        tess.indexes[2] = 2;
        tess.indexes[3] = 0;
        tess.indexes[4] = 2;
        tess.indexes[5] = 3;
        tess.numIndexes = 6;
        
        VectorSet(tess.xyz[0], -100,  100, -10);
        VectorSet(tess.xyz[1],  100,  100, -10);
        VectorSet(tess.xyz[2],  100, -100, -10);
        VectorSet(tess.xyz[3], -100, -100, -10);
        tess.numVertexes = 4;
        
		for (int k = 0; k < tess.numVertexes; k++) {
			VectorSet(tess.svars.colors[k], 153, 153, 153);
			tess.svars.colors[k][3] = 255;
		}
        
        VK_UploadAttribDataOffset(&vk_d.vertexbuffer, vk_d.offset * sizeof(vec4_t), tess.numVertexes * sizeof(vec4_t), (void*)&tess.xyz[0]);
        VK_UploadAttribDataOffset(&vk_d.colorbuffer, vk_d.offset * sizeof(color4ub_t), tess.numVertexes * sizeof(color4ub_t), (void *) &tess.svars.colors[0]);
        VK_UploadAttribDataOffset(&vk_d.uvbuffer1, vk_d.offset * sizeof(vec2_t), tess.numVertexes * sizeof(vec2_t), (void *) &tess.svars.texcoords[0]);
        
        float tmp[16];
        Com_Memcpy(tmp, vk_d.modelViewMatrix, 64);
        Com_Memset(vk_d.modelViewMatrix, 0, 64);
        vk_d.modelViewMatrix[0] = 1.0f;
        vk_d.modelViewMatrix[5] = 1.0f;
        vk_d.modelViewMatrix[10] = 1.0f;
        vk_d.modelViewMatrix[15] = 1.0f;

        myGlMultMatrix(vk_d.modelViewMatrix, vk_d.projectionMatrix, vk_d.mvp);
        tr_api.R_DrawElements(tess.numIndexes, tess.indexes);
        
        vk_d.offset += tess.numVertexes;
        vk_d.offsetIdx += tess.numIndexes;
        
        vk_d.state.dsBlend.stencilTestEnable = VK_FALSE;
        tess.numIndexes = 0;
        tess.numVertexes = 0;
        Com_Memcpy(vk_d.modelViewMatrix, tmp, 64);
    }
}


/*
=================
RB_ProjectionShadowDeform

=================
*/
void RB_ProjectionShadowDeform( void ) {
	float	*xyz;
	int		i;
	float	h;
	vec3_t	ground;
	vec3_t	light;
	float	groundDist;
	float	d;
	vec3_t	lightDir;

	xyz = ( float * ) tess.xyz;

	ground[0] = backEnd.or.axis[0][2];
	ground[1] = backEnd.or.axis[1][2];
	ground[2] = backEnd.or.axis[2][2];

	groundDist = backEnd.or.origin[2] - backEnd.currentEntity->e.shadowPlane;

	VectorCopy( backEnd.currentEntity->lightDir, lightDir );
	d = DotProduct( lightDir, ground );
	// don't let the shadows get too long or go negative
	if ( d < 0.5 ) {
		VectorMA( lightDir, (0.5 - d), ground, lightDir );
		d = DotProduct( lightDir, ground );
	}
	d = 1.0 / d;

	light[0] = lightDir[0] * d;
	light[1] = lightDir[1] * d;
	light[2] = lightDir[2] * d;

	for ( i = 0; i < tess.numVertexes; i++, xyz += 4 ) {
		h = DotProduct( xyz, ground ) + groundDist;

		xyz[0] -= light[0] * h;
		xyz[1] -= light[1] * h;
		xyz[2] -= light[2] * h;
	}
}
