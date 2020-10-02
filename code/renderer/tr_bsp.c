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
// tr_map.c

#include "tr_local.h"

/*

Loads and prepares a map file for scene rendering.

A single entry point:

void RE_LoadWorldMap( const char *name );

*/

static	world_t		s_worldData;
static	byte		*fileBase;

int			c_subdivisions;
int			c_gridVerts;

//===============================================================================

static void HSVtoRGB( float h, float s, float v, float rgb[3] )
{
	int i;
	float f;
	float p, q, t;

	h *= 5;

	i = floor( h );
	f = h - i;

	p = v * ( 1 - s );
	q = v * ( 1 - s * f );
	t = v * ( 1 - s * ( 1 - f ) );

	switch ( i )
	{
	case 0:
		rgb[0] = v;
		rgb[1] = t;
		rgb[2] = p;
		break;
	case 1:
		rgb[0] = q;
		rgb[1] = v;
		rgb[2] = p;
		break;
	case 2:
		rgb[0] = p;
		rgb[1] = v;
		rgb[2] = t;
		break;
	case 3:
		rgb[0] = p;
		rgb[1] = q;
		rgb[2] = v;
		break;
	case 4:
		rgb[0] = t;
		rgb[1] = p;
		rgb[2] = v;
		break;
	case 5:
		rgb[0] = v;
		rgb[1] = p;
		rgb[2] = q;
		break;
	}
}

/*
===============
R_ColorShiftLightingBytes

===============
*/
static	void R_ColorShiftLightingBytes( byte in[4], byte out[4] ) {
	int		shift, r, g, b;

	// shift the color data based on overbright range
	shift = r_mapOverBrightBits->integer - tr.overbrightBits;

	// shift the data based on overbright range
	r = in[0] << shift;
	g = in[1] << shift;
	b = in[2] << shift;
	
	// normalize by color instead of saturating to white
	if ( ( r | g | b ) > 255 ) {
		int		max;

		max = r > g ? r : g;
		max = max > b ? max : b;
		r = r * 255 / max;
		g = g * 255 / max;
		b = b * 255 / max;
	}

	out[0] = r;
	out[1] = g;
	out[2] = b;
	out[3] = in[3];
}

/*
===============
R_LoadLightmaps

===============
*/
#define	LIGHTMAP_SIZE	128
static	void R_LoadLightmaps( lump_t *l ) {
	byte		*buf, *buf_p;
	int			len;
	MAC_STATIC byte		image[LIGHTMAP_SIZE*LIGHTMAP_SIZE*4];
	int			i, j;
	float maxIntensity = 0;
	double sumIntensity = 0;

    len = l->filelen;
	if ( !len ) {
		return;
	}
	buf = fileBase + l->fileofs;

	// we are about to upload textures
	R_SyncRenderThread();

	// create all the lightmaps
	tr.numLightmaps = len / (LIGHTMAP_SIZE * LIGHTMAP_SIZE * 3);
	if ( tr.numLightmaps == 1 ) {
		//FIXME: HACK: maps with only one lightmap turn up fullbright for some reason.
		//this avoids this, but isn't the correct solution.
		tr.numLightmaps++;
	}

	// if we are in r_vertexLight mode, we don't need the lightmaps at all
	if ( r_vertexLight->integer) {
		return;
	}
    
	for ( i = 0 ; i < tr.numLightmaps ; i++ ) {
		// expand the 24 bit on-disk to 32 bit
		buf_p = buf + i * LIGHTMAP_SIZE*LIGHTMAP_SIZE * 3;

		if ( r_lightmap->integer == 2 )
		{	// color code by intensity as development tool	(FIXME: check range)
			for ( j = 0; j < LIGHTMAP_SIZE * LIGHTMAP_SIZE; j++ )
			{
				float r = buf_p[j*3+0];
				float g = buf_p[j*3+1];
				float b = buf_p[j*3+2];
				float intensity;
				float out[3];

				intensity = 0.33f * r + 0.685f * g + 0.063f * b;

				if ( intensity > 255 )
					intensity = 1.0f;
				else
					intensity /= 255.0f;

				if ( intensity > maxIntensity )
					maxIntensity = intensity;

				HSVtoRGB( intensity, 1.00, 0.50, out );

				image[j*4+0] = out[0] * 255;
				image[j*4+1] = out[1] * 255;
				image[j*4+2] = out[2] * 255;
				image[j*4+3] = 255;

				sumIntensity += intensity;
			}
		} else {
			for ( j = 0 ; j < LIGHTMAP_SIZE * LIGHTMAP_SIZE; j++ ) {
				R_ColorShiftLightingBytes( &buf_p[j*3], &image[j*4] );
				image[j*4+3] = 255;
			}
		}
		tr.lightmaps[i] = R_CreateImage( va("*lightmap%d",i), image, 
			LIGHTMAP_SIZE, LIGHTMAP_SIZE, qfalse, qfalse, GL_CLAMP );
	}

	if ( r_lightmap->integer == 2 )	{
		ri.Printf( PRINT_ALL, "Brightest lightmap value: %d\n", ( int ) ( maxIntensity * 255 ) );
	}
}


/*
=================
RE_SetWorldVisData

This is called by the clipmodel subsystem so we can share the 1.8 megs of
space in big maps...
=================
*/
void		RE_SetWorldVisData( const byte *vis ) {
	tr.externalVisData = vis;
}


/*
=================
R_LoadVisibility
=================
*/
static	void R_LoadVisibility( lump_t *l ) {
	int		len;
	byte	*buf;

	len = ( s_worldData.numClusters + 63 ) & ~63;
	s_worldData.novis = ri.Hunk_Alloc( len, h_low );
	Com_Memset( s_worldData.novis, 0xff, len );

    len = l->filelen;
	if ( !len ) {
		return;
	}
	buf = fileBase + l->fileofs;

	s_worldData.numClusters = LittleLong( ((int *)buf)[0] );
	s_worldData.clusterBytes = LittleLong( ((int *)buf)[1] );

	// CM_Load should have given us the vis data to share, so
	// we don't need to allocate another copy
	if ( tr.externalVisData ) {
		s_worldData.vis = tr.externalVisData;
	} else {
		byte	*dest;

		dest = ri.Hunk_Alloc( len - 8, h_low );
		Com_Memcpy( dest, buf + 8, len - 8 );
		s_worldData.vis = dest;
	}
}

//===============================================================================


/*
===============
ShaderForShaderNum
===============
*/
static shader_t *ShaderForShaderNum( int shaderNum, int lightmapNum ) {
	shader_t	*shader;
	dshader_t	*dsh;

    int shaderNumLittleLong = LittleLong( shaderNum );
	shaderNum = shaderNumLittleLong;
	if ( shaderNum < 0 || shaderNum >= s_worldData.numShaders ) {
		ri.Error( ERR_DROP, "ShaderForShaderNum: bad num %i", shaderNum );
	}
	dsh = &s_worldData.shaders[ shaderNum ];

	if ( r_vertexLight->integer) {
		lightmapNum = LIGHTMAP_BY_VERTEX;
	}

	if ( r_fullbright->integer ) {
		lightmapNum = LIGHTMAP_WHITEIMAGE;
	}

	shader = R_FindShader( dsh->shader, lightmapNum, qtrue );

	// if the shader had errors, just use default shader
	if ( shader->defaultShader ) {
		return tr.defaultShader;
	}

	return shader;
}

/*
===============
ParseFace
===============
*/
static void ParseFace( dsurface_t *ds, drawVert_t *verts, msurface_t *surf, int *indexes  ) {
	int			i, j;
	srfSurfaceFace_t	*cv;
	int			numPoints, numIndexes;
	int			lightmapNum;
	int			sfaceSize, ofsIndexes;

	lightmapNum = LittleLong( ds->lightmapNum );

	// get fog volume
	surf->fogIndex = LittleLong( ds->fogNum ) + 1;

	// get shader value
	surf->shader = ShaderForShaderNum( ds->shaderNum, lightmapNum );
	if ( r_singleShader->integer && !surf->shader->isSky ) {
		surf->shader = tr.defaultShader;
	}

	numPoints = LittleLong( ds->numVerts );
	if (numPoints > MAX_FACE_POINTS) {
		ri.Printf( PRINT_WARNING, "WARNING: MAX_FACE_POINTS exceeded: %i\n", numPoints);
    numPoints = MAX_FACE_POINTS;
    surf->shader = tr.defaultShader;
	}

	numIndexes = LittleLong( ds->numIndexes );

	// create the srfSurfaceFace_t
	sfaceSize = ( int ) &((srfSurfaceFace_t *)0)->points[numPoints];
	ofsIndexes = sfaceSize;
	sfaceSize += sizeof( int ) * numIndexes;

	cv = ri.Hunk_Alloc( sfaceSize, h_low );
	cv->surfaceType = SF_FACE;
	cv->numPoints = numPoints;
	cv->numIndices = numIndexes;
	cv->ofsIndices = ofsIndexes;

	verts += LittleLong( ds->firstVert );
	for ( i = 0 ; i < numPoints ; i++ ) {
		for ( j = 0 ; j < 3 ; j++ ) {
			cv->points[i][j] = LittleFloat( verts[i].xyz[j] );
		}
		for ( j = 0 ; j < 2 ; j++ ) {
			cv->points[i][3+j] = LittleFloat( verts[i].st[j] );
			cv->points[i][5+j] = LittleFloat( verts[i].lightmap[j] );
		}
		R_ColorShiftLightingBytes( verts[i].color, (byte *)&cv->points[i][7] );
	}

	indexes += LittleLong( ds->firstIndex );
	for ( i = 0 ; i < numIndexes ; i++ ) {
		((int *)((byte *)cv + cv->ofsIndices ))[i] = LittleLong( indexes[ i ] );
	}

	// take the plane information from the lightmap vector
	for ( i = 0 ; i < 3 ; i++ ) {
		cv->plane.normal[i] = LittleFloat( ds->lightmapVecs[2][i] );
	}
	cv->plane.dist = DotProduct( cv->points[0], cv->plane.normal );
	SetPlaneSignbits( &cv->plane );
	cv->plane.type = PlaneTypeForNormal( cv->plane.normal );

	surf->data = (surfaceType_t *)cv;
}


/*
===============
ParseMesh
===============
*/
static void ParseMesh ( dsurface_t *ds, drawVert_t *verts, msurface_t *surf ) {
	srfGridMesh_t	*grid;
	int				i, j;
	int				width, height, numPoints;
	MAC_STATIC drawVert_t points[MAX_PATCH_SIZE*MAX_PATCH_SIZE];
	int				lightmapNum;
	vec3_t			bounds[2];
	vec3_t			tmpVec;
	static surfaceType_t	skipData = SF_SKIP;

	lightmapNum = LittleLong( ds->lightmapNum );

	// get fog volume
	surf->fogIndex = LittleLong( ds->fogNum ) + 1;

	// get shader value
	surf->shader = ShaderForShaderNum( ds->shaderNum, lightmapNum );
	if ( r_singleShader->integer && !surf->shader->isSky ) {
		surf->shader = tr.defaultShader;
	}

	// we may have a nodraw surface, because they might still need to
	// be around for movement clipping
	if ( s_worldData.shaders[ LittleLong( ds->shaderNum ) ].surfaceFlags & SURF_NODRAW ) {
		surf->data = &skipData;
		return;
	}

	width = LittleLong( ds->patchWidth );
	height = LittleLong( ds->patchHeight );

	verts += LittleLong( ds->firstVert );
	numPoints = width * height;
	for ( i = 0 ; i < numPoints ; i++ ) {
		for ( j = 0 ; j < 3 ; j++ ) {
			points[i].xyz[j] = LittleFloat( verts[i].xyz[j] );
			points[i].normal[j] = LittleFloat( verts[i].normal[j] );
		}
		for ( j = 0 ; j < 2 ; j++ ) {
			points[i].st[j] = LittleFloat( verts[i].st[j] );
			points[i].lightmap[j] = LittleFloat( verts[i].lightmap[j] );
		}
		R_ColorShiftLightingBytes( verts[i].color, points[i].color );
	}

	// pre-tesseleate
	grid = R_SubdividePatchToGrid( width, height, points );
	surf->data = (surfaceType_t *)grid;

	// copy the level of detail origin, which is the center
	// of the group of all curves that must subdivide the same
	// to avoid cracking
	for ( i = 0 ; i < 3 ; i++ ) {
		bounds[0][i] = LittleFloat( ds->lightmapVecs[0][i] );
		bounds[1][i] = LittleFloat( ds->lightmapVecs[1][i] );
	}
	VectorAdd( bounds[0], bounds[1], bounds[1] );
	VectorScale( bounds[1], 0.5f, grid->lodOrigin );
	VectorSubtract( bounds[0], grid->lodOrigin, tmpVec );
	grid->lodRadius = VectorLength( tmpVec );
}

/*
===============
ParseTriSurf
===============
*/
static void ParseTriSurf( dsurface_t *ds, drawVert_t *verts, msurface_t *surf, int *indexes ) {
	srfTriangles_t	*tri;
	int				i, j;
	int				numVerts, numIndexes;

	// get fog volume
	surf->fogIndex = LittleLong( ds->fogNum ) + 1;

	// get shader
	surf->shader = ShaderForShaderNum( ds->shaderNum, LIGHTMAP_BY_VERTEX );
	if ( r_singleShader->integer && !surf->shader->isSky ) {
		surf->shader = tr.defaultShader;
	}

	numVerts = LittleLong( ds->numVerts );
	numIndexes = LittleLong( ds->numIndexes );

	tri = ri.Hunk_Alloc( sizeof( *tri ) + numVerts * sizeof( tri->verts[0] ) 
		+ numIndexes * sizeof( tri->indexes[0] ), h_low );
	tri->surfaceType = SF_TRIANGLES;
	tri->numVerts = numVerts;
	tri->numIndexes = numIndexes;
	tri->verts = (drawVert_t *)(tri + 1);
	tri->indexes = (int *)(tri->verts + tri->numVerts );

	surf->data = (surfaceType_t *)tri;

	// copy vertexes
	ClearBounds( tri->bounds[0], tri->bounds[1] );
	verts += LittleLong( ds->firstVert );
	for ( i = 0 ; i < numVerts ; i++ ) {
		for ( j = 0 ; j < 3 ; j++ ) {
			tri->verts[i].xyz[j] = LittleFloat( verts[i].xyz[j] );
			tri->verts[i].normal[j] = LittleFloat( verts[i].normal[j] );
		}
		AddPointToBounds( tri->verts[i].xyz, tri->bounds[0], tri->bounds[1] );
		for ( j = 0 ; j < 2 ; j++ ) {
			tri->verts[i].st[j] = LittleFloat( verts[i].st[j] );
			tri->verts[i].lightmap[j] = LittleFloat( verts[i].lightmap[j] );
		}

		R_ColorShiftLightingBytes( verts[i].color, tri->verts[i].color );
	}

	// copy indexes
	indexes += LittleLong( ds->firstIndex );
	for ( i = 0 ; i < numIndexes ; i++ ) {
		tri->indexes[i] = LittleLong( indexes[i] );
		if ( tri->indexes[i] < 0 || tri->indexes[i] >= numVerts ) {
			ri.Error( ERR_DROP, "Bad index in triangle surface" );
		}
	}
}

/*
===============
ParseFlare
===============
*/
static void ParseFlare( dsurface_t *ds, drawVert_t *verts, msurface_t *surf, int *indexes ) {
	srfFlare_t		*flare;
	int				i;

	// get fog volume
	surf->fogIndex = LittleLong( ds->fogNum ) + 1;

	// get shader
	surf->shader = ShaderForShaderNum( ds->shaderNum, LIGHTMAP_BY_VERTEX );
	if ( r_singleShader->integer && !surf->shader->isSky ) {
		surf->shader = tr.defaultShader;
	}

	flare = ri.Hunk_Alloc( sizeof( *flare ), h_low );
	flare->surfaceType = SF_FLARE;

	surf->data = (surfaceType_t *)flare;

	for ( i = 0 ; i < 3 ; i++ ) {
		flare->origin[i] = LittleFloat( ds->lightmapOrigin[i] );
		flare->color[i] = LittleFloat( ds->lightmapVecs[0][i] );
		flare->normal[i] = LittleFloat( ds->lightmapVecs[2][i] );
	}
}


/*
=================
R_MergedWidthPoints

returns true if there are grid points merged on a width edge
=================
*/
int R_MergedWidthPoints(srfGridMesh_t *grid, int offset) {
	int i, j;

	for (i = 1; i < grid->width-1; i++) {
		for (j = i + 1; j < grid->width-1; j++) {
			if ( fabs(grid->verts[i + offset].xyz[0] - grid->verts[j + offset].xyz[0]) > .1) continue;
			if ( fabs(grid->verts[i + offset].xyz[1] - grid->verts[j + offset].xyz[1]) > .1) continue;
			if ( fabs(grid->verts[i + offset].xyz[2] - grid->verts[j + offset].xyz[2]) > .1) continue;
			return qtrue;
		}
	}
	return qfalse;
}

/*
=================
R_MergedHeightPoints

returns true if there are grid points merged on a height edge
=================
*/
int R_MergedHeightPoints(srfGridMesh_t *grid, int offset) {
	int i, j;

	for (i = 1; i < grid->height-1; i++) {
		for (j = i + 1; j < grid->height-1; j++) {
			if ( fabs(grid->verts[grid->width * i + offset].xyz[0] - grid->verts[grid->width * j + offset].xyz[0]) > .1) continue;
			if ( fabs(grid->verts[grid->width * i + offset].xyz[1] - grid->verts[grid->width * j + offset].xyz[1]) > .1) continue;
			if ( fabs(grid->verts[grid->width * i + offset].xyz[2] - grid->verts[grid->width * j + offset].xyz[2]) > .1) continue;
			return qtrue;
		}
	}
	return qfalse;
}

/*
=================
R_FixSharedVertexLodError_r

NOTE: never sync LoD through grid edges with merged points!

FIXME: write generalized version that also avoids cracks between a patch and one that meets half way?
=================
*/
void R_FixSharedVertexLodError_r( int start, srfGridMesh_t *grid1 ) {
	int j, k, l, m, n, offset1, offset2, touch;
	srfGridMesh_t *grid2;

	for ( j = start; j < s_worldData.numsurfaces; j++ ) {
		//
		grid2 = (srfGridMesh_t *) s_worldData.surfaces[j].data;
		// if this surface is not a grid
		if ( grid2->surfaceType != SF_GRID ) continue;
		// if the LOD errors are already fixed for this patch
		if ( grid2->lodFixed == 2 ) continue;
		// grids in the same LOD group should have the exact same lod radius
		if ( grid1->lodRadius != grid2->lodRadius ) continue;
		// grids in the same LOD group should have the exact same lod origin
		if ( grid1->lodOrigin[0] != grid2->lodOrigin[0] ) continue;
		if ( grid1->lodOrigin[1] != grid2->lodOrigin[1] ) continue;
		if ( grid1->lodOrigin[2] != grid2->lodOrigin[2] ) continue;
		//
		touch = qfalse;
		for (n = 0; n < 2; n++) {
			//
			if (n) offset1 = (grid1->height-1) * grid1->width;
			else offset1 = 0;
			if (R_MergedWidthPoints(grid1, offset1)) continue;
			for (k = 1; k < grid1->width-1; k++) {
				for (m = 0; m < 2; m++) {

					if (m) offset2 = (grid2->height-1) * grid2->width;
					else offset2 = 0;
					if (R_MergedWidthPoints(grid2, offset2)) continue;
					for ( l = 1; l < grid2->width-1; l++) {
					//
						if ( fabs(grid1->verts[k + offset1].xyz[0] - grid2->verts[l + offset2].xyz[0]) > .1) continue;
						if ( fabs(grid1->verts[k + offset1].xyz[1] - grid2->verts[l + offset2].xyz[1]) > .1) continue;
						if ( fabs(grid1->verts[k + offset1].xyz[2] - grid2->verts[l + offset2].xyz[2]) > .1) continue;
						// ok the points are equal and should have the same lod error
						grid2->widthLodError[l] = grid1->widthLodError[k];
						touch = qtrue;
					}
				}
				for (m = 0; m < 2; m++) {

					if (m) offset2 = grid2->width-1;
					else offset2 = 0;
					if (R_MergedHeightPoints(grid2, offset2)) continue;
					for ( l = 1; l < grid2->height-1; l++) {
					//
						if ( fabs(grid1->verts[k + offset1].xyz[0] - grid2->verts[grid2->width * l + offset2].xyz[0]) > .1) continue;
						if ( fabs(grid1->verts[k + offset1].xyz[1] - grid2->verts[grid2->width * l + offset2].xyz[1]) > .1) continue;
						if ( fabs(grid1->verts[k + offset1].xyz[2] - grid2->verts[grid2->width * l + offset2].xyz[2]) > .1) continue;
						// ok the points are equal and should have the same lod error
						grid2->heightLodError[l] = grid1->widthLodError[k];
						touch = qtrue;
					}
				}
			}
		}
		for (n = 0; n < 2; n++) {
			//
			if (n) offset1 = grid1->width-1;
			else offset1 = 0;
			if (R_MergedHeightPoints(grid1, offset1)) continue;
			for (k = 1; k < grid1->height-1; k++) {
				for (m = 0; m < 2; m++) {

					if (m) offset2 = (grid2->height-1) * grid2->width;
					else offset2 = 0;
					if (R_MergedWidthPoints(grid2, offset2)) continue;
					for ( l = 1; l < grid2->width-1; l++) {
					//
						if ( fabs(grid1->verts[grid1->width * k + offset1].xyz[0] - grid2->verts[l + offset2].xyz[0]) > .1) continue;
						if ( fabs(grid1->verts[grid1->width * k + offset1].xyz[1] - grid2->verts[l + offset2].xyz[1]) > .1) continue;
						if ( fabs(grid1->verts[grid1->width * k + offset1].xyz[2] - grid2->verts[l + offset2].xyz[2]) > .1) continue;
						// ok the points are equal and should have the same lod error
						grid2->widthLodError[l] = grid1->heightLodError[k];
						touch = qtrue;
					}
				}
				for (m = 0; m < 2; m++) {

					if (m) offset2 = grid2->width-1;
					else offset2 = 0;
					if (R_MergedHeightPoints(grid2, offset2)) continue;
					for ( l = 1; l < grid2->height-1; l++) {
					//
						if ( fabs(grid1->verts[grid1->width * k + offset1].xyz[0] - grid2->verts[grid2->width * l + offset2].xyz[0]) > .1) continue;
						if ( fabs(grid1->verts[grid1->width * k + offset1].xyz[1] - grid2->verts[grid2->width * l + offset2].xyz[1]) > .1) continue;
						if ( fabs(grid1->verts[grid1->width * k + offset1].xyz[2] - grid2->verts[grid2->width * l + offset2].xyz[2]) > .1) continue;
						// ok the points are equal and should have the same lod error
						grid2->heightLodError[l] = grid1->heightLodError[k];
						touch = qtrue;
					}
				}
			}
		}
		if (touch) {
			grid2->lodFixed = 2;
			R_FixSharedVertexLodError_r ( start, grid2 );
			//NOTE: this would be correct but makes things really slow
			//grid2->lodFixed = 1;
		}
	}
}

/*
=================
R_FixSharedVertexLodError

This function assumes that all patches in one group are nicely stitched together for the highest LoD.
If this is not the case this function will still do its job but won't fix the highest LoD cracks.
=================
*/
void R_FixSharedVertexLodError( void ) {
	int i;
	srfGridMesh_t *grid1;

	for ( i = 0; i < s_worldData.numsurfaces; i++ ) {
		//
		grid1 = (srfGridMesh_t *) s_worldData.surfaces[i].data;
		// if this surface is not a grid
		if ( grid1->surfaceType != SF_GRID )
			continue;
		//
		if ( grid1->lodFixed )
			continue;
		//
		grid1->lodFixed = 2;
		// recursively fix other patches in the same LOD group
		R_FixSharedVertexLodError_r( i + 1, grid1);
	}
}


/*
===============
R_StitchPatches
===============
*/
int R_StitchPatches( int grid1num, int grid2num ) {
	float *v1, *v2;
	srfGridMesh_t *grid1, *grid2;
	int k, l, m, n, offset1, offset2, row, column;

	grid1 = (srfGridMesh_t *) s_worldData.surfaces[grid1num].data;
	grid2 = (srfGridMesh_t *) s_worldData.surfaces[grid2num].data;
	for (n = 0; n < 2; n++) {
		//
		if (n) offset1 = (grid1->height-1) * grid1->width;
		else offset1 = 0;
		if (R_MergedWidthPoints(grid1, offset1))
			continue;
		for (k = 0; k < grid1->width-2; k += 2) {

			for (m = 0; m < 2; m++) {

				if ( grid2->width >= MAX_GRID_SIZE )
					break;
				if (m) offset2 = (grid2->height-1) * grid2->width;
				else offset2 = 0;
				for ( l = 0; l < grid2->width-1; l++) {
				//
					v1 = grid1->verts[k + offset1].xyz;
					v2 = grid2->verts[l + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;

					v1 = grid1->verts[k + 2 + offset1].xyz;
					v2 = grid2->verts[l + 1 + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;
					//
					v1 = grid2->verts[l + offset2].xyz;
					v2 = grid2->verts[l + 1 + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) < .01 &&
							fabs(v1[1] - v2[1]) < .01 &&
							fabs(v1[2] - v2[2]) < .01)
						continue;
					//
					//ri.Printf( PRINT_ALL, "found highest LoD crack between two patches\n" );
					// insert column into grid2 right after after column l
					if (m) row = grid2->height-1;
					else row = 0;
					grid2 = R_GridInsertColumn( grid2, l+1, row,
									grid1->verts[k + 1 + offset1].xyz, grid1->widthLodError[k+1]);
					grid2->lodStitched = qfalse;
					s_worldData.surfaces[grid2num].data = (void *) grid2;
					return qtrue;
				}
			}
			for (m = 0; m < 2; m++) {

				if (grid2->height >= MAX_GRID_SIZE)
					break;
				if (m) offset2 = grid2->width-1;
				else offset2 = 0;
				for ( l = 0; l < grid2->height-1; l++) {
					//
					v1 = grid1->verts[k + offset1].xyz;
					v2 = grid2->verts[grid2->width * l + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;

					v1 = grid1->verts[k + 2 + offset1].xyz;
					v2 = grid2->verts[grid2->width * (l + 1) + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;
					//
					v1 = grid2->verts[grid2->width * l + offset2].xyz;
					v2 = grid2->verts[grid2->width * (l + 1) + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) < .01 &&
							fabs(v1[1] - v2[1]) < .01 &&
							fabs(v1[2] - v2[2]) < .01)
						continue;
					//
					//ri.Printf( PRINT_ALL, "found highest LoD crack between two patches\n" );
					// insert row into grid2 right after after row l
					if (m) column = grid2->width-1;
					else column = 0;
					grid2 = R_GridInsertRow( grid2, l+1, column,
										grid1->verts[k + 1 + offset1].xyz, grid1->widthLodError[k+1]);
					grid2->lodStitched = qfalse;
					s_worldData.surfaces[grid2num].data = (void *) grid2;
					return qtrue;
				}
			}
		}
	}
	for (n = 0; n < 2; n++) {
		//
		if (n) offset1 = grid1->width-1;
		else offset1 = 0;
		if (R_MergedHeightPoints(grid1, offset1))
			continue;
		for (k = 0; k < grid1->height-2; k += 2) {
			for (m = 0; m < 2; m++) {

				if ( grid2->width >= MAX_GRID_SIZE )
					break;
				if (m) offset2 = (grid2->height-1) * grid2->width;
				else offset2 = 0;
				for ( l = 0; l < grid2->width-1; l++) {
				//
					v1 = grid1->verts[grid1->width * k + offset1].xyz;
					v2 = grid2->verts[l + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;

					v1 = grid1->verts[grid1->width * (k + 2) + offset1].xyz;
					v2 = grid2->verts[l + 1 + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;
					//
					v1 = grid2->verts[l + offset2].xyz;
					v2 = grid2->verts[(l + 1) + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) < .01 &&
							fabs(v1[1] - v2[1]) < .01 &&
							fabs(v1[2] - v2[2]) < .01)
						continue;
					//
					//ri.Printf( PRINT_ALL, "found highest LoD crack between two patches\n" );
					// insert column into grid2 right after after column l
					if (m) row = grid2->height-1;
					else row = 0;
					grid2 = R_GridInsertColumn( grid2, l+1, row,
									grid1->verts[grid1->width * (k + 1) + offset1].xyz, grid1->heightLodError[k+1]);
					grid2->lodStitched = qfalse;
					s_worldData.surfaces[grid2num].data = (void *) grid2;
					return qtrue;
				}
			}
			for (m = 0; m < 2; m++) {

				if (grid2->height >= MAX_GRID_SIZE)
					break;
				if (m) offset2 = grid2->width-1;
				else offset2 = 0;
				for ( l = 0; l < grid2->height-1; l++) {
				//
					v1 = grid1->verts[grid1->width * k + offset1].xyz;
					v2 = grid2->verts[grid2->width * l + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;

					v1 = grid1->verts[grid1->width * (k + 2) + offset1].xyz;
					v2 = grid2->verts[grid2->width * (l + 1) + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;
					//
					v1 = grid2->verts[grid2->width * l + offset2].xyz;
					v2 = grid2->verts[grid2->width * (l + 1) + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) < .01 &&
							fabs(v1[1] - v2[1]) < .01 &&
							fabs(v1[2] - v2[2]) < .01)
						continue;
					//
					//ri.Printf( PRINT_ALL, "found highest LoD crack between two patches\n" );
					// insert row into grid2 right after after row l
					if (m) column = grid2->width-1;
					else column = 0;
					grid2 = R_GridInsertRow( grid2, l+1, column,
									grid1->verts[grid1->width * (k + 1) + offset1].xyz, grid1->heightLodError[k+1]);
					grid2->lodStitched = qfalse;
					s_worldData.surfaces[grid2num].data = (void *) grid2;
					return qtrue;
				}
			}
		}
	}
	for (n = 0; n < 2; n++) {
		//
		if (n) offset1 = (grid1->height-1) * grid1->width;
		else offset1 = 0;
		if (R_MergedWidthPoints(grid1, offset1))
			continue;
		for (k = grid1->width-1; k > 1; k -= 2) {

			for (m = 0; m < 2; m++) {

				if ( grid2->width >= MAX_GRID_SIZE )
					break;
				if (m) offset2 = (grid2->height-1) * grid2->width;
				else offset2 = 0;
				for ( l = 0; l < grid2->width-1; l++) {
				//
					v1 = grid1->verts[k + offset1].xyz;
					v2 = grid2->verts[l + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;

					v1 = grid1->verts[k - 2 + offset1].xyz;
					v2 = grid2->verts[l + 1 + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;
					//
					v1 = grid2->verts[l + offset2].xyz;
					v2 = grid2->verts[(l + 1) + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) < .01 &&
							fabs(v1[1] - v2[1]) < .01 &&
							fabs(v1[2] - v2[2]) < .01)
						continue;
					//
					//ri.Printf( PRINT_ALL, "found highest LoD crack between two patches\n" );
					// insert column into grid2 right after after column l
					if (m) row = grid2->height-1;
					else row = 0;
					grid2 = R_GridInsertColumn( grid2, l+1, row,
										grid1->verts[k - 1 + offset1].xyz, grid1->widthLodError[k+1]);
					grid2->lodStitched = qfalse;
					s_worldData.surfaces[grid2num].data = (void *) grid2;
					return qtrue;
				}
			}
			for (m = 0; m < 2; m++) {

				if (grid2->height >= MAX_GRID_SIZE)
					break;
				if (m) offset2 = grid2->width-1;
				else offset2 = 0;
				for ( l = 0; l < grid2->height-1; l++) {
				//
					v1 = grid1->verts[k + offset1].xyz;
					v2 = grid2->verts[grid2->width * l + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;

					v1 = grid1->verts[k - 2 + offset1].xyz;
					v2 = grid2->verts[grid2->width * (l + 1) + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;
					//
					v1 = grid2->verts[grid2->width * l + offset2].xyz;
					v2 = grid2->verts[grid2->width * (l + 1) + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) < .01 &&
							fabs(v1[1] - v2[1]) < .01 &&
							fabs(v1[2] - v2[2]) < .01)
						continue;
					//
					//ri.Printf( PRINT_ALL, "found highest LoD crack between two patches\n" );
					// insert row into grid2 right after after row l
					if (m) column = grid2->width-1;
					else column = 0;
					grid2 = R_GridInsertRow( grid2, l+1, column,
										grid1->verts[k - 1 + offset1].xyz, grid1->widthLodError[k+1]);
					if (!grid2)
						break;
					grid2->lodStitched = qfalse;
					s_worldData.surfaces[grid2num].data = (void *) grid2;
					return qtrue;
				}
			}
		}
	}
	for (n = 0; n < 2; n++) {
		//
		if (n) offset1 = grid1->width-1;
		else offset1 = 0;
		if (R_MergedHeightPoints(grid1, offset1))
			continue;
		for (k = grid1->height-1; k > 1; k -= 2) {
			for (m = 0; m < 2; m++) {

				if ( grid2->width >= MAX_GRID_SIZE )
					break;
				if (m) offset2 = (grid2->height-1) * grid2->width;
				else offset2 = 0;
				for ( l = 0; l < grid2->width-1; l++) {
				//
					v1 = grid1->verts[grid1->width * k + offset1].xyz;
					v2 = grid2->verts[l + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;

					v1 = grid1->verts[grid1->width * (k - 2) + offset1].xyz;
					v2 = grid2->verts[l + 1 + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;
					//
					v1 = grid2->verts[l + offset2].xyz;
					v2 = grid2->verts[(l + 1) + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) < .01 &&
							fabs(v1[1] - v2[1]) < .01 &&
							fabs(v1[2] - v2[2]) < .01)
						continue;
					//
					//ri.Printf( PRINT_ALL, "found highest LoD crack between two patches\n" );
					// insert column into grid2 right after after column l
					if (m) row = grid2->height-1;
					else row = 0;
					grid2 = R_GridInsertColumn( grid2, l+1, row,
										grid1->verts[grid1->width * (k - 1) + offset1].xyz, grid1->heightLodError[k+1]);
					grid2->lodStitched = qfalse;
					s_worldData.surfaces[grid2num].data = (void *) grid2;
					return qtrue;
				}
			}
			for (m = 0; m < 2; m++) {

				if (grid2->height >= MAX_GRID_SIZE)
					break;
				if (m) offset2 = grid2->width-1;
				else offset2 = 0;
				for ( l = 0; l < grid2->height-1; l++) {
				//
					v1 = grid1->verts[grid1->width * k + offset1].xyz;
					v2 = grid2->verts[grid2->width * l + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;

					v1 = grid1->verts[grid1->width * (k - 2) + offset1].xyz;
					v2 = grid2->verts[grid2->width * (l + 1) + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) > .1)
						continue;
					if ( fabs(v1[1] - v2[1]) > .1)
						continue;
					if ( fabs(v1[2] - v2[2]) > .1)
						continue;
					//
					v1 = grid2->verts[grid2->width * l + offset2].xyz;
					v2 = grid2->verts[grid2->width * (l + 1) + offset2].xyz;
					if ( fabs(v1[0] - v2[0]) < .01 &&
							fabs(v1[1] - v2[1]) < .01 &&
							fabs(v1[2] - v2[2]) < .01)
						continue;
					//
					//ri.Printf( PRINT_ALL, "found highest LoD crack between two patches\n" );
					// insert row into grid2 right after after row l
					if (m) column = grid2->width-1;
					else column = 0;
					grid2 = R_GridInsertRow( grid2, l+1, column,
										grid1->verts[grid1->width * (k - 1) + offset1].xyz, grid1->heightLodError[k+1]);
					grid2->lodStitched = qfalse;
					s_worldData.surfaces[grid2num].data = (void *) grid2;
					return qtrue;
				}
			}
		}
	}
	return qfalse;
}

/*
===============
R_TryStitchPatch

This function will try to stitch patches in the same LoD group together for the highest LoD.

Only single missing vertice cracks will be fixed.

Vertices will be joined at the patch side a crack is first found, at the other side
of the patch (on the same row or column) the vertices will not be joined and cracks
might still appear at that side.
===============
*/
int R_TryStitchingPatch( int grid1num ) {
	int j, numstitches;
	srfGridMesh_t *grid1, *grid2;

	numstitches = 0;
	grid1 = (srfGridMesh_t *) s_worldData.surfaces[grid1num].data;
	for ( j = 0; j < s_worldData.numsurfaces; j++ ) {
		//
		grid2 = (srfGridMesh_t *) s_worldData.surfaces[j].data;
		// if this surface is not a grid
		if ( grid2->surfaceType != SF_GRID ) continue;
		// grids in the same LOD group should have the exact same lod radius
		if ( grid1->lodRadius != grid2->lodRadius ) continue;
		// grids in the same LOD group should have the exact same lod origin
		if ( grid1->lodOrigin[0] != grid2->lodOrigin[0] ) continue;
		if ( grid1->lodOrigin[1] != grid2->lodOrigin[1] ) continue;
		if ( grid1->lodOrigin[2] != grid2->lodOrigin[2] ) continue;
		//
		while (R_StitchPatches(grid1num, j))
		{
			numstitches++;
		}
	}
	return numstitches;
}

/*
===============
R_StitchAllPatches
===============
*/
void R_StitchAllPatches( void ) {
	int i, stitched, numstitches;
	srfGridMesh_t *grid1;

	numstitches = 0;
	do
	{
		stitched = qfalse;
		for ( i = 0; i < s_worldData.numsurfaces; i++ ) {
			//
			grid1 = (srfGridMesh_t *) s_worldData.surfaces[i].data;
			// if this surface is not a grid
			if ( grid1->surfaceType != SF_GRID )
				continue;
			//
			if ( grid1->lodStitched )
				continue;
			//
			grid1->lodStitched = qtrue;
			stitched = qtrue;
			//
			numstitches += R_TryStitchingPatch( i );
		}
	}
	while (stitched);
	ri.Printf( PRINT_ALL, "stitched %d LoD cracks\n", numstitches );
}

/*
===============
R_MovePatchSurfacesToHunk
===============
*/
void R_MovePatchSurfacesToHunk(void) {
	int i, size;
	srfGridMesh_t *grid, *hunkgrid;

	for ( i = 0; i < s_worldData.numsurfaces; i++ ) {
		//
		grid = (srfGridMesh_t *) s_worldData.surfaces[i].data;
		// if this surface is not a grid
		if ( grid->surfaceType != SF_GRID )
			continue;
		//
		size = (grid->width * grid->height - 1) * sizeof( drawVert_t ) + sizeof( *grid );
		hunkgrid = ri.Hunk_Alloc( size, h_low );
		Com_Memcpy(hunkgrid, grid, size);

		hunkgrid->widthLodError = ri.Hunk_Alloc( grid->width * 4, h_low );
		Com_Memcpy( hunkgrid->widthLodError, grid->widthLodError, grid->width * 4 );

		hunkgrid->heightLodError = ri.Hunk_Alloc( grid->height * 4, h_low );
		Com_Memcpy( grid->heightLodError, grid->heightLodError, grid->height * 4 );

		R_FreeSurfaceGridMesh( grid );

		s_worldData.surfaces[i].data = (void *) hunkgrid;
	}
}

/*
===============
R_LoadSurfaces
===============
*/
static	void R_LoadSurfaces( lump_t *surfs, lump_t *verts, lump_t *indexLump ) {
	dsurface_t	*in;
	msurface_t	*out;
	drawVert_t	*dv;
	int			*indexes;
	int			count;
	int			numFaces, numMeshes, numTriSurfs, numFlares;
	int			i;

	numFaces = 0;
	numMeshes = 0;
	numTriSurfs = 0;
	numFlares = 0;

	in = (void *)(fileBase + surfs->fileofs);
	if (surfs->filelen % sizeof(*in))
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name);
	count = surfs->filelen / sizeof(*in);

	dv = (void *)(fileBase + verts->fileofs);
	if (verts->filelen % sizeof(*dv))
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name);

	indexes = (void *)(fileBase + indexLump->fileofs);
	if ( indexLump->filelen % sizeof(*indexes))
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name);

	out = ri.Hunk_Alloc ( count * sizeof(*out), h_low );	

	s_worldData.surfaces = out;
	s_worldData.numsurfaces = count;

	for ( i = 0 ; i < count ; i++, in++, out++ ) {
		switch ( LittleLong( in->surfaceType ) ) {
		case MST_PATCH:
			ParseMesh ( in, dv, out );
			numMeshes++;
			break;
		case MST_TRIANGLE_SOUP:
			ParseTriSurf( in, dv, out, indexes );
			numTriSurfs++;
			break;
		case MST_PLANAR:
			ParseFace( in, dv, out, indexes );
			numFaces++;
			break;
		case MST_FLARE:
			ParseFlare( in, dv, out, indexes );
			numFlares++;
			break;
		default:
			ri.Error( ERR_DROP, "Bad surfaceType" );
		}
	}

#ifdef PATCH_STITCHING
	R_StitchAllPatches();
#endif

	R_FixSharedVertexLodError();

#ifdef PATCH_STITCHING
	R_MovePatchSurfacesToHunk();
#endif

	ri.Printf( PRINT_ALL, "...loaded %d faces, %i meshes, %i trisurfs, %i flares\n", 
		numFaces, numMeshes, numTriSurfs, numFlares );
}



/*
=================
R_LoadSubmodels
=================
*/
static	void R_LoadSubmodels( lump_t *l ) {
	dmodel_t	*in;
	bmodel_t	*out;
	int			i, j, count;

	in = (void *)(fileBase + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name);
	count = l->filelen / sizeof(*in);

	s_worldData.bmodels = out = ri.Hunk_Alloc( count * sizeof(*out), h_low );

	for ( i=0 ; i<count ; i++, in++, out++ ) {
		model_t *model;

		model = R_AllocModel();

		assert( model != NULL );			// this should never happen

		model->type = MOD_BRUSH;
		model->bmodel = out;
		Com_sprintf( model->name, sizeof( model->name ), "*%d", i );

		for (j=0 ; j<3 ; j++) {
			out->bounds[0][j] = LittleFloat (in->mins[j]);
			out->bounds[1][j] = LittleFloat (in->maxs[j]);
		}

		out->firstSurface = s_worldData.surfaces + LittleLong( in->firstSurface );
		out->numSurfaces = LittleLong( in->numSurfaces );
	}
}



//==================================================================

/*
=================
R_SetParent
=================
*/
static	void R_SetParent (mnode_t *node, mnode_t *parent)
{
	node->parent = parent;
	if (node->contents != -1)
		return;
	R_SetParent (node->children[0], node);
	R_SetParent (node->children[1], node);
}

/*
=================
R_LoadNodesAndLeafs
=================
*/
static	void R_LoadNodesAndLeafs (lump_t *nodeLump, lump_t *leafLump) {
	int			i, j, p;
	dnode_t		*in;
	dleaf_t		*inLeaf;
	mnode_t 	*out;
	int			numNodes, numLeafs;

	in = (void *)(fileBase + nodeLump->fileofs);
	if (nodeLump->filelen % sizeof(dnode_t) ||
		leafLump->filelen % sizeof(dleaf_t) ) {
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name);
	}
	numNodes = nodeLump->filelen / sizeof(dnode_t);
	numLeafs = leafLump->filelen / sizeof(dleaf_t);

	out = ri.Hunk_Alloc ( (numNodes + numLeafs) * sizeof(*out), h_low);	

	s_worldData.nodes = out;
	s_worldData.numnodes = numNodes + numLeafs;
	s_worldData.numDecisionNodes = numNodes;

	// load nodes
	for ( i=0 ; i<numNodes; i++, in++, out++)
	{
		for (j=0 ; j<3 ; j++)
		{
			out->mins[j] = LittleLong (in->mins[j]);
			out->maxs[j] = LittleLong (in->maxs[j]);
		}
	
		p = LittleLong(in->planeNum);
		out->plane = s_worldData.planes + p;

		out->contents = CONTENTS_NODE;	// differentiate from leafs

		for (j=0 ; j<2 ; j++)
		{
			p = LittleLong (in->children[j]);
			if (p >= 0)
				out->children[j] = s_worldData.nodes + p;
			else
				out->children[j] = s_worldData.nodes + numNodes + (-1 - p);
		}
	}
	
	// load leafs
	inLeaf = (void *)(fileBase + leafLump->fileofs);
	for ( i=0 ; i<numLeafs ; i++, inLeaf++, out++)
	{
		for (j=0 ; j<3 ; j++)
		{
			out->mins[j] = LittleLong (inLeaf->mins[j]);
			out->maxs[j] = LittleLong (inLeaf->maxs[j]);
		}

		out->cluster = LittleLong(inLeaf->cluster);
		out->area = LittleLong(inLeaf->area);

		if ( out->cluster >= s_worldData.numClusters ) {
			s_worldData.numClusters = out->cluster + 1;
		}

		out->firstmarksurface = s_worldData.marksurfaces +
			LittleLong(inLeaf->firstLeafSurface);
		out->nummarksurfaces = LittleLong(inLeaf->numLeafSurfaces);
	}	

	// chain decendants
	R_SetParent (s_worldData.nodes, NULL);
}

//=============================================================================

/*
=================
R_LoadShaders
=================
*/
static	void R_LoadShaders( lump_t *l ) {	
	int		i, count;
	dshader_t	*in, *out;
	
	in = (void *)(fileBase + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name);
	count = l->filelen / sizeof(*in);
	out = ri.Hunk_Alloc ( count*sizeof(*out), h_low );

	s_worldData.shaders = out;
	s_worldData.numShaders = count;

	Com_Memcpy( out, in, count*sizeof(*out) );

	for ( i=0 ; i<count ; i++ ) {
		out[i].surfaceFlags = LittleLong( out[i].surfaceFlags );
		out[i].contentFlags = LittleLong( out[i].contentFlags );
	}
}


/*
=================
R_LoadMarksurfaces
=================
*/
static	void R_LoadMarksurfaces (lump_t *l)
{	
	int		i, j, count;
	int		*in;
	msurface_t **out;
	
	in = (void *)(fileBase + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name);
	count = l->filelen / sizeof(*in);
	out = ri.Hunk_Alloc ( count*sizeof(*out), h_low);	

	s_worldData.marksurfaces = out;
	s_worldData.nummarksurfaces = count;

	for ( i=0 ; i<count ; i++)
	{
		j = LittleLong(in[i]);
		out[i] = s_worldData.surfaces + j;
	}
}


/*
=================
R_LoadPlanes
=================
*/
static	void R_LoadPlanes( lump_t *l ) {
	int			i, j;
	cplane_t	*out;
	dplane_t 	*in;
	int			count;
	int			bits;
	
	in = (void *)(fileBase + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name);
	count = l->filelen / sizeof(*in);
	out = ri.Hunk_Alloc ( count*2*sizeof(*out), h_low);	
	
	s_worldData.planes = out;
	s_worldData.numplanes = count;

	for ( i=0 ; i<count ; i++, in++, out++) {
		bits = 0;
		for (j=0 ; j<3 ; j++) {
			out->normal[j] = LittleFloat (in->normal[j]);
			if (out->normal[j] < 0) {
				bits |= 1<<j;
			}
		}

		out->dist = LittleFloat (in->dist);
		out->type = PlaneTypeForNormal( out->normal );
		out->signbits = bits;
	}
}

/*
=================
R_LoadFogs

=================
*/
static	void R_LoadFogs( lump_t *l, lump_t *brushesLump, lump_t *sidesLump ) {
	int			i;
	fog_t		*out;
	dfog_t		*fogs;
	dbrush_t 	*brushes, *brush;
	dbrushside_t	*sides;
	int			count, brushesCount, sidesCount;
	int			sideNum;
	int			planeNum;
	shader_t	*shader;
	float		d;
	int			firstSide;

	fogs = (void *)(fileBase + l->fileofs);
	if (l->filelen % sizeof(*fogs)) {
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name);
	}
	count = l->filelen / sizeof(*fogs);

	// create fog strucutres for them
	s_worldData.numfogs = count + 1;
	s_worldData.fogs = ri.Hunk_Alloc ( s_worldData.numfogs*sizeof(*out), h_low);
	out = s_worldData.fogs + 1;

	if ( !count ) {
		return;
	}

	brushes = (void *)(fileBase + brushesLump->fileofs);
	if (brushesLump->filelen % sizeof(*brushes)) {
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name);
	}
	brushesCount = brushesLump->filelen / sizeof(*brushes);

	sides = (void *)(fileBase + sidesLump->fileofs);
	if (sidesLump->filelen % sizeof(*sides)) {
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name);
	}
	sidesCount = sidesLump->filelen / sizeof(*sides);

	for ( i=0 ; i<count ; i++, fogs++) {
		out->originalBrushNumber = LittleLong( fogs->brushNum );

		if ( (unsigned)out->originalBrushNumber >= brushesCount ) {
			ri.Error( ERR_DROP, "fog brushNumber out of range" );
		}
		brush = brushes + out->originalBrushNumber;

		firstSide = LittleLong( brush->firstSide );

			if ( (unsigned)firstSide > sidesCount - 6 ) {
			ri.Error( ERR_DROP, "fog brush sideNumber out of range" );
		}

		// brushes are always sorted with the axial sides first
		sideNum = firstSide + 0;
		planeNum = LittleLong( sides[ sideNum ].planeNum );
		out->bounds[0][0] = -s_worldData.planes[ planeNum ].dist;

		sideNum = firstSide + 1;
		planeNum = LittleLong( sides[ sideNum ].planeNum );
		out->bounds[1][0] = s_worldData.planes[ planeNum ].dist;

		sideNum = firstSide + 2;
		planeNum = LittleLong( sides[ sideNum ].planeNum );
		out->bounds[0][1] = -s_worldData.planes[ planeNum ].dist;

		sideNum = firstSide + 3;
		planeNum = LittleLong( sides[ sideNum ].planeNum );
		out->bounds[1][1] = s_worldData.planes[ planeNum ].dist;

		sideNum = firstSide + 4;
		planeNum = LittleLong( sides[ sideNum ].planeNum );
		out->bounds[0][2] = -s_worldData.planes[ planeNum ].dist;

		sideNum = firstSide + 5;
		planeNum = LittleLong( sides[ sideNum ].planeNum );
		out->bounds[1][2] = s_worldData.planes[ planeNum ].dist;

		// get information from the shader for fog parameters
		shader = R_FindShader( fogs->shader, LIGHTMAP_NONE, qtrue );

		out->parms = shader->fogParms;

		out->colorInt = ColorBytes4 ( shader->fogParms.color[0] * tr.identityLight, 
			                          shader->fogParms.color[1] * tr.identityLight, 
			                          shader->fogParms.color[2] * tr.identityLight, 1.0 );

		d = shader->fogParms.depthForOpaque < 1 ? 1 : shader->fogParms.depthForOpaque;
		out->tcScale = 1.0f / ( d * 8 );

		// set the gradient vector
		sideNum = LittleLong( fogs->visibleSide );

		if ( sideNum == -1 ) {
			out->hasSurface = qfalse;
		} else {
			out->hasSurface = qtrue;
			planeNum = LittleLong( sides[ firstSide + sideNum ].planeNum );
			VectorSubtract( vec3_origin, s_worldData.planes[ planeNum ].normal, out->surface );
			out->surface[3] = -s_worldData.planes[ planeNum ].dist;
		}

		out++;
	}

}


/*
================
R_LoadLightGrid

================
*/
void R_LoadLightGrid( lump_t *l ) {
	int		i;
	vec3_t	maxs;
	int		numGridPoints;
	world_t	*w;
	float	*wMins, *wMaxs;

	w = &s_worldData;

	w->lightGridInverseSize[0] = 1.0f / w->lightGridSize[0];
	w->lightGridInverseSize[1] = 1.0f / w->lightGridSize[1];
	w->lightGridInverseSize[2] = 1.0f / w->lightGridSize[2];

	wMins = w->bmodels[0].bounds[0];
	wMaxs = w->bmodels[0].bounds[1];

	for ( i = 0 ; i < 3 ; i++ ) {
		w->lightGridOrigin[i] = w->lightGridSize[i] * ceil( wMins[i] / w->lightGridSize[i] );
		maxs[i] = w->lightGridSize[i] * floor( wMaxs[i] / w->lightGridSize[i] );
		w->lightGridBounds[i] = (maxs[i] - w->lightGridOrigin[i])/w->lightGridSize[i] + 1;
	}

	numGridPoints = w->lightGridBounds[0] * w->lightGridBounds[1] * w->lightGridBounds[2];

	if ( l->filelen != numGridPoints * 8 ) {
		ri.Printf( PRINT_WARNING, "WARNING: light grid mismatch\n" );
		w->lightGridData = NULL;
		return;
	}

	w->lightGridData = ri.Hunk_Alloc( l->filelen, h_low );
	Com_Memcpy( w->lightGridData, (void *)(fileBase + l->fileofs), l->filelen );

	// deal with overbright bits
	for ( i = 0 ; i < numGridPoints ; i++ ) {
		R_ColorShiftLightingBytes( &w->lightGridData[i*8], &w->lightGridData[i*8] );
		R_ColorShiftLightingBytes( &w->lightGridData[i*8+3], &w->lightGridData[i*8+3] );
	}
}

/*
================
R_LoadEntities
================
*/
void R_LoadEntities( lump_t *l ) {
	char *p, *token, *s;
	char keyname[MAX_TOKEN_CHARS];
	char value[MAX_TOKEN_CHARS];
	world_t	*w;

	w = &s_worldData;
	w->lightGridSize[0] = 64;
	w->lightGridSize[1] = 64;
	w->lightGridSize[2] = 128;

	p = (char *)(fileBase + l->fileofs);

	// store for reference by the cgame
	w->entityString = ri.Hunk_Alloc( l->filelen + 1, h_low );
	strcpy( w->entityString, p );
	w->entityParsePoint = w->entityString;

	token = COM_ParseExt( &p, qtrue );
	if (!*token || *token != '{') {
		return;
	}

	// only parse the world spawn
	while ( 1 ) {	
		// parse key
		token = COM_ParseExt( &p, qtrue );

		if ( !*token || *token == '}' ) {
			break;
		}
		Q_strncpyz(keyname, token, sizeof(keyname));

		// parse value
		token = COM_ParseExt( &p, qtrue );

		if ( !*token || *token == '}' ) {
			break;
		}
		Q_strncpyz(value, token, sizeof(value));

		// check for remapping of shaders for vertex lighting
		s = "vertexremapshader";
		if (!Q_strncmp(keyname, s, strlen(s)) ) {
			s = strchr(value, ';');
			if (!s) {
				ri.Printf( PRINT_WARNING, "WARNING: no semi colon in vertexshaderremap '%s'\n", value );
				break;
			}
			*s++ = 0;
			if (r_vertexLight->integer) {
				R_RemapShader(value, s, "0");
			}
			continue;
		}
		// check for remapping of shaders
		s = "remapshader";
		if (!Q_strncmp(keyname, s, strlen(s)) ) {
			s = strchr(value, ';');
			if (!s) {
				ri.Printf( PRINT_WARNING, "WARNING: no semi colon in shaderremap '%s'\n", value );
				break;
			}
			*s++ = 0;
			R_RemapShader(value, s, "0");
			continue;
		}
		// check for a different grid size
		if (!Q_stricmp(keyname, "gridsize")) {
			sscanf(value, "%f %f %f", &w->lightGridSize[0], &w->lightGridSize[1], &w->lightGridSize[2] );
			continue;
		}
	}
}

/*
=================
R_GetEntityToken
=================
*/
qboolean R_GetEntityToken( char *buffer, int size ) {
	const char	*s;

	s = COM_Parse( &s_worldData.entityParsePoint );
	Q_strncpyz( buffer, s, size );
	if ( !s_worldData.entityParsePoint || !s[0] ) {
		s_worldData.entityParsePoint = s_worldData.entityString;
		return qfalse;
	} else {
		return qtrue;
	}
}

void RB_AddLightToLightList(int cluster, uint32_t type, uint32_t offsetidx, uint32_t offsetxyz) {
	
	
	for (int i = 0; i < tess.numIndexes; i+=6) {
		vec4_t pos = { 0,0,0,0 };
		/*for (int i = 0; i < tess.numVertexes; i++) {
			VectorAdd(pos, tess.xyz[i], pos);
		}
		VectorScale(pos, 1.0f / tess.numVertexes, pos);*/

		if (vk_d.lightList.numLights < RTX_MAX_LIGHTS) {
			if (vk_d.lightList.numLights >= RTX_MAX_LIGHTS) {
				ri.Error(ERR_FATAL, "Vulkan: Too many lights");
			}

			vec4_t AB;
			vec4_t AC;
			VectorSubtract(tess.xyz[tess.indexes[i + 1]], tess.xyz[tess.indexes[i + 0]], AB);
			VectorSubtract(tess.xyz[tess.indexes[i + 2]], tess.xyz[tess.indexes[i + 0]], AC);
			vec3_t normal;
			CrossProduct(AB, AC, normal);
			float size = VectorLength(normal);
			VectorScale(normal, 1.0f / size, normal);

			

			VectorCopy(tess.xyz[tess.indexes[i + 0]], vk_d.lightList.lights[vk_d.lightList.numLights].pos);
			vk_d.lightList.lights[vk_d.lightList.numLights].cluster = cluster;
			vk_d.lightList.lights[vk_d.lightList.numLights].type = type;
			vk_d.lightList.lights[vk_d.lightList.numLights].offsetIDX = offsetidx;
			vk_d.lightList.lights[vk_d.lightList.numLights].offsetXYZ = offsetxyz;
			vk_d.lightList.lights[vk_d.lightList.numLights].size = size/2;
			memcpy(vk_d.lightList.lights[vk_d.lightList.numLights].normal, normal, sizeof(vec3_t));
			memcpy(vk_d.lightList.lights[vk_d.lightList.numLights].AB, AB, sizeof(vec4_t));
			memcpy(vk_d.lightList.lights[vk_d.lightList.numLights].AC, AC, sizeof(vec4_t));
			if (strstr(tess.shader->name, "proto_lightred")) {
				vk_d.lightList.lights[vk_d.lightList.numLights].color[0] = 245.0f / 255.0f;
				vk_d.lightList.lights[vk_d.lightList.numLights].color[1] = 0;
				vk_d.lightList.lights[vk_d.lightList.numLights].color[2] = 0;
			}
			else if (strstr(tess.shader->name, "ceil1_4")) {
				vk_d.lightList.lights[vk_d.lightList.numLights].color[0] = 245.0f / 255.0f;
				vk_d.lightList.lights[vk_d.lightList.numLights].color[1] = 205.0f / 255.0f;
				vk_d.lightList.lights[vk_d.lightList.numLights].color[2] = 0;
			}
			else if (strstr(tess.shader->name, "proto_light")) {
				vk_d.lightList.lights[vk_d.lightList.numLights].color[0] = 245.0f / 255.0f;
				vk_d.lightList.lights[vk_d.lightList.numLights].color[1] = 205.0f / 255.0f;
				vk_d.lightList.lights[vk_d.lightList.numLights].color[2] = 139.0f / 255.0f;
				if (strstr(tess.shader->name, "proto_light_2k")) vk_d.lightList.lights[vk_d.lightList.numLights].size /= 3;
				else vk_d.lightList.lights[vk_d.lightList.numLights].size /= 3;
				//vk_d.lightList.lights[vk_d.lightList.numLights].size /= 3;
			}
			else if (strstr(tess.shader->name, "gothic_light")) {
				vk_d.lightList.lights[vk_d.lightList.numLights].color[0] = 245.0f / 255.0f;
				vk_d.lightList.lights[vk_d.lightList.numLights].color[1] = 205.0f / 255.0f;
				vk_d.lightList.lights[vk_d.lightList.numLights].color[2] = 139.0f / 255.0f;
				if (strstr(tess.shader->name, "pentagram")) vk_d.lightList.lights[vk_d.lightList.numLights].size /= 2;
				else if(strstr(tess.shader->name, "gothic_light3_2K")) vk_d.lightList.lights[vk_d.lightList.numLights].size /= 6;
				//else vk_d.lightList.lights[vk_d.lightList.numLights].size = 0;
			}
			else if (strstr(tess.shader->name, "baslt4_1")) {
				vk_d.lightList.lights[vk_d.lightList.numLights].color[0] = 245.0f / 255.0f;
				vk_d.lightList.lights[vk_d.lightList.numLights].color[1] = 205.0f / 255.0f;
				vk_d.lightList.lights[vk_d.lightList.numLights].color[2] = 139.0f / 255.0f;
				vk_d.lightList.lights[vk_d.lightList.numLights].size /= 10;
			}
			else if (strstr(tess.shader->name, "flame")) {
				vk_d.lightList.lights[vk_d.lightList.numLights].color[0] = 226.0f / 255.0f;
				vk_d.lightList.lights[vk_d.lightList.numLights].color[1] = 88.0f / 255.0f;
				vk_d.lightList.lights[vk_d.lightList.numLights].color[2] = 34.0f / 255.0f;
				vk_d.lightList.lights[vk_d.lightList.numLights].size /= 2;
				if (strstr(tess.shader->name, "flame1side")) vk_d.lightList.lights[vk_d.lightList.numLights].size /= 3;
			}
			else if (strstr(tess.shader->name, "xceil1_39")) {
				vk_d.lightList.lights[vk_d.lightList.numLights].color[0] = 218.0f / 255.0f;
				vk_d.lightList.lights[vk_d.lightList.numLights].color[1] = 205.0f / 255.0f;
				vk_d.lightList.lights[vk_d.lightList.numLights].color[2] = 173.0f / 255.0f;
			}
			else if (strstr(tess.shader->name, "ceil1_38")) {
				vk_d.lightList.lights[vk_d.lightList.numLights].color[0] = 216.0f / 255.0f;
				vk_d.lightList.lights[vk_d.lightList.numLights].color[1] = 216.0f / 255.0f;
				vk_d.lightList.lights[vk_d.lightList.numLights].color[2] = 208.0f / 255.0f;
				vk_d.lightList.lights[vk_d.lightList.numLights].size /= 4;
			}
			else if (strstr(tess.shader->name, "lamplight_y")) {
				vk_d.lightList.lights[vk_d.lightList.numLights].color[0] = 248.0f / 255.0f;
				vk_d.lightList.lights[vk_d.lightList.numLights].color[1] = 213.0f / 255.0f;
				vk_d.lightList.lights[vk_d.lightList.numLights].color[2] = 104.0f / 255.0f;
			}
			else if (strstr(tess.shader->name, "base_light/light1_")) {
				vk_d.lightList.lights[vk_d.lightList.numLights].color[0] = 193.0f / 255.0f;
				vk_d.lightList.lights[vk_d.lightList.numLights].color[1] = 100.0f / 255.0f;
				vk_d.lightList.lights[vk_d.lightList.numLights].color[2] = 100.0f / 255.0f;
				vk_d.lightList.lights[vk_d.lightList.numLights].size /= 1.75;
			}
			else if (strstr(tess.shader->name, "base_light/ceil1_39")) {
				vk_d.lightList.lights[vk_d.lightList.numLights].color[0] = 140.0f / 255.0f;
				vk_d.lightList.lights[vk_d.lightList.numLights].color[1] = 123.0f / 255.0f;
				vk_d.lightList.lights[vk_d.lightList.numLights].color[2] = 69.0f / 255.0f;
				vk_d.lightList.lights[vk_d.lightList.numLights].size /= 4;
			}
			else if (strstr(tess.shader->name, "base_light/ceil_white5k")) {
				vk_d.lightList.lights[vk_d.lightList.numLights].color[0] = 173.0f / 255.0f;
				vk_d.lightList.lights[vk_d.lightList.numLights].color[1] = 169.0f / 255.0f;
				vk_d.lightList.lights[vk_d.lightList.numLights].color[2] = 140.0f / 255.0f;
				vk_d.lightList.lights[vk_d.lightList.numLights].size /= 4;
			}
			else if (strstr(tess.shader->name, "base_light/ceil1_22a")) {
				vk_d.lightList.lights[vk_d.lightList.numLights].color[0] = 254.0f / 255.0f;
				vk_d.lightList.lights[vk_d.lightList.numLights].color[1] = 86.0f / 255.0f;
				vk_d.lightList.lights[vk_d.lightList.numLights].color[2] = 24.0f / 255.0f;
				vk_d.lightList.lights[vk_d.lightList.numLights].size /= 4;
			}
			else if (strstr(tess.shader->name, "base_light/ceil1_37")) {
				vk_d.lightList.lights[vk_d.lightList.numLights].color[0] = 128.0f / 255.0f;
				vk_d.lightList.lights[vk_d.lightList.numLights].color[1] = 71.0f / 255.0f;
				vk_d.lightList.lights[vk_d.lightList.numLights].color[2] = 142.0f / 255.0f;
				vk_d.lightList.lights[vk_d.lightList.numLights].size /= 1;
			}
			else {
				vk_d.lightList.lights[vk_d.lightList.numLights].color[0] = 0;
				vk_d.lightList.lights[vk_d.lightList.numLights].color[1] = 0;
				vk_d.lightList.lights[vk_d.lightList.numLights].color[2] = 1;
			}
			vk_d.lightList.lights[vk_d.lightList.numLights].color[3] = 0;
			if(vk_d.lightList.lights[vk_d.lightList.numLights].size > 200)vk_d.lightList.lights[vk_d.lightList.numLights].size /= 4;


			if (strstr(tess.shader->name, "lamplight_y")) {
				vec3_t pos = { 0,0,0 };
				for (int u = 0; u < tess.numIndexes; u++) {
					VectorAdd(tess.xyz[tess.indexes[u + 0]], pos, pos);
				}
				VectorScale(pos, 1.0f / tess.numIndexes, pos);
				VectorCopy(pos, vk_d.lightList.lights[vk_d.lightList.numLights].pos);
				vk_d.lightList.lights[vk_d.lightList.numLights].size *= 5;
				vk_d.lightList.numLights++;
				return;
			}

			if (strstr(tess.shader->name, "globalSun")) {
				if (strstr(s_worldData.name, "q3dm3")) {
					vk_d.lightList.lights[vk_d.lightList.numLights].color[0] = 0.5;
					vk_d.lightList.lights[vk_d.lightList.numLights].color[1] = 0.25;
					vk_d.lightList.lights[vk_d.lightList.numLights].color[2] = 0.25;
					//vk_d.lightList.lights[vk_d.lightList.numLights].size *= 5;
				}
				else if (strstr(s_worldData.name, "q3dm0")) {
					vk_d.lightList.lights[vk_d.lightList.numLights].color[0] = 0.25;
					vk_d.lightList.lights[vk_d.lightList.numLights].color[1] = 0.125;
					vk_d.lightList.lights[vk_d.lightList.numLights].color[2] = 0.04;
					vk_d.lightList.lights[vk_d.lightList.numLights].size /= 25;
				}
				else if (strstr(s_worldData.name, "q3dm1")) {
					vk_d.lightList.lights[vk_d.lightList.numLights].color[0] = 0.5;
					vk_d.lightList.lights[vk_d.lightList.numLights].color[1] = 0.23;
					vk_d.lightList.lights[vk_d.lightList.numLights].color[2] = 0.24;
					vk_d.lightList.lights[vk_d.lightList.numLights].size =150;
				}
				else if (strstr(s_worldData.name, "q3dm2")) {
					vk_d.lightList.lights[vk_d.lightList.numLights].color[0] = 0.25;
					vk_d.lightList.lights[vk_d.lightList.numLights].color[1] = 0.125;
					vk_d.lightList.lights[vk_d.lightList.numLights].color[2] = 0.04;
					vk_d.lightList.lights[vk_d.lightList.numLights].size /= 25;
				}
				else if (strstr(s_worldData.name, "q3dm4")) {
					vk_d.lightList.lights[vk_d.lightList.numLights].color[0] = 0.25;
					vk_d.lightList.lights[vk_d.lightList.numLights].color[1] = 0.125;
					vk_d.lightList.lights[vk_d.lightList.numLights].color[2] = 0.04;
					vk_d.lightList.lights[vk_d.lightList.numLights].size /= 25;
				}
				else if (strstr(s_worldData.name, "q3dm9")) {
					vk_d.lightList.lights[vk_d.lightList.numLights].color[0] = 0.25;
					vk_d.lightList.lights[vk_d.lightList.numLights].color[1] = 0.125;
					vk_d.lightList.lights[vk_d.lightList.numLights].color[2] = 0.04;
					vk_d.lightList.lights[vk_d.lightList.numLights].size /= 25;
				}
				else if (strstr(s_worldData.name, "q3dm7")) {
					vk_d.lightList.lights[vk_d.lightList.numLights].color[0] = 0.25;
					vk_d.lightList.lights[vk_d.lightList.numLights].color[1] = 0.125;
					vk_d.lightList.lights[vk_d.lightList.numLights].color[2] = 0.04;
					vk_d.lightList.lights[vk_d.lightList.numLights].size /= 25;
				}
				else {
					vk_d.lightList.lights[vk_d.lightList.numLights].color[0] = 0;
					vk_d.lightList.lights[vk_d.lightList.numLights].color[1] = 0;
					vk_d.lightList.lights[vk_d.lightList.numLights].color[2] = 1;
				}
				vk_d.lightList.numLights++;
				return;
			}

			vk_d.lightList.numLights++;
		}
	}
}

//#define ANIMATE_TEXTURE (tess.shader->stages[0]->bundle[0].numImageAnimations > 0)
//#define UV_CHANGES		(tess.shader->stages[0] != NULL ? ((tess.shader->stages[0]->bundle[0].tcGen != TCGEN_BAD)  && tess.shader->stages[0]->bundle[0].numTexMods > 0) : qfalse)
qboolean RB_ASDataDynamic(shader_t* shader) {
	qboolean changes = qfalse;
	for (int i = 0; i < MAX_SHADER_STAGES; i++) {
		if (tess.shader->rtstages[i] != NULL && tess.shader->rtstages[i]->active) {
			for (int j = 0; j < NUM_TEXTURE_BUNDLES; j++) {
				if (shader->rtstages[i]->bundle[j].numImageAnimations > 0) return qtrue;
				if((shader->rtstages[i]->bundle[j].tcGen != TCGEN_BAD) && (shader->rtstages[i]->bundle[j].numTexMods > 0)) return qtrue;
				if (shader->rtstages[i]->rgbGen == CGEN_WAVEFORM) {
					return qtrue;
				}
			}
		}
	}
	return changes;
}

qboolean RB_ASDynamic(shader_t* shader) {
	return (shader->numDeforms > 0) || (backEnd.currentEntity->e.frame > 0 || backEnd.currentEntity->e.oldframe > 0);
}

// multiple different ways to find a cluster
 int R_FindClusterForPos(const vec3_t p) {
	mnode_t* node;
	float		d;
	cplane_t* plane;

	node = s_worldData.nodes;
	while (1) {
		if (node->contents != -1) {
			break;
		}
		plane = node->plane;
		d = DotProduct(p, plane->normal) - plane->dist;
		if (d >= 0) {
			node = node->children[0];
		}
		else {
			node = node->children[1];
		}
	}

	return node->cluster;
}

 int R_FindClusterForPos2(const vec3_t p) {
	 mnode_t* node;
	 float		d;
	 cplane_t* plane;

	 node = s_worldData.nodes;
	 while (1) {
		 if (node->contents != -1) {
			 break;
		 }
		 plane = node->plane;
		 d = DotProduct(p, plane->normal) - plane->dist;
		 if (d > 0) {
			 node = node->children[0];
		 }
		 else {
			 node = node->children[1];
		 }
	 }

	 return node->cluster;
 }

int R_FindClusterForPos3(const vec3_t p) {
	for (int i = 0; i < s_worldData.numClusters; i++) {
		if (vk_d.clusterList[i].mins[0] <= p[0] && p[0] <= vk_d.clusterList[i].maxs[0] &&
			vk_d.clusterList[i].mins[1] <= p[1] && p[1] <= vk_d.clusterList[i].maxs[1] &&
			vk_d.clusterList[i].mins[2] <= p[2] && p[2] <= vk_d.clusterList[i].maxs[2]) {
			return i;
		}
	}
	return -1;
}

int R_GetClusterFromSurface(surfaceType_t* surf) {
	for (int i = 0; i < s_worldData.numnodes; i++) {
		mnode_t* node = &s_worldData.nodes[i];
		if (node->contents == -1) continue;

		msurface_t** mark = node->firstmarksurface;
		int c = node->nummarksurfaces;
		for (int j = 0; j < c; j++) {
			if (mark[j]->data == surf) return node->cluster;
		}
	}
	return -1;
}

void R_RecursiveCreateAS(mnode_t* node, uint32_t* countIDXstatic, uint32_t* countXYZstatic, uint32_t* countIDXdynamicData, uint32_t* countXYZdynamicData, uint32_t* countIDXdynamicAS, uint32_t* countXYZdynamicAS, qboolean transparent) {
	
	do {
		if (node->contents != -1) {
			break;
		}
		R_RecursiveCreateAS(node->children[0], countIDXstatic, countXYZstatic, countIDXdynamicData, countXYZdynamicData, countIDXdynamicAS, countXYZdynamicAS, transparent);
		node = node->children[1];
	} while (1);
	{
		// leaf node, so add mark surfaces
		int			c;
		msurface_t* surf, ** mark;
		
		mark = node->firstmarksurface;
		c = node->nummarksurfaces;
		for (int j = 0; j < c; j++) {
			tess.numVertexes = 0;
			tess.numIndexes = 0;
			surf = mark[j];
			surf->notBrush = qtrue;

			shader_t* shader = tr.shaders[surf->shader->index];
			
			if (RB_IsTransparent(shader) != transparent) continue;
			if (RB_SkipObject(shader)
				|| *surf->data == SF_BAD || *surf->data == SF_SKIP) {
				surf->skip = qtrue;
				continue;
			}

			if (strstr(shader->name, "flag")) {
				int x = 2; //continue;
				//continue;
			}
			//if (strstr(shader->name, "models/mapobjects/console/under") || strstr(shader->name, "textures/sfx/beam") || strstr(shader->name, "models/mapobjects/lamps/flare03")
			//	|| strstr(shader->name, "Shadow") || shader->isSky
			//	|| *surf->data == SF_BAD || *surf->data == SF_SKIP
			//	|| shader->surfaceFlags == SURF_NODRAW || shader->surfaceFlags == SURF_SKIP
			//	|| shader->rtstages[0] == NULL || !shader->rtstages[0]->active
			//	|| strstr(shader->name, "slamp/slamp3")
			//	|| strstr(shader->name, "gratelamp_flare")) {

			//	//continue;
			//	if (!strstr(shader->name, "glass") && !strstr(shader->name, "console/jacobs") && !strstr(shader->name, "kmlamp_white") && !strstr(shader->name, "slamp/slamp2")
			//		&& !strstr(shader->name, "timlamp/timlamp") && !strstr(shader->name, "lamplight_y") && !strstr(shader->name, "textures/liquids/calm_poollight")){// && !strstr(shader->name, "flame")) {
			//		surf->skip = qtrue;
			//		continue;
			//	}
			//}
			//if (!transparent && ((shader->contentFlags & CONTENTS_TRANSLUCENT) == CONTENTS_TRANSLUCENT || shader->sort > SS_OPAQUE)) {
			//	if (!strstr(shader->name, "glass") && !strstr(shader->name, "console/jacobs") && !strstr(shader->name, "kmlamp_white") && !strstr(shader->name, "slamp/slamp2")
			//		 && !strstr(shader->name, "lamplight_y") && !strstr(shader->name, "textures/liquids/calm_poollight"))continue;
			//}

			
			//grate1_3
			tess.shader = shader;

			rb_surfaceTable[*surf->data](surf->data);
			if (tess.numIndexes == 0) continue;


			if (!surf->added && !surf->skip) {		
				int clusterIDX = node->cluster;

				uint32_t material = 0;
				// different buffer and offsets for static, dynamic data and dynamic as
				uint32_t* countIDX;
				uint32_t* countXYZ;
				vkbuffer_t *idx_buffer;
				vkbuffer_t* xyz_buffer;
				uint32_t* idx_buffer_offset;
				uint32_t* xyz_buffer_offset;

				qboolean dynamic = qfalse;
				if (tess.shader->surfaceFlags == SURF_NODRAW) continue;

				// for dm0 some strange object in the distance
				if (Distance(tess.xyz, (vec3_t){ -1830.72034, 3114.09717, 166.582550 }) < 250) {
					continue;
				}
				// if as is static we need one buffer
				if (!RB_ASDynamic(tess.shader) && !RB_ASDataDynamic(tess.shader)) {
					countIDX = countIDXstatic;
					countXYZ = countXYZstatic;
					idx_buffer = &vk_d.geometry.idx_world_static;
					xyz_buffer = &vk_d.geometry.xyz_world_static;
					idx_buffer_offset = &vk_d.geometry.idx_world_static_offset;
					xyz_buffer_offset = &vk_d.geometry.xyz_world_static_offset;

					if (RB_IsLight(tess.shader)) RB_AddLightToLightList(clusterIDX, BAS_WORLD_STATIC,
						 (*countIDX),
						0);
					RB_UploadCluster(&vk_d.geometry.cluster_world_static, vk_d.geometry.cluster_world_static_offset, node->cluster);
					vk_d.geometry.cluster_world_static_offset += (tess.numIndexes/3);
				}
				// if the data of an object changes we need one as buffer but #swapchain object data buffers
				else if (!RB_ASDynamic(tess.shader) && RB_ASDataDynamic(tess.shader)) {
					countIDX = countIDXdynamicData;
					countXYZ = countXYZdynamicData;
					idx_buffer = &vk_d.geometry.idx_world_dynamic_data;
					xyz_buffer = &vk_d.geometry.xyz_world_dynamic_data;
					idx_buffer_offset = &vk_d.geometry.idx_world_dynamic_data_offset;
					xyz_buffer_offset = &vk_d.geometry.xyz_world_dynamic_data_offset;
					dynamic = qtrue;

					// keep track of dynamic data surf
					vk_d.updateDataOffsetXYZ[vk_d.updateDataOffsetXYZCount].shader = tess.shader;
					vk_d.updateDataOffsetXYZ[vk_d.updateDataOffsetXYZCount].numXYZ = tess.numVertexes;
					vk_d.updateDataOffsetXYZ[vk_d.updateDataOffsetXYZCount].surf = surf;
					vk_d.updateDataOffsetXYZ[vk_d.updateDataOffsetXYZCount].offsetIDX = *idx_buffer_offset;
					vk_d.updateDataOffsetXYZ[vk_d.updateDataOffsetXYZCount].offsetXYZ = *xyz_buffer_offset;
					vk_d.updateDataOffsetXYZ[vk_d.updateDataOffsetXYZCount].cluster = clusterIDX;
					vk_d.updateDataOffsetXYZCount++;

					if (RB_IsLight(tess.shader)) RB_AddLightToLightList(clusterIDX, BAS_WORLD_DYNAMIC_DATA,
						 (*countIDX),
						0);
					RB_UploadCluster(&vk_d.geometry.cluster_world_dynamic_data, vk_d.geometry.cluster_world_dynamic_data_offset, node->cluster);
					vk_d.geometry.cluster_world_dynamic_data_offset += (tess.numIndexes / 3);
				}
				// object changes we need #swapchain as buffer
				else if (RB_ASDynamic(tess.shader)) {
					countIDX = countIDXdynamicAS;
					countXYZ = countXYZdynamicAS;
					idx_buffer = &vk_d.geometry.idx_world_dynamic_as;
					xyz_buffer = &vk_d.geometry.xyz_world_dynamic_as;
					idx_buffer_offset = &vk_d.geometry.idx_world_dynamic_as_offset;
					xyz_buffer_offset = &vk_d.geometry.xyz_world_dynamic_as_offset;
					dynamic = qtrue;

					// keep track of dynamic as surf
					vk_d.updateASOffsetXYZ[vk_d.updateASOffsetXYZCount].shader = tess.shader;
					vk_d.updateASOffsetXYZ[vk_d.updateASOffsetXYZCount].numXYZ = tess.numVertexes;
					vk_d.updateASOffsetXYZ[vk_d.updateASOffsetXYZCount].surf = surf;
					vk_d.updateASOffsetXYZ[vk_d.updateASOffsetXYZCount].offsetIDX = *idx_buffer_offset;
					vk_d.updateASOffsetXYZ[vk_d.updateASOffsetXYZCount].offsetXYZ = *xyz_buffer_offset;
					vk_d.updateASOffsetXYZ[vk_d.updateASOffsetXYZCount].countXYZ = *countXYZ;
					vk_d.updateASOffsetXYZ[vk_d.updateASOffsetXYZCount].cluster = clusterIDX;
					vk_d.updateASOffsetXYZCount++;

					if (RB_IsLight(tess.shader)) RB_AddLightToLightList(clusterIDX, BAS_WORLD_DYNAMIC_AS,
						(*countIDX),
						0);
					RB_UploadCluster(&vk_d.geometry.cluster_world_dynamic_as, vk_d.geometry.cluster_world_dynamic_as_offset, node->cluster);
					vk_d.geometry.cluster_world_dynamic_as_offset += (tess.numIndexes / 3);
				}
				else {
					surf->skip = qtrue;
					continue;
				}
				
				// write idx
				RB_UploadIDX(idx_buffer, (*idx_buffer_offset), (*countXYZ));
				if (dynamic)for (int i = 1; i < vk.swapchain.imageCount; i++) RB_UploadIDX(idx_buffer[i], (*idx_buffer_offset), (*countXYZ));
				
				// write xyz
				RB_UploadXYZ(xyz_buffer, (*xyz_buffer_offset), clusterIDX);
				if (dynamic) {
					for (int i = 1; i < vk.swapchain.imageCount; i++) {
						RB_UploadXYZ(&xyz_buffer[i], (*xyz_buffer_offset), clusterIDX);
					}
				}	
				surf->added = qtrue;
	
				(*idx_buffer_offset) += tess.numIndexes;
				(*xyz_buffer_offset) += tess.numVertexes;
				(*countIDX) += tess.numIndexes;
				(*countXYZ) += tess.numVertexes;
			}
			tess.numVertexes = 0;
			tess.numIndexes = 0;
		}	
	}
}

void R_CalcClusterAABB(mnode_t* node) {
	do {
		if (node->contents != -1) {
			break;
		}
		R_CalcClusterAABB(node->children[0]);
		node = node->children[1];
	} while (1);
	{
		if (node->cluster < 0) {
			return;
		}
		if (node->cluster > s_worldData.numClusters) {
			return;
		}
		vk_d.clusterList[node->cluster].mins[0] = min(vk_d.clusterList[node->cluster].mins[0], node->mins[0]);
		vk_d.clusterList[node->cluster].mins[1] = min(vk_d.clusterList[node->cluster].mins[1], node->mins[1]);
		vk_d.clusterList[node->cluster].mins[2] = min(vk_d.clusterList[node->cluster].mins[2], node->mins[2]);

		vk_d.clusterList[node->cluster].maxs[0] = max(vk_d.clusterList[node->cluster].maxs[0], node->maxs[0]);
		vk_d.clusterList[node->cluster].maxs[1] = max(vk_d.clusterList[node->cluster].maxs[1], node->maxs[1]);
		vk_d.clusterList[node->cluster].maxs[2] = max(vk_d.clusterList[node->cluster].maxs[2], node->maxs[2]);
	}
}

void R_CreatePrimaryRaysPipeline() {
	VkShaderStageFlagBits flags = VK_SHADER_STAGE_RAYGEN_BIT_NV | VK_SHADER_STAGE_ANY_HIT_BIT_NV | VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;

	for (int i = 0; i < vk.swapchain.imageCount; i++) {
		VK_AddAccelerationStructure(&vk_d.rtxDescriptor[i], BINDING_OFFSET_AS, flags);
		VK_SetAccelerationStructure(&vk_d.rtxDescriptor[i], BINDING_OFFSET_AS, flags, &vk_d.topAS[i].accelerationStructure);
		VK_AddStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_INSTANCE_DATA, flags);
		VK_SetStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_INSTANCE_DATA, flags, vk_d.instanceDataBuffer[i].buffer);

		VK_AddStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_XYZ_WORLD_STATIC, flags);
		VK_SetStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_XYZ_WORLD_STATIC, flags, vk_d.geometry.xyz_world_static.buffer);
		VK_AddStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_IDX_WORLD_STATIC, flags);
		VK_SetStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_IDX_WORLD_STATIC, flags, vk_d.geometry.idx_world_static.buffer);
		VK_AddStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_XYZ_WORLD_DYNAMIC_DATA, flags);
		VK_SetStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_XYZ_WORLD_DYNAMIC_DATA, flags, vk_d.geometry.xyz_world_dynamic_data[i].buffer);
		VK_AddStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_IDX_WORLD_DYNAMIC_DATA, flags);
		VK_SetStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_IDX_WORLD_DYNAMIC_DATA, flags, vk_d.geometry.idx_world_dynamic_data[i].buffer);
		VK_AddStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_XYZ_WORLD_DYNAMIC_AS, flags);
		VK_SetStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_XYZ_WORLD_DYNAMIC_AS, flags, vk_d.geometry.xyz_world_dynamic_as[i].buffer);
		VK_AddStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_IDX_WORLD_DYNAMIC_AS, flags);
		VK_SetStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_IDX_WORLD_DYNAMIC_AS, flags, vk_d.geometry.idx_world_dynamic_as[i].buffer);
		VK_AddStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_XYZ_ENTITY_STATIC, flags);
		VK_SetStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_XYZ_ENTITY_STATIC, flags, vk_d.geometry.xyz_entity_static.buffer);
		VK_AddStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_IDX_ENTITY_STATIC, flags);
		VK_SetStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_IDX_ENTITY_STATIC, flags, vk_d.geometry.idx_entity_static.buffer);
		VK_AddStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_XYZ_ENTITY_DYNAMIC, flags);
		VK_SetStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_XYZ_ENTITY_DYNAMIC, flags, vk_d.geometry.xyz_entity_dynamic[i].buffer);
		VK_AddStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_IDX_ENTITY_DYNAMIC, flags);
		VK_SetStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_IDX_ENTITY_DYNAMIC, flags, vk_d.geometry.idx_entity_dynamic[i].buffer);

		VK_AddStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_CLUSTER_WORLD_STATIC, flags);
		VK_SetStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_CLUSTER_WORLD_STATIC, flags, vk_d.geometry.cluster_world_static.buffer);
		VK_AddStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_CLUSTER_WORLD_DYNAMIC_DATA, flags);
		VK_SetStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_CLUSTER_WORLD_DYNAMIC_DATA, flags, vk_d.geometry.cluster_world_dynamic_data.buffer);
		VK_AddStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_CLUSTER_WORLD_DYNAMIC_AS, flags);
		VK_SetStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_CLUSTER_WORLD_DYNAMIC_AS, flags, vk_d.geometry.cluster_world_dynamic_as.buffer);
		VK_AddStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_CLUSTER_ENTITY_STATIC, flags);
		VK_SetStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_CLUSTER_ENTITY_STATIC, flags, vk_d.geometry.cluster_entity_static.buffer);

		VK_AddSampler(&vk_d.rtxDescriptor[i], BINDING_OFFSET_ENVMAP, VK_SHADER_STAGE_RAYGEN_BIT_NV | VK_SHADER_STAGE_MISS_BIT_NV);
		VK_SetSampler(&vk_d.rtxDescriptor[i], BINDING_OFFSET_ENVMAP, VK_SHADER_STAGE_RAYGEN_BIT_NV | VK_SHADER_STAGE_MISS_BIT_NV, vk_d.accelerationStructures.envmap.sampler, vk_d.accelerationStructures.envmap.view);
		VK_AddSampler(&vk_d.rtxDescriptor[i], BINDING_OFFSET_BLUE_NOISE, VK_SHADER_STAGE_RAYGEN_BIT_NV);
		VK_SetSampler(&vk_d.rtxDescriptor[i], BINDING_OFFSET_BLUE_NOISE, VK_SHADER_STAGE_RAYGEN_BIT_NV, vk_d.blueNoiseTex.sampler, vk_d.blueNoiseTex.view);

		VK_AddStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_UBO_LIGHTS, VK_SHADER_STAGE_RAYGEN_BIT_NV);
		VK_SetStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_UBO_LIGHTS, VK_SHADER_STAGE_RAYGEN_BIT_NV, vk_d.uboLightList[i].buffer);
		VK_AddStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_VIS_DATA, VK_SHADER_STAGE_RAYGEN_BIT_NV);
		VK_SetStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_VIS_DATA, VK_SHADER_STAGE_RAYGEN_BIT_NV, vk_d.accelerationStructures.visData.view);
		VK_AddStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_LIGHT_VIS_DATA, VK_SHADER_STAGE_RAYGEN_BIT_NV);
		VK_SetStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_LIGHT_VIS_DATA, VK_SHADER_STAGE_RAYGEN_BIT_NV, vk_d.accelerationStructures.lightVisData.view);
		VK_AddStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_LIGHT_VIS_DATA2, VK_SHADER_STAGE_RAYGEN_BIT_NV);
		VK_SetStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_LIGHT_VIS_DATA2, VK_SHADER_STAGE_RAYGEN_BIT_NV, vk_d.accelerationStructures.lightVisData2.view);

		
		// global ubo
		VK_AddUniformBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_GLOBAL_UBO, VK_SHADER_STAGE_RAYGEN_BIT_NV);
		VK_SetUniformBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_GLOBAL_UBO, VK_SHADER_STAGE_RAYGEN_BIT_NV, vk_d.uboBuffer[i].buffer);


		VK_AddStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_GBUFFER_DIRECT_ILLUMINATION, VK_SHADER_STAGE_RAYGEN_BIT_NV);
		VK_SetStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_GBUFFER_DIRECT_ILLUMINATION, VK_SHADER_STAGE_RAYGEN_BIT_NV, vk_d.gBuffer[i].directIllumination.view);
		VK_AddStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_GBUFFER_INDIRECT_ILLUMINATION, VK_SHADER_STAGE_RAYGEN_BIT_NV);
		VK_SetStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_GBUFFER_INDIRECT_ILLUMINATION, VK_SHADER_STAGE_RAYGEN_BIT_NV, vk_d.gBuffer[i].indirectIllumination.view);
		VK_AddStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_GBUFFER_ALBEDO, VK_SHADER_STAGE_RAYGEN_BIT_NV);
		VK_SetStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_GBUFFER_ALBEDO, VK_SHADER_STAGE_RAYGEN_BIT_NV, vk_d.gBuffer[i].albedo.view);
		VK_AddStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_GBUFFER_NORMAL, VK_SHADER_STAGE_RAYGEN_BIT_NV);
		VK_SetStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_GBUFFER_NORMAL, VK_SHADER_STAGE_RAYGEN_BIT_NV, vk_d.gBuffer[i].normals.view);
		VK_AddStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_GBUFFER_REFLECTION, VK_SHADER_STAGE_RAYGEN_BIT_NV);
		VK_SetStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_GBUFFER_REFLECTION, VK_SHADER_STAGE_RAYGEN_BIT_NV, vk_d.gBuffer[i].reflection.view);
		VK_AddStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_GBUFFER_POS, VK_SHADER_STAGE_RAYGEN_BIT_NV);
		VK_SetStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_GBUFFER_POS, VK_SHADER_STAGE_RAYGEN_BIT_NV, vk_d.gBuffer[i].position.view);
		VK_AddStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_GBUFFER_OBJECT, VK_SHADER_STAGE_RAYGEN_BIT_NV);
		VK_SetStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_GBUFFER_OBJECT, VK_SHADER_STAGE_RAYGEN_BIT_NV, vk_d.gBuffer[i].objectInfo.view);
		VK_AddStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_GBUFFER_MOTION, VK_SHADER_STAGE_RAYGEN_BIT_NV);
		VK_SetStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_GBUFFER_MOTION, VK_SHADER_STAGE_RAYGEN_BIT_NV, vk_d.gBuffer[i].motion.view);
		VK_AddStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_GBUFFER_VIEW_DIR, VK_SHADER_STAGE_RAYGEN_BIT_NV);
		VK_SetStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_GBUFFER_VIEW_DIR, VK_SHADER_STAGE_RAYGEN_BIT_NV, vk_d.gBuffer[i].viewDir.view);
		VK_AddStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_GBUFFER_TRANSPARENT, VK_SHADER_STAGE_RAYGEN_BIT_NV);
		VK_SetStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_GBUFFER_TRANSPARENT, VK_SHADER_STAGE_RAYGEN_BIT_NV, vk_d.gBuffer[i].transparent.view);
		// result
		VK_AddStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_RESULT_OUTPUT, VK_SHADER_STAGE_RAYGEN_BIT_NV);
		VK_SetStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_RESULT_OUTPUT, VK_SHADER_STAGE_RAYGEN_BIT_NV, vk_d.accelerationStructures.resultImage[i].view);
		// accumulation
		VK_AddStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_RESULT_ACCUMULATION, VK_SHADER_STAGE_RAYGEN_BIT_NV);
		VK_SetStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_RESULT_ACCUMULATION, VK_SHADER_STAGE_RAYGEN_BIT_NV, vk_d.accelerationStructures.accumulationImage[i].view);

		// asvgf
		VK_AddStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_ASVGF_RNG, VK_SHADER_STAGE_RAYGEN_BIT_NV);
		VK_SetStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_ASVGF_RNG, VK_SHADER_STAGE_RAYGEN_BIT_NV, vk_d.asvgf[i].rngSeed.view);
		//VK_AddStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_ASVGF_RNG_B, VK_SHADER_STAGE_RAYGEN_BIT_NV);
		//VK_SetStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_ASVGF_RNG_B, VK_SHADER_STAGE_RAYGEN_BIT_NV, vk_d.asvgf[i].rngSeedB.view);
		VK_AddStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_ASVGF_GRAD_SMPL_POS, VK_SHADER_STAGE_RAYGEN_BIT_NV);
		VK_SetStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_ASVGF_GRAD_SMPL_POS, VK_SHADER_STAGE_RAYGEN_BIT_NV, vk_d.asvgf[i].gradSamplePos.view);

		int prevIndex = (i + (vk.swapchain.imageCount - 1)) % vk.swapchain.imageCount;
		VK_AddUniformBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_GLOBAL_UBO_PREV, VK_SHADER_STAGE_RAYGEN_BIT_NV);
		VK_SetUniformBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_GLOBAL_UBO_PREV, VK_SHADER_STAGE_RAYGEN_BIT_NV, vk_d.uboBuffer[prevIndex].buffer);
		// accumulation prev
		VK_AddStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_RESULT_ACCUMULATION_PREV, VK_SHADER_STAGE_RAYGEN_BIT_NV);
		VK_SetStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_RESULT_ACCUMULATION_PREV, VK_SHADER_STAGE_RAYGEN_BIT_NV, vk_d.accelerationStructures.accumulationImage[prevIndex].view);
		
		VK_AddStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_POS_FWD, VK_SHADER_STAGE_RAYGEN_BIT_NV);
		VK_SetStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_POS_FWD, VK_SHADER_STAGE_RAYGEN_BIT_NV, vk_d.asvgf[i].positionFwd.view);
		VK_AddStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_OBJECT_FWD, VK_SHADER_STAGE_RAYGEN_BIT_NV);
		VK_SetStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_OBJECT_FWD, VK_SHADER_STAGE_RAYGEN_BIT_NV, vk_d.asvgf[i].objectFwd.view);

		VK_AddStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_GBUFFER_DEPTH_NORMAL, VK_SHADER_STAGE_RAYGEN_BIT_NV);
		VK_SetStorageImage(&vk_d.rtxDescriptor[i], BINDING_OFFSET_GBUFFER_DEPTH_NORMAL, VK_SHADER_STAGE_RAYGEN_BIT_NV, vk_d.gBuffer[i].depthNormal.view);

		VK_AddStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_INSTANCE_DATA_PREV, flags);
		VK_SetStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_INSTANCE_DATA_PREV, flags, vk_d.instanceDataBuffer[prevIndex].buffer);
		VK_AddStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_XYZ_WORLD_DYNAMIC_DATA_PREV, flags);
		VK_SetStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_XYZ_WORLD_DYNAMIC_DATA_PREV, flags, vk_d.geometry.xyz_world_dynamic_data[prevIndex].buffer);
		VK_AddStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_IDX_WORLD_DYNAMIC_DATA_PREV, flags);
		VK_SetStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_IDX_WORLD_DYNAMIC_DATA_PREV, flags, vk_d.geometry.idx_world_dynamic_data[prevIndex].buffer);
		VK_AddStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_XYZ_WORLD_DYNAMIC_AS_PREV, flags);
		VK_SetStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_XYZ_WORLD_DYNAMIC_AS_PREV, flags, vk_d.geometry.xyz_world_dynamic_as[prevIndex].buffer);
		VK_AddStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_IDX_WORLD_DYNAMIC_AS_PREV, flags);
		VK_SetStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_IDX_WORLD_DYNAMIC_AS_PREV, flags, vk_d.geometry.idx_world_dynamic_as[prevIndex].buffer);
		VK_AddStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_XYZ_ENTITY_DYNAMIC_PREV, flags);
		VK_SetStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_XYZ_ENTITY_DYNAMIC_PREV, flags, vk_d.geometry.xyz_entity_dynamic[prevIndex].buffer);
		VK_AddStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_IDX_ENTITY_DYNAMIC_PREV, flags);
		VK_SetStorageBuffer(&vk_d.rtxDescriptor[i], BINDING_OFFSET_IDX_ENTITY_DYNAMIC_PREV, flags, vk_d.geometry.idx_entity_dynamic[prevIndex].buffer);
		VK_FinishDescriptor(&vk_d.rtxDescriptor[i]);

		VK_AddStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_INSTANCE_DATA, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_INSTANCE_DATA, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.instanceDataBuffer[i].buffer);

		VK_AddStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_XYZ_WORLD_STATIC, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_XYZ_WORLD_STATIC, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.geometry.xyz_world_static.buffer);
		VK_AddStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_IDX_WORLD_STATIC, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_IDX_WORLD_STATIC, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.geometry.idx_world_static.buffer);
		VK_AddStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_XYZ_WORLD_DYNAMIC_DATA, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_XYZ_WORLD_DYNAMIC_DATA, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.geometry.xyz_world_dynamic_data[i].buffer);
		VK_AddStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_IDX_WORLD_DYNAMIC_DATA, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_IDX_WORLD_DYNAMIC_DATA, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.geometry.idx_world_dynamic_data[i].buffer);
		VK_AddStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_XYZ_WORLD_DYNAMIC_AS, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_XYZ_WORLD_DYNAMIC_AS, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.geometry.xyz_world_dynamic_as[i].buffer);
		VK_AddStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_IDX_WORLD_DYNAMIC_AS, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_IDX_WORLD_DYNAMIC_AS, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.geometry.idx_world_dynamic_as[i].buffer);
		VK_AddStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_XYZ_ENTITY_STATIC, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_XYZ_ENTITY_STATIC, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.geometry.xyz_entity_static.buffer);
		VK_AddStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_IDX_ENTITY_STATIC, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_IDX_ENTITY_STATIC, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.geometry.idx_entity_static.buffer);
		VK_AddStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_XYZ_ENTITY_DYNAMIC, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_XYZ_ENTITY_DYNAMIC, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.geometry.xyz_entity_dynamic[i].buffer);
		VK_AddStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_IDX_ENTITY_DYNAMIC, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_IDX_ENTITY_DYNAMIC, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.geometry.idx_entity_dynamic[i].buffer);

		VK_AddUniformBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_GLOBAL_UBO, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetUniformBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_GLOBAL_UBO, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.uboBuffer[i].buffer);
		VK_AddUniformBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_GLOBAL_UBO_PREV, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetUniformBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_GLOBAL_UBO_PREV, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.uboBuffer[prevIndex].buffer);

		VK_AddStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_GBUFFER_VIEW_DIR_PREV, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_GBUFFER_VIEW_DIR_PREV, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.gBuffer[prevIndex].viewDir.view);
		VK_AddStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_GBUFFER_NORMAL_PREV, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_GBUFFER_NORMAL_PREV, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.gBuffer[prevIndex].normals.view);

		VK_AddStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_GBUFFER_ALBEDO, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_GBUFFER_ALBEDO, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.gBuffer[i].albedo.view);

		VK_AddSampler(&vk_d.computeDescriptor[i], BINDING_OFFSET_BLUE_NOISE, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetSampler(&vk_d.computeDescriptor[i], BINDING_OFFSET_BLUE_NOISE, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.blueNoiseTex.sampler, vk_d.blueNoiseTex.view);

		VK_AddStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_GBUFFER_TRANSPARENT, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_GBUFFER_TRANSPARENT, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.gBuffer[i].transparent.view);
		VK_AddStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_GBUFFER_MOTION, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_GBUFFER_MOTION, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.gBuffer[i].motion.view);
		VK_AddStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_GBUFFER_VIEW_DIR, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_GBUFFER_VIEW_DIR, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.gBuffer[i].viewDir.view);
		VK_AddStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_GBUFFER_NORMAL, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_GBUFFER_NORMAL, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.gBuffer[i].normals.view);
		VK_AddStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_GBUFFER_OBJECT, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_GBUFFER_OBJECT, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.gBuffer[i].objectInfo.view);
		VK_AddStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_GBUFFER_OBJECT_PREV, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_GBUFFER_OBJECT_PREV, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.gBuffer[prevIndex].objectInfo.view);
		/*VK_AddStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_GBUFFER_DIRECT_ILLUMINATION, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_GBUFFER_DIRECT_ILLUMINATION, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.gBuffer[i].directIllumination.view);
		VK_AddStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_GBUFFER_DIRECT_ILLUMINATION_PREV, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_GBUFFER_DIRECT_ILLUMINATION_PREV, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.gBuffer[prevIndex].directIllumination.view);*/

		VK_AddStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_POS_FWD, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_POS_FWD, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.asvgf[i].positionFwd.view);
		VK_AddStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_OBJECT_FWD, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_OBJECT_FWD, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.asvgf[i].objectFwd.view);

		VK_AddStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_INSTANCE_PREV_TO_CURR, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_INSTANCE_PREV_TO_CURR, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.prevToCurrInstanceBuffer[i].buffer);

		
		// cluster prev
		VK_AddStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_INSTANCE_DATA_PREV, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_INSTANCE_DATA_PREV, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.instanceDataBuffer[prevIndex].buffer);
		VK_AddStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_XYZ_WORLD_DYNAMIC_DATA_PREV, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_XYZ_WORLD_DYNAMIC_DATA_PREV, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.geometry.xyz_world_dynamic_data[prevIndex].buffer);
		VK_AddStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_IDX_WORLD_DYNAMIC_DATA_PREV, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_IDX_WORLD_DYNAMIC_DATA_PREV, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.geometry.idx_world_dynamic_data[prevIndex].buffer);
		VK_AddStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_XYZ_WORLD_DYNAMIC_AS_PREV, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_XYZ_WORLD_DYNAMIC_AS_PREV, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.geometry.xyz_world_dynamic_as[prevIndex].buffer);
		VK_AddStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_IDX_WORLD_DYNAMIC_AS_PREV, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_IDX_WORLD_DYNAMIC_AS_PREV, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.geometry.idx_world_dynamic_as[prevIndex].buffer);
		VK_AddStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_XYZ_ENTITY_DYNAMIC_PREV, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_XYZ_ENTITY_DYNAMIC_PREV, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.geometry.xyz_entity_dynamic[prevIndex].buffer);
		VK_AddStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_IDX_ENTITY_DYNAMIC_PREV, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_IDX_ENTITY_DYNAMIC_PREV, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.geometry.idx_entity_dynamic[prevIndex].buffer);

		VK_AddStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_CLUSTER_WORLD_STATIC, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_CLUSTER_WORLD_STATIC, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.geometry.cluster_world_static.buffer);
		VK_AddStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_CLUSTER_WORLD_DYNAMIC_DATA, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_CLUSTER_WORLD_DYNAMIC_DATA, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.geometry.cluster_world_dynamic_data.buffer);
		VK_AddStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_CLUSTER_WORLD_DYNAMIC_AS, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_CLUSTER_WORLD_DYNAMIC_AS, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.geometry.cluster_world_dynamic_as.buffer);
		VK_AddStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_CLUSTER_ENTITY_STATIC, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_CLUSTER_ENTITY_STATIC, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.geometry.cluster_entity_static.buffer);

		// result
		VK_AddStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_RESULT_OUTPUT, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_RESULT_OUTPUT, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.accelerationStructures.resultImage[i].view);
		// accumulation
		VK_AddStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_RESULT_ACCUMULATION, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_RESULT_ACCUMULATION, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.accelerationStructures.accumulationImage[i].view);
		VK_AddStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_RESULT_ACCUMULATION_PREV, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_RESULT_ACCUMULATION_PREV, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.accelerationStructures.accumulationImage[prevIndex].view);


		// asvgf

		VK_AddStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_RNG, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_RNG, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.asvgf[i].rngSeed.view);
		VK_AddStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_RNG_PREV, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_RNG_PREV, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.asvgf[prevIndex].rngSeed.view);
		/*VK_AddStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_RNG_B, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_RNG_B, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.asvgf[i].rngSeedB.view);*/
		VK_AddStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_GRAD_A, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_GRAD_A, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.asvgf[i].gradA.view);
		VK_AddStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_GRAD_B, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_GRAD_B, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.asvgf[i].gradB.view);
		VK_AddSampler(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_GRAD_A_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetSampler(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_GRAD_A_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.asvgf[i].gradA.sampler, vk_d.asvgf[i].gradA.view);
		VK_AddSampler(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_GRAD_B_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetSampler(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_GRAD_B_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.asvgf[i].gradB.sampler, vk_d.asvgf[i].gradB.view);



		VK_AddStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_GRAD_SMPL_POS, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_GRAD_SMPL_POS, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.asvgf[i].gradSamplePos.view);
		VK_AddStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_GRAD_SMPL_POS_PREV, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_GRAD_SMPL_POS_PREV, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.asvgf[prevIndex].gradSamplePos.view);
		VK_AddStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_HIST_COLOR, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_HIST_COLOR, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.asvgf[i].histColor.view);
		VK_AddStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_HIST_COLOR_PREV, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_HIST_COLOR_PREV, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.asvgf[prevIndex].histColor.view);
		VK_AddSampler(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_HIST_COLOR_S, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetSampler(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_HIST_COLOR_S, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.asvgf[i].histColor.sampler, vk_d.asvgf[i].histColor.view);


		VK_AddStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_HIST_MOMENTS, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_HIST_MOMENTS, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.asvgf[i].histMoments.view);
		VK_AddStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_HIST_MOMENTS_PREV, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_HIST_MOMENTS_PREV, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.asvgf[prevIndex].histMoments.view);

		VK_AddStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_ATROUS_A, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_ATROUS_A, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.asvgf[i].atrousA.view);
		VK_AddStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_ATROUS_B, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_ATROUS_B, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.asvgf[i].atrousB.view);
		VK_AddSampler(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_ATROUS_A_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetSampler(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_ATROUS_A_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.asvgf[i].atrousA.sampler, vk_d.asvgf[i].atrousA.view);
		VK_AddSampler(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_ATROUS_B_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetSampler(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_ATROUS_B_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.asvgf[i].atrousB.sampler, vk_d.asvgf[i].atrousB.view);

		VK_AddStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_TAA_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_TAA_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.asvgf[i].taa.view);
		/*VK_AddStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_TAA_PREV, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_TAA_PREV, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.asvgf[prevIndex].taa.view);*/

		VK_AddSampler(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_TAA, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetSampler(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_TAA, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.asvgf[i].taa.sampler, vk_d.asvgf[i].taa.view);
		VK_AddSampler(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_TAA_PREV, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetSampler(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_TAA_PREV, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.asvgf[prevIndex].taa.sampler, vk_d.asvgf[prevIndex].taa.view);

		VK_AddStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_COLOR, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_COLOR, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.asvgf[i].color.view);

		VK_AddStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_GBUFFER_DIRECT_ILLUMINATION, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_GBUFFER_DIRECT_ILLUMINATION, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.gBuffer[i].directIllumination.view);
		VK_AddStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_GBUFFER_DIRECT_ILLUMINATION_PREV, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_GBUFFER_DIRECT_ILLUMINATION_PREV, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.gBuffer[prevIndex].directIllumination.view);
		VK_AddStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_GBUFFER_INDIRECT_ILLUMINATION, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_GBUFFER_INDIRECT_ILLUMINATION, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.gBuffer[i].indirectIllumination.view);

		VK_AddStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_DEBUG, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageImage(&vk_d.computeDescriptor[i], BINDING_OFFSET_ASVGF_DEBUG, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.asvgf[i].debug.view);

		VK_AddStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_UBO_LIGHTS, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_SetStorageBuffer(&vk_d.computeDescriptor[i], BINDING_OFFSET_UBO_LIGHTS, VK_SHADER_STAGE_COMPUTE_BIT, vk_d.uboLightList[i].buffer);

		VK_FinishDescriptor(&vk_d.computeDescriptor[i]);
	}

	vkshader_t asvgfRngShader = { 0 };
	VK_AsvgfRngCompShader(&asvgfRngShader);
	VK_SetComputeShader(&vk_d.accelerationStructures.asvgfRngPipeline, &asvgfRngShader);
	VK_SetCompute2DescriptorSets(&vk_d.accelerationStructures.asvgfRngPipeline, &vk_d.computeDescriptor[0], &vk_d.imageDescriptor);
	VK_FinishComputePipeline(&vk_d.accelerationStructures.asvgfRngPipeline);

	vkshader_t asvgfFwdShader = { 0 };
	VK_AsvgfFwdCompShader(&asvgfFwdShader);
	VK_SetComputeShader(&vk_d.accelerationStructures.asvgfFwdPipeline, &asvgfFwdShader);
	VK_SetCompute2DescriptorSets(&vk_d.accelerationStructures.asvgfFwdPipeline, &vk_d.computeDescriptor[0], &vk_d.imageDescriptor);
	VK_FinishComputePipeline(&vk_d.accelerationStructures.asvgfFwdPipeline);

	vkshader_t asvgfGradShader = { 0 };
	VK_AsvgfGradCompShader(&asvgfGradShader);
	VK_SetComputeShader(&vk_d.accelerationStructures.asvgfGradImgPipeline, &asvgfGradShader);
	VK_SetCompute2DescriptorSets(&vk_d.accelerationStructures.asvgfGradImgPipeline, &vk_d.computeDescriptor[0], &vk_d.imageDescriptor);
	VK_FinishComputePipeline(&vk_d.accelerationStructures.asvgfGradImgPipeline);

	vkshader_t asvgfGradAtrousShader = { 0 };
	VK_AsvgfGradAtrousCompShader(&asvgfGradAtrousShader);
	VK_SetComputeShader(&vk_d.accelerationStructures.asvgfGradAtrousPipeline, &asvgfGradAtrousShader);
	VK_SetCompute2DescriptorSets(&vk_d.accelerationStructures.asvgfGradAtrousPipeline, &vk_d.computeDescriptor[0], &vk_d.imageDescriptor);
	VK_AddComputePushConstant(&vk_d.accelerationStructures.asvgfGradAtrousPipeline, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t));
	VK_FinishComputePipeline(&vk_d.accelerationStructures.asvgfGradAtrousPipeline);

	vkshader_t asvgfTemporalShader = { 0 };
	VK_AsvgfTemporalCompShader(&asvgfTemporalShader);
	VK_SetComputeShader(&vk_d.accelerationStructures.asvgfTemporalPipeline, &asvgfTemporalShader);
	VK_SetCompute2DescriptorSets(&vk_d.accelerationStructures.asvgfTemporalPipeline, &vk_d.computeDescriptor[0], &vk_d.imageDescriptor);
	VK_FinishComputePipeline(&vk_d.accelerationStructures.asvgfTemporalPipeline);

	vkshader_t asvgfTaaShader = { 0 };
	VK_AsvgfTaaCompShader(&asvgfTaaShader);
	VK_SetComputeShader(&vk_d.accelerationStructures.asvgfTaaPipeline, &asvgfTaaShader);
	VK_SetCompute2DescriptorSets(&vk_d.accelerationStructures.asvgfTaaPipeline, &vk_d.computeDescriptor[0], &vk_d.imageDescriptor);
	VK_FinishComputePipeline(&vk_d.accelerationStructures.asvgfTaaPipeline);

	vkshader_t asvgfAtrousShader = { 0 };
	VK_AsvgfAtrousCompShader(&asvgfAtrousShader);
	VK_SetComputeShader(&vk_d.accelerationStructures.asvgfAtrousPipeline, &asvgfAtrousShader);
	VK_SetCompute2DescriptorSets(&vk_d.accelerationStructures.asvgfAtrousPipeline, &vk_d.computeDescriptor[0], &vk_d.imageDescriptor);
	VK_AddComputePushConstant(&vk_d.accelerationStructures.asvgfAtrousPipeline, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t));
	VK_FinishComputePipeline(&vk_d.accelerationStructures.asvgfAtrousPipeline);

	vkshader_t compositingShader = { 0 };
	VK_CompositingCompShader(&compositingShader);
	VK_SetComputeShader(&vk_d.accelerationStructures.compositingPipeline, &compositingShader);
	VK_SetCompute2DescriptorSets(&vk_d.accelerationStructures.compositingPipeline, &vk_d.computeDescriptor[0], &vk_d.imageDescriptor);
	VK_AddComputePushConstant(&vk_d.accelerationStructures.compositingPipeline, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t));
	VK_FinishComputePipeline(&vk_d.accelerationStructures.compositingPipeline);

	/*vkshader_t s = { 0 };
	VK_RayTracingShaderWithAny(&s);
	VK_Set2RayTracingDescriptorSets(&vk_d.accelerationStructures.pipeline, &vk_d.rtxDescriptor[0], &vk_d.imageDescriptor);
	VK_SetRayTracingShader(&vk_d.accelerationStructures.pipeline, &s);
	VK_FinishRayTracingPipeline(&vk_d.accelerationStructures.pipeline);*/

	vkshader_t primaryRayShader = { 0 };
	VK_RTX_PrimaryRayShader(&primaryRayShader);
	VK_Set2RayTracingDescriptorSets(&vk_d.primaryRaysPipeline, &vk_d.rtxDescriptor[0], &vk_d.imageDescriptor);
	VK_SetRayTracingShader(&vk_d.primaryRaysPipeline, &primaryRayShader);
	//VK_AddRayTracingPushConstant(&vk_d.primaryRaysPipeline, VK_SHADER_STAGE_RAYGEN_BIT_NV, 0, 40 * sizeof(float));
	//VK_AddRayTracingPushConstant(&vk_d.primaryRaysPipeline, VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV, 40 * sizeof(float), 16 * sizeof(float));
	VK_FinishRayTracingPipeline(&vk_d.primaryRaysPipeline);

	vkshader_t reflectRaysShader = { 0 };
	VK_RTX_ReflectRaysShader(&reflectRaysShader);
	VK_Set2RayTracingDescriptorSets(&vk_d.reflectRaysPipeline, &vk_d.rtxDescriptor[0], &vk_d.imageDescriptor);
	VK_SetRayTracingShader(&vk_d.reflectRaysPipeline, &reflectRaysShader);
	VK_FinishRayTracingPipeline(&vk_d.reflectRaysPipeline);

	vkshader_t directIlluminationShader = { 0 };
	VK_RTX_DirectIlluminationShader(&directIlluminationShader);
	VK_Set2RayTracingDescriptorSets(&vk_d.directIlluminationPipeline, &vk_d.rtxDescriptor[0], &vk_d.imageDescriptor);
	VK_SetRayTracingShader(&vk_d.directIlluminationPipeline, &directIlluminationShader);
	VK_FinishRayTracingPipeline(&vk_d.directIlluminationPipeline);
	//VK_AddRayTracingPushConstant(&vk_d.primaryRaysPipeline, VK_SHADER_STAGE_RAYGEN_BIT_NV, 0, 40 * sizeof(float));
	//VK_AddRayTracingPushConstant(&vk_d.primaryRaysPipeline, VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV, 40 * sizeof(float), 16 * sizeof(float));
	//VK_FinishRayTracingPipeline(&vk_d.indirectIlluminationPipeline);

	vkshader_t indirectIlluminationShader = { 0 };
	VK_RTX_IndirectIlluminationShader(&indirectIlluminationShader);
	VK_Set2RayTracingDescriptorSets(&vk_d.indirectIlluminationPipeline, &vk_d.rtxDescriptor[0], &vk_d.imageDescriptor);
	VK_SetRayTracingShader(&vk_d.indirectIlluminationPipeline, &indirectIlluminationShader);
	VK_FinishRayTracingPipeline(&vk_d.indirectIlluminationPipeline);
}

void R_PreparePT() {
	int i;
	//tr.world->numClusters
	build_pvs2(&s_worldData);

	vk_d.numFixedCluster = s_worldData.numClusters;
	vk_d.numClusters = s_worldData.numClusters;
	vk_d.numMaxClusters = s_worldData.numClusters * 3;
	vk_d.clusterBytes = s_worldData.clusterBytes;
	vk_d.vis = calloc(vk_d.numMaxClusters, sizeof(byte) * s_worldData.clusterBytes);
	memcpy(vk_d.vis, s_worldData.vis, s_worldData.numClusters * sizeof(byte) * s_worldData.clusterBytes);
	//const byte* clusterVis = s_worldData.vis + cluster * s_worldData.clusterBytes;
	
	vk_d.clusterList = calloc(s_worldData.numClusters, sizeof(cluster_t));
	for (int i = 0; i < s_worldData.numClusters; i++) {
		vk_d.clusterList[i].idx = i;
		vk_d.clusterList[i].mins[0] = vk_d.clusterList[i].mins[1] = vk_d.clusterList[i].mins[2] = 99999;
		vk_d.clusterList[i].maxs[0] = vk_d.clusterList[i].maxs[1] = vk_d.clusterList[i].maxs[2] = -99999;
	}

	R_CalcClusterAABB(s_worldData.nodes);

	uint32_t offsetXYZ = 0;
	uint32_t offsetXYZdynamicData = 0;
	uint32_t offsetXYZdynamicAS = 0;
	uint32_t offsetIDX = 0;
	uint32_t offsetIDXdynamicData = 0;
	uint32_t offsetIDXdynamicAS = 0;

	VkDeviceSize offsetStaticWorld = 0;
	VkDeviceSize offsetDynamicDataWorld = 0;
	VkDeviceSize offsetDynamicASWorld[VK_MAX_SWAPCHAIN_SIZE] = { 0 };


	tess.numVertexes = 0;
	tess.numIndexes = 0;

	R_RecursiveCreateAS(s_worldData.nodes, &offsetIDX, &offsetXYZ, &offsetIDXdynamicData, &offsetXYZdynamicData, &offsetIDXdynamicAS, &offsetXYZdynamicAS, qfalse);

	qboolean showSun = qfalse;
	qboolean addSun = qtrue;
	if (addSun) {
		// world light
		tess.numVertexes = 0;
		tess.numIndexes = 0;
		tess.shader = tr.defaultShader;
		strcpy(tess.shader->name, "globalSun");
		vec3_t origin = { 500,500,1500 };
		vec3_t left = { 3000,0,0 };
		vec3_t up = { 0,3000,0 };
		if (strstr(s_worldData.name, "q3dm1")) {
			origin[0] = 500;
			origin[0] = 500;
			origin[1] = 2000;
			origin[2] = 1500;
			left[0] = 500;
			up[1] = 500;
		}
		tess.shader->sort = 10;
		RB_AddQuadStampExt(origin, left, up, tess.vertexColors, 0, 0, 1, 1);

		vkbuffer_t* idx_buffer;
		vkbuffer_t* xyz_buffer;
		uint32_t* idx_buffer_offset;
		uint32_t* xyz_buffer_offset;
		idx_buffer = &vk_d.geometry.idx_world_static;
		xyz_buffer = &vk_d.geometry.xyz_world_static;
		idx_buffer_offset = &vk_d.geometry.idx_world_static_offset;
		xyz_buffer_offset = &vk_d.geometry.xyz_world_static_offset;

		RB_AddLightToLightList(-2, BAS_WORLD_STATIC, (offsetIDX), 0);
		RB_UploadCluster(&vk_d.geometry.cluster_world_static, vk_d.geometry.cluster_world_static_offset, 0);
		vk_d.geometry.cluster_world_static_offset += (tess.numIndexes / 3);
		// write idx
		RB_UploadIDX(idx_buffer, (*idx_buffer_offset), (offsetXYZ));
		// write xyz
		RB_UploadXYZ(xyz_buffer, (*xyz_buffer_offset), 0);

		(*idx_buffer_offset) += tess.numIndexes;
		(*xyz_buffer_offset) += tess.numVertexes;
		if (showSun) {
			(offsetIDX) += tess.numIndexes;
			(offsetXYZ) += tess.numVertexes;
		}
	}
	tess.numVertexes = 0;
	tess.numIndexes = 0;

	// world static
	{
		vk_d.bottomASWorldStatic.geometries.sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
		vk_d.bottomASWorldStatic.geometries.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
		vk_d.bottomASWorldStatic.geometries.geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
		vk_d.bottomASWorldStatic.geometries.geometry.triangles.vertexCount = offsetXYZ;
		vk_d.bottomASWorldStatic.geometries.geometry.triangles.vertexStride = sizeof(VertexBuffer);
		vk_d.bottomASWorldStatic.geometries.geometry.triangles.indexCount = offsetIDX;
		vk_d.bottomASWorldStatic.geometries.geometry.triangles.vertexOffset = 0 * sizeof(VertexBuffer);
		vk_d.bottomASWorldStatic.geometries.geometry.triangles.indexOffset = 0 * sizeof(uint32_t);
		{
			vk_d.bottomASWorldStatic.geometries.geometry.triangles.vertexData = vk_d.geometry.xyz_world_static.buffer;
			vk_d.bottomASWorldStatic.geometries.geometry.triangles.indexData = vk_d.geometry.idx_world_static.buffer;
		}
		vk_d.bottomASWorldStatic.geometries.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
		vk_d.bottomASWorldStatic.geometries.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
		vk_d.bottomASWorldStatic.geometries.geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
		vk_d.bottomASWorldStatic.geometries.flags = 0;

		vk_d.bottomASWorldStatic.data.offsetIDX = 0;
		vk_d.bottomASWorldStatic.data.offsetXYZ = 0;

		VkCommandBuffer commandBuffer = { 0 };
		VK_BeginSingleTimeCommands(&commandBuffer);
		VK_CreateBottomAS(commandBuffer,
			&vk_d.bottomASWorldStatic, &vk_d.basBufferStaticWorld,
			&offsetStaticWorld, VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV);
		VK_EndSingleTimeCommands(&commandBuffer);

		vk_d.bottomASWorldStatic.data.type = BAS_WORLD_STATIC;
		vk_d.bottomASWorldStatic.geometryInstance.instanceCustomIndex = 0;
		vk_d.bottomASWorldStatic.geometryInstance.mask = RAY_FIRST_PERSON_MIRROR_OPAQUE_VISIBLE;
		vk_d.bottomASWorldStatic.geometryInstance.flags = VK_GEOMETRY_INSTANCE_FORCE_OPAQUE_BIT_NV;
		//vk_d.bottomASWorldStatic.geometryInstance.flags |= VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV;
		vk_d.bottomASWorldStatic.geometryInstance.accelerationStructureHandle = vk_d.bottomASWorldStatic.handle;

		float tM[12];
		tM[0] = 1; tM[1] = 0; tM[2] = 0; tM[3] = 0;
		tM[4] = 0; tM[5] = 1; tM[6] = 0; tM[7] = 0;
		tM[8] = 0; tM[9] = 0; tM[10] = 1; tM[11] = 0;
		Com_Memcpy(&vk_d.bottomASWorldStatic.geometryInstance.transform, &tM, sizeof(float[12]));
		Com_Memcpy(&vk_d.bottomASWorldStatic.data.modelmat, &tM, sizeof(float[12]));
	}
	// world dynamic data
	{
		vk_d.bottomASWorldDynamicData.geometries.sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
		vk_d.bottomASWorldDynamicData.geometries.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
		vk_d.bottomASWorldDynamicData.geometries.geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
		vk_d.bottomASWorldDynamicData.geometries.geometry.triangles.vertexCount = offsetXYZdynamicData;
		vk_d.bottomASWorldDynamicData.geometries.geometry.triangles.vertexStride = sizeof(VertexBuffer);
		vk_d.bottomASWorldDynamicData.geometries.geometry.triangles.indexCount = offsetIDXdynamicData;
		vk_d.bottomASWorldDynamicData.geometries.geometry.triangles.vertexOffset = 0 * sizeof(VertexBuffer);
		vk_d.bottomASWorldDynamicData.geometries.geometry.triangles.indexOffset = 0 * sizeof(uint32_t);
		{
			vk_d.bottomASWorldDynamicData.geometries.geometry.triangles.vertexData = vk_d.geometry.xyz_world_dynamic_data[0].buffer;
			vk_d.bottomASWorldDynamicData.geometries.geometry.triangles.indexData = vk_d.geometry.idx_world_dynamic_data[0].buffer;
		}
		vk_d.bottomASWorldDynamicData.geometries.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
		vk_d.bottomASWorldDynamicData.geometries.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
		vk_d.bottomASWorldDynamicData.geometries.geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
		vk_d.bottomASWorldDynamicData.geometries.flags = 0;

		vk_d.bottomASWorldDynamicData.data.offsetIDX = 0;
		vk_d.bottomASWorldDynamicData.data.offsetXYZ = 0;

		VkCommandBuffer commandBuffer = { 0 };
		VK_BeginSingleTimeCommands(&commandBuffer);
		VkDeviceSize offset = 0;
		VK_CreateBottomAS(commandBuffer,
			&vk_d.bottomASWorldDynamicData, &vk_d.basBufferWorldDynamicData,
			&offsetDynamicDataWorld, VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV);
		VK_EndSingleTimeCommands(&commandBuffer);

		vk_d.bottomASWorldDynamicData.data.type = BAS_WORLD_DYNAMIC_DATA;
		vk_d.bottomASWorldDynamicData.geometryInstance.instanceCustomIndex = 0;
		vk_d.bottomASWorldDynamicData.geometryInstance.mask = RAY_FIRST_PERSON_MIRROR_OPAQUE_VISIBLE;
		vk_d.bottomASWorldDynamicData.geometryInstance.flags = VK_GEOMETRY_INSTANCE_FORCE_OPAQUE_BIT_NV;
		vk_d.bottomASWorldDynamicData.geometryInstance.flags |= VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV;
		vk_d.bottomASWorldDynamicData.geometryInstance.accelerationStructureHandle = vk_d.bottomASWorldDynamicData.handle;

		float tM[12];
		tM[0] = 1; tM[1] = 0; tM[2] = 0; tM[3] = 0;
		tM[4] = 0; tM[5] = 1; tM[6] = 0; tM[7] = 0;
		tM[8] = 0; tM[9] = 0; tM[10] = 1; tM[11] = 0;
		Com_Memcpy(&vk_d.bottomASWorldDynamicData.geometryInstance.transform, &tM, sizeof(float[12]));
		Com_Memcpy(&vk_d.bottomASWorldDynamicData.data.modelmat, &tM, sizeof(float[12]));
	}
	// world dynamic as
	{
		for (int i = 0; i < vk.swapchain.imageCount; i++) {
			vk_d.bottomASWorldDynamicAS[i].geometries.sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
			vk_d.bottomASWorldDynamicAS[i].geometries.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
			vk_d.bottomASWorldDynamicAS[i].geometries.geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
			vk_d.bottomASWorldDynamicAS[i].geometries.geometry.triangles.vertexCount = offsetXYZdynamicAS;
			vk_d.bottomASWorldDynamicAS[i].geometries.geometry.triangles.vertexStride = sizeof(VertexBuffer);
			vk_d.bottomASWorldDynamicAS[i].geometries.geometry.triangles.indexCount = offsetIDXdynamicAS;
			vk_d.bottomASWorldDynamicAS[i].geometries.geometry.triangles.vertexOffset = 0 * sizeof(VertexBuffer);
			vk_d.bottomASWorldDynamicAS[i].geometries.geometry.triangles.indexOffset = 0 * sizeof(uint32_t);
			{
				vk_d.bottomASWorldDynamicAS[i].geometries.geometry.triangles.vertexData = vk_d.geometry.xyz_world_dynamic_as[i].buffer;
				vk_d.bottomASWorldDynamicAS[i].geometries.geometry.triangles.indexData = vk_d.geometry.idx_world_dynamic_as[i].buffer;
			}
			vk_d.bottomASWorldDynamicAS[i].geometries.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
			vk_d.bottomASWorldDynamicAS[i].geometries.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
			vk_d.bottomASWorldDynamicAS[i].geometries.geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
			vk_d.bottomASWorldDynamicAS[i].geometries.flags = 0;

			vk_d.bottomASWorldDynamicAS[i].data.offsetIDX = 0;
			vk_d.bottomASWorldDynamicAS[i].data.offsetXYZ = 0;

			VkCommandBuffer commandBuffer = { 0 };
			VK_BeginSingleTimeCommands(&commandBuffer);
			VkDeviceSize offset = 0;
			VK_CreateBottomAS(commandBuffer,
				&vk_d.bottomASWorldDynamicAS[i], &vk_d.basBufferWorldDynamicAS[i],
				&offsetDynamicASWorld[i], VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV);
			VK_EndSingleTimeCommands(&commandBuffer);

			vk_d.bottomASWorldDynamicAS[i].data.type = BAS_WORLD_DYNAMIC_AS;
			vk_d.bottomASWorldDynamicAS[i].geometryInstance.instanceCustomIndex = 0;
			vk_d.bottomASWorldDynamicAS[i].geometryInstance.mask = RAY_FIRST_PERSON_MIRROR_OPAQUE_VISIBLE;
			vk_d.bottomASWorldDynamicAS[i].geometryInstance.flags = VK_GEOMETRY_INSTANCE_FORCE_OPAQUE_BIT_NV;
			vk_d.bottomASWorldDynamicAS[i].geometryInstance.flags |= VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV;
			vk_d.bottomASWorldDynamicAS[i].geometryInstance.accelerationStructureHandle = vk_d.bottomASWorldDynamicAS[i].handle;

			float tM[12];
			tM[0] = 1; tM[1] = 0; tM[2] = 0; tM[3] = 0;
			tM[4] = 0; tM[5] = 1; tM[6] = 0; tM[7] = 0;
			tM[8] = 0; tM[9] = 0; tM[10] = 1; tM[11] = 0;
			Com_Memcpy(&vk_d.bottomASWorldDynamicAS[i].geometryInstance.transform, &tM, sizeof(float[12]));
			Com_Memcpy(&vk_d.bottomASWorldDynamicAS[i].data.modelmat, &tM, sizeof(float[12]));
		}
	}
	offsetIDX = 0;
	offsetXYZ = 0;
	offsetIDXdynamicData = 0;
	offsetXYZdynamicData = 0;
	offsetIDXdynamicAS = 0;
	offsetXYZdynamicAS = 0;
	
	vk_d.bottomASWorldStaticTrans.geometries.geometry.triangles.vertexOffset = vk_d.geometry.xyz_world_static_offset * sizeof(VertexBuffer);
	vk_d.bottomASWorldStaticTrans.geometries.geometry.triangles.indexOffset = vk_d.geometry.idx_world_static_offset * sizeof(uint32_t);
	vk_d.bottomASWorldStaticTrans.data.offsetXYZ = vk_d.geometry.xyz_world_static_offset;
	vk_d.bottomASWorldStaticTrans.data.offsetIDX = vk_d.geometry.idx_world_static_offset;

	vk_d.bottomASWorldDynamicDataTrans.geometries.geometry.triangles.vertexOffset = vk_d.geometry.xyz_world_dynamic_data_offset * sizeof(VertexBuffer);
	vk_d.bottomASWorldDynamicDataTrans.geometries.geometry.triangles.indexOffset = vk_d.geometry.idx_world_dynamic_data_offset * sizeof(uint32_t);
	vk_d.bottomASWorldDynamicDataTrans.data.offsetXYZ = vk_d.geometry.xyz_world_dynamic_data_offset;
	vk_d.bottomASWorldDynamicDataTrans.data.offsetIDX = vk_d.geometry.idx_world_dynamic_data_offset;

	for (int i = 0; i < vk.swapchain.imageCount; i++) {
		vk_d.bottomASWorldDynamicASTrans[i].geometries.geometry.triangles.vertexOffset = vk_d.geometry.xyz_world_dynamic_as_offset[0] * sizeof(VertexBuffer);
		vk_d.bottomASWorldDynamicASTrans[i].geometries.geometry.triangles.indexOffset = vk_d.geometry.idx_world_dynamic_as_offset[0] * sizeof(uint32_t);
		vk_d.bottomASWorldDynamicASTrans[i].data.offsetXYZ = vk_d.geometry.xyz_world_dynamic_as_offset[0];
		vk_d.bottomASWorldDynamicASTrans[i].data.offsetIDX = vk_d.geometry.idx_world_dynamic_as_offset[0];
	}
	
	R_RecursiveCreateAS(s_worldData.nodes, &offsetIDX, &offsetXYZ, &offsetIDXdynamicData, &offsetXYZdynamicData, &offsetIDXdynamicAS, &offsetXYZdynamicAS, qtrue);
	// world static trans
	{
		vk_d.bottomASWorldStaticTrans.geometries.sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
		vk_d.bottomASWorldStaticTrans.geometries.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
		vk_d.bottomASWorldStaticTrans.geometries.geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
		vk_d.bottomASWorldStaticTrans.geometries.geometry.triangles.vertexCount = offsetXYZ;
		vk_d.bottomASWorldStaticTrans.geometries.geometry.triangles.vertexStride = sizeof(VertexBuffer);
		vk_d.bottomASWorldStaticTrans.geometries.geometry.triangles.indexCount = offsetIDX;
		vk_d.bottomASWorldStaticTrans.geometries.geometry.triangles.vertexData = vk_d.geometry.xyz_world_static.buffer;
		vk_d.bottomASWorldStaticTrans.geometries.geometry.triangles.indexData = vk_d.geometry.idx_world_static.buffer;
		vk_d.bottomASWorldStaticTrans.geometries.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
		vk_d.bottomASWorldStaticTrans.geometries.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
		vk_d.bottomASWorldStaticTrans.geometries.geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
		vk_d.bottomASWorldStaticTrans.geometries.flags = 0;

		VkCommandBuffer commandBuffer = { 0 };
		VK_BeginSingleTimeCommands(&commandBuffer);
		VK_CreateBottomAS(commandBuffer,
			&vk_d.bottomASWorldStaticTrans, &vk_d.basBufferStaticWorld,
			&offsetStaticWorld, VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV);
		VK_EndSingleTimeCommands(&commandBuffer);

		vk_d.bottomASWorldStaticTrans.data.type = BAS_WORLD_STATIC;
		vk_d.bottomASWorldStaticTrans.geometryInstance.instanceCustomIndex = 0;
		vk_d.bottomASWorldStaticTrans.geometryInstance.instanceOffset = 1;
		vk_d.bottomASWorldStaticTrans.geometryInstance.mask = RAY_FIRST_PERSON_MIRROR_OPAQUE_VISIBLE;
		vk_d.bottomASWorldStaticTrans.geometryInstance.flags = VK_GEOMETRY_INSTANCE_FORCE_NO_OPAQUE_BIT_NV;
		vk_d.bottomASWorldStaticTrans.geometryInstance.flags |= VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV;
		vk_d.bottomASWorldStaticTrans.geometryInstance.accelerationStructureHandle = vk_d.bottomASWorldStaticTrans.handle;

		float tM[12];
		tM[0] = 1; tM[1] = 0; tM[2] = 0; tM[3] = 0;
		tM[4] = 0; tM[5] = 1; tM[6] = 0; tM[7] = 0;
		tM[8] = 0; tM[9] = 0; tM[10] = 1; tM[11] = 0;
		Com_Memcpy(&vk_d.bottomASWorldStaticTrans.geometryInstance.transform, &tM, sizeof(float[12]));
		Com_Memcpy(&vk_d.bottomASWorldStaticTrans.data.modelmat, &tM, sizeof(float[12]));
	}
	// world dynamic data trans
	{
		vk_d.bottomASWorldDynamicDataTrans.geometries.sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
		vk_d.bottomASWorldDynamicDataTrans.geometries.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
		vk_d.bottomASWorldDynamicDataTrans.geometries.geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
		vk_d.bottomASWorldDynamicDataTrans.geometries.geometry.triangles.vertexCount = offsetXYZdynamicData;
		vk_d.bottomASWorldDynamicDataTrans.geometries.geometry.triangles.vertexStride = sizeof(VertexBuffer);
		vk_d.bottomASWorldDynamicDataTrans.geometries.geometry.triangles.indexCount = offsetIDXdynamicData;
		{
			vk_d.bottomASWorldDynamicDataTrans.geometries.geometry.triangles.vertexData = vk_d.geometry.xyz_world_dynamic_data[0].buffer;
			vk_d.bottomASWorldDynamicDataTrans.geometries.geometry.triangles.indexData = vk_d.geometry.idx_world_dynamic_data[0].buffer;
		}
		vk_d.bottomASWorldDynamicDataTrans.geometries.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
		vk_d.bottomASWorldDynamicDataTrans.geometries.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
		vk_d.bottomASWorldDynamicDataTrans.geometries.geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
		vk_d.bottomASWorldDynamicDataTrans.geometries.flags = 0;

		VkCommandBuffer commandBuffer = { 0 };
		VK_BeginSingleTimeCommands(&commandBuffer);
		VK_CreateBottomAS(commandBuffer,
			&vk_d.bottomASWorldDynamicDataTrans, &vk_d.basBufferWorldDynamicData,
			&offsetDynamicDataWorld, VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV);
		VK_EndSingleTimeCommands(&commandBuffer);

		vk_d.bottomASWorldDynamicDataTrans.data.type = BAS_WORLD_DYNAMIC_DATA;
		vk_d.bottomASWorldDynamicDataTrans.geometryInstance.instanceCustomIndex = 0;
		vk_d.bottomASWorldDynamicDataTrans.geometryInstance.instanceOffset = 1;
		vk_d.bottomASWorldDynamicDataTrans.geometryInstance.mask = RAY_FIRST_PERSON_MIRROR_OPAQUE_VISIBLE;
		vk_d.bottomASWorldDynamicDataTrans.geometryInstance.flags = VK_GEOMETRY_INSTANCE_FORCE_NO_OPAQUE_BIT_NV;
		vk_d.bottomASWorldDynamicDataTrans.geometryInstance.flags |= VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV;
		vk_d.bottomASWorldDynamicDataTrans.geometryInstance.accelerationStructureHandle = vk_d.bottomASWorldDynamicDataTrans.handle;

		float tM[12];
		tM[0] = 1; tM[1] = 0; tM[2] = 0; tM[3] = 0;
		tM[4] = 0; tM[5] = 1; tM[6] = 0; tM[7] = 0;
		tM[8] = 0; tM[9] = 0; tM[10] = 1; tM[11] = 0;
		Com_Memcpy(&vk_d.bottomASWorldDynamicDataTrans.geometryInstance.transform, &tM, sizeof(float[12]));
		Com_Memcpy(&vk_d.bottomASWorldDynamicDataTrans.data.modelmat, &tM, sizeof(float[12]));
	}
	// world dynamic as trans
	{
		for (int i = 0; i < vk.swapchain.imageCount; i++) {
			vk_d.bottomASWorldDynamicASTrans[i].geometries.sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
			vk_d.bottomASWorldDynamicASTrans[i].geometries.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
			vk_d.bottomASWorldDynamicASTrans[i].geometries.geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
			vk_d.bottomASWorldDynamicASTrans[i].geometries.geometry.triangles.vertexCount = offsetXYZdynamicAS;
			vk_d.bottomASWorldDynamicASTrans[i].geometries.geometry.triangles.vertexStride = sizeof(VertexBuffer);
			vk_d.bottomASWorldDynamicASTrans[i].geometries.geometry.triangles.indexCount = offsetIDXdynamicAS;
			{
				vk_d.bottomASWorldDynamicASTrans[i].geometries.geometry.triangles.vertexData = vk_d.geometry.xyz_world_dynamic_as[i].buffer;
				vk_d.bottomASWorldDynamicASTrans[i].geometries.geometry.triangles.indexData = vk_d.geometry.idx_world_dynamic_as[i].buffer;
			}
			vk_d.bottomASWorldDynamicASTrans[i].geometries.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
			vk_d.bottomASWorldDynamicASTrans[i].geometries.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
			vk_d.bottomASWorldDynamicASTrans[i].geometries.geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
			vk_d.bottomASWorldDynamicASTrans[i].geometries.flags = 0;

			VkCommandBuffer commandBuffer = { 0 };
			VK_BeginSingleTimeCommands(&commandBuffer);
			VK_CreateBottomAS(commandBuffer,
				&vk_d.bottomASWorldDynamicASTrans[i], &vk_d.basBufferWorldDynamicAS[i],
				&offsetDynamicASWorld[i], VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV);
			VK_EndSingleTimeCommands(&commandBuffer);

			vk_d.bottomASWorldDynamicASTrans[i].data.type = BAS_WORLD_DYNAMIC_AS;
			vk_d.bottomASWorldDynamicASTrans[i].geometryInstance.instanceCustomIndex = 0;
			vk_d.bottomASWorldDynamicASTrans[i].geometryInstance.instanceOffset = 1;
			vk_d.bottomASWorldDynamicASTrans[i].geometryInstance.mask = RAY_FIRST_PERSON_MIRROR_OPAQUE_VISIBLE;
			vk_d.bottomASWorldDynamicASTrans[i].geometryInstance.flags = VK_GEOMETRY_INSTANCE_FORCE_NO_OPAQUE_BIT_NV;
			vk_d.bottomASWorldDynamicASTrans[i].geometryInstance.flags |= VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV;
			vk_d.bottomASWorldDynamicASTrans[i].geometryInstance.accelerationStructureHandle = vk_d.bottomASWorldDynamicASTrans[i].handle;

			float tM[12];
			tM[0] = 1; tM[1] = 0; tM[2] = 0; tM[3] = 0;
			tM[4] = 0; tM[5] = 1; tM[6] = 0; tM[7] = 0;
			tM[8] = 0; tM[9] = 0; tM[10] = 1; tM[11] = 0;
			Com_Memcpy(&vk_d.bottomASWorldDynamicASTrans[i].geometryInstance.transform, &tM, sizeof(float[12]));
			Com_Memcpy(&vk_d.bottomASWorldDynamicASTrans[i].data.modelmat, &tM, sizeof(float[12]));
		}
	}
	
	// skybox
	qboolean cmInit = qfalse;

	for (i = 0; i < s_worldData.numsurfaces; i++) {
		shader_t* shader = tr.shaders[s_worldData.surfaces[i].shader->index];
		tess.shader = shader;
		if (shader->isSky && !cmInit) {
			int		width, height;
			byte* pic;
			if (shader->sky.outerbox[0] != NULL) {
				width = shader->sky.outerbox[0]->width;
				height = shader->sky.outerbox[0]->height;
				VK_CreateCubeMap(&vk_d.accelerationStructures.envmap, width, height,
					VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1, 6);

				R_LoadImage(shader->sky.outerbox[3]->imgName, &pic, &width, &height);
				if (width == 0 || height == 0) goto skyFromStage;
				VK_UploadImageData(&vk_d.accelerationStructures.envmap, width, height, pic, 4, 0, 0); // back
				ri.Free(pic);

				R_LoadImage(shader->sky.outerbox[1]->imgName, &pic, &width, &height);
				VK_UploadImageData(&vk_d.accelerationStructures.envmap, width, height, pic, 4, 0, 1); // front
				ri.Free(pic);

				R_LoadImage(shader->sky.outerbox[4]->imgName, &pic, &width, &height);
				VK_UploadImageData(&vk_d.accelerationStructures.envmap, width, height, pic, 4, 0, 2); // bottom
				ri.Free(pic);

				R_LoadImage(shader->sky.outerbox[5]->imgName, &pic, &width, &height);
				VK_UploadImageData(&vk_d.accelerationStructures.envmap, width, height, pic, 4, 0, 3); // up
				ri.Free(pic);

				R_LoadImage(shader->sky.outerbox[0]->imgName, &pic, &width, &height);
				VK_UploadImageData(&vk_d.accelerationStructures.envmap, width, height, pic, 4, 0, 4); // right
				ri.Free(pic);

				R_LoadImage(shader->sky.outerbox[2]->imgName, &pic, &width, &height);
				VK_UploadImageData(&vk_d.accelerationStructures.envmap, width, height, pic, 4, 0, 5); // left
				ri.Free(pic);

				VK_UploadImageData(&vk_d.accelerationStructures.envmap, width, height, pic, 4, 0, 5);
			}
			else if (shader->rtstages[0] != NULL) {
			skyFromStage:
				width = shader->rtstages[0]->bundle[0].image[0]->width;
				height = shader->rtstages[0]->bundle[0].image[0]->height;
				VK_CreateCubeMap(&vk_d.accelerationStructures.envmap, width, height,
					VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1, 6);

				R_LoadImage(shader->rtstages[0]->bundle[0].image[0]->imgName/*"textures/skies/bluedimclouds.tga"*/, &pic, &width, &height);
				VK_UploadImageData(&vk_d.accelerationStructures.envmap, width, height, pic, 4, 0, 0);
				VK_UploadImageData(&vk_d.accelerationStructures.envmap, width, height, pic, 4, 0, 1);
				VK_UploadImageData(&vk_d.accelerationStructures.envmap, width, height, pic, 4, 0, 2);
				VK_UploadImageData(&vk_d.accelerationStructures.envmap, width, height, pic, 4, 0, 3);
				VK_UploadImageData(&vk_d.accelerationStructures.envmap, width, height, pic, 4, 0, 4);
				VK_UploadImageData(&vk_d.accelerationStructures.envmap, width, height, pic, 4, 0, 5);
			}
			VK_CreateSampler(&vk_d.accelerationStructures.envmap, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
			cmInit = qtrue;
			continue;
		}
		if (tess.shader->rtstages[0] == NULL) continue;

		// add brush models
		if (s_worldData.surfaces[i].bAS == NULL && !s_worldData.surfaces[i].notBrush && !s_worldData.surfaces[i].added && !s_worldData.surfaces[i].skip) {
			vk_d.scratchBufferOffset = 0;
			tess.numVertexes = 0;
			tess.numIndexes = 0;
			float originalTime = backEnd.refdef.floatTime;
			RB_BeginSurface(s_worldData.surfaces[i].shader, 0);
			backEnd.refdef.floatTime = originalTime;
			tess.shaderTime = backEnd.refdef.floatTime - tess.shader->timeOffset;

			// create bas
			rb_surfaceTable[*((surfaceType_t*)s_worldData.surfaces[i].data)]((surfaceType_t*)s_worldData.surfaces[i].data);
			RB_CreateEntityBottomAS(&s_worldData.surfaces[i].bAS, qfalse);
			s_worldData.surfaces[i].bAS->isWorldSurface = qtrue;
			s_worldData.surfaces[i].bAS->data.isBrushModel = qtrue;
			s_worldData.surfaces[i].added = qtrue;

			int clu[3] = { -1, -1, -1 };
			vec4_t pos = { 0,0,0,0 };
			for (int j = 0; j < tess.numVertexes; j++) {
				VectorAdd(pos, tess.xyz[j], pos);

				s_worldData.surfaces[i].bAS->c = R_FindClusterForPos3(tess.xyz[j]);
				if (s_worldData.surfaces[i].bAS->c != -1) break;
			}

			RB_UploadCluster(&vk_d.geometry.cluster_entity_static, s_worldData.surfaces[i].bAS->data.offsetIDX, R_GetClusterFromSurface(s_worldData.surfaces[i].data));

			backEnd.refdef.floatTime = originalTime;
			tess.numVertexes = 0;
			tess.numIndexes = 0;
			vk_d.scratchBufferOffset = 0;
		}
	}

	vk_d.scratchBufferOffset = 0;

	if (!cmInit) {
		byte black[4] = { 0,0,0,0 };
		VK_CreateCubeMap(&vk_d.accelerationStructures.envmap, 1, 1,
			VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1, 6);
		for (int skyIndex = 0; skyIndex < 5; skyIndex++) {
			VK_UploadImageData(&vk_d.accelerationStructures.envmap, 1, 1, &black, 4, 0, skyIndex);
		}
		VK_CreateSampler(&vk_d.accelerationStructures.envmap, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
	}

	// build top as
	for (i = 0; i < vk.swapchain.imageCount; i++) {
		VkCommandBuffer commandBuffer = { 0 };
		VK_BeginSingleTimeCommands(&commandBuffer);
		vk_d.scratchBufferOffset = 0;
		VK_MakeTopAS(commandBuffer, &vk_d.topAS[i], &vk_d.topASBuffer[i], vk_d.bottomASWorldStatic, 1, &vk_d.instanceBuffer[i], VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_NV);
		VK_EndSingleTimeCommands(&commandBuffer);
	}
	vk_d.scratchBufferOffset = 0;

	// lights
	for (i = 0; i < vk.swapchain.imageCount; i++) {
		VK_UploadBufferDataOffset(&vk_d.uboLightList[i], 0, sizeof(LightList_s), (void*)&vk_d.lightList);
	}

	VK_CreateImage(&vk_d.accelerationStructures.visData, s_worldData.clusterBytes, vk_d.numClusters, VK_FORMAT_R8_UINT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, 1);
	VK_UploadMipImageData(&vk_d.accelerationStructures.visData, s_worldData.clusterBytes, vk_d.numClusters, vk_d.vis, 1, 0);
	VK_TransitionImage(&vk_d.accelerationStructures.visData, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);


	// cluster rows
	// first index number lights
	uint32_t* lightVisibility = calloc(vk_d.numClusters * RTX_MAX_LIGHTS, sizeof(uint32_t));
	for (int cluster = 0; cluster < vk_d.numClusters; cluster++) {
		const byte* clusterVis = vk_d.vis + cluster * s_worldData.clusterBytes;
		uint32_t lightCount = 0;
		for (uint32_t l = 0; l < vk_d.lightList.numLights; l++) {
			//lightVisibility[cluster * RTX_MAX_LIGHTS] = 0;

			int lightCluster = vk_d.lightList.lights[l].cluster;
			if (lightCluster == -1) ri.Error(ERR_FATAL, "PT: Light cluster -1!");
			if (lightCluster == -2) {
				lightCount++;
				lightVisibility[cluster * RTX_MAX_LIGHTS + lightCount] = l;
			}
			else if ( (clusterVis[lightCluster >> 3] & (1 << (lightCluster & 7))) > 0) {
				lightCount++;
				lightVisibility[cluster * RTX_MAX_LIGHTS + lightCount] = l;
			}
			
		}
		lightVisibility[cluster * RTX_MAX_LIGHTS] = lightCount;
		if (lightCount > RTX_MAX_LIGHTS) ri.Error(ERR_FATAL, "PT: To many lights!");
	}

	VK_CreateImage(&vk_d.accelerationStructures.lightVisData, RTX_MAX_LIGHTS, vk_d.numMaxClusters, VK_FORMAT_R32_UINT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, 1);
	VK_UploadMipImageData(&vk_d.accelerationStructures.lightVisData, RTX_MAX_LIGHTS, vk_d.numClusters, &lightVisibility[0], 4, 0);
	VK_TransitionImage(&vk_d.accelerationStructures.lightVisData, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
	free(lightVisibility);

	// 2
	lightVisibility = calloc(s_worldData.numClusters * RTX_MAX_LIGHTS, sizeof(uint32_t));
	for (int cluster = 0; cluster < s_worldData.numClusters; cluster++) {
		const byte* clusterVis = s_worldData.vis2 + cluster * s_worldData.clusterBytes;
		uint32_t lightCount = 0;
		for (uint32_t l = 0; l < vk_d.lightList.numLights; l++) {
			//lightVisibility[cluster * RTX_MAX_LIGHTS] = 0;

			int lightCluster = vk_d.lightList.lights[l].cluster;
			if (lightCluster == -1) ri.Error(ERR_FATAL, "PT: Light cluster -1!");
			if (lightCluster == -2) {
				lightCount++;
				lightVisibility[cluster * RTX_MAX_LIGHTS + lightCount] = l;
			}
			else if ((clusterVis[lightCluster >> 3] & (1 << (lightCluster & 7))) > 0) {
				lightCount++;
				lightVisibility[cluster * RTX_MAX_LIGHTS + lightCount] = l;
			}
			
		}
		lightVisibility[cluster * RTX_MAX_LIGHTS] = lightCount;
		if (lightCount > RTX_MAX_LIGHTS) ri.Error(ERR_FATAL, "PT: To many lights!");
	}

	VK_CreateImage(&vk_d.accelerationStructures.lightVisData2, RTX_MAX_LIGHTS, s_worldData.numClusters, VK_FORMAT_R32_UINT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, 1);
	VK_UploadMipImageData(&vk_d.accelerationStructures.lightVisData2, RTX_MAX_LIGHTS, s_worldData.numClusters, &lightVisibility[0], 4, 0);
	VK_TransitionImage(&vk_d.accelerationStructures.lightVisData2, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
	free(lightVisibility);

	R_CreatePrimaryRaysPipeline();
	vk_d.worldASInit = qtrue;
}


/*
=================
RE_LoadWorldMap

Called directly from cgame
=================
*/
void RE_LoadWorldMap( const char *name ) {
	int			i;
	dheader_t	*header;
	byte		*buffer;
	byte		*startMarker;

	if ( tr.worldMapLoaded ) {
		ri.Error( ERR_DROP, "ERROR: attempted to redundantly load world map\n" );
	}

	// set default sun direction to be used if it isn't
	// overridden by a shader
	tr.sunDirection[0] = 0.45f;
	tr.sunDirection[1] = 0.3f;
	tr.sunDirection[2] = 0.9f;

	VectorNormalize( tr.sunDirection );

	tr.worldMapLoaded = qtrue;

	// load it
    ri.FS_ReadFile( name, (void **)&buffer );
	if ( !buffer ) {
		ri.Error (ERR_DROP, "RE_LoadWorldMap: %s not found", name);
	}

	// clear tr.world so if the level fails to load, the next
	// try will not look at the partially loaded version
	tr.world = NULL;

	Com_Memset( &s_worldData, 0, sizeof( s_worldData ) );
	Q_strncpyz( s_worldData.name, name, sizeof( s_worldData.name ) );

	Q_strncpyz( s_worldData.baseName, COM_SkipPath( s_worldData.name ), sizeof( s_worldData.name ) );
	COM_StripExtension( s_worldData.baseName, s_worldData.baseName );

	startMarker = ri.Hunk_Alloc(0, h_low);
	c_gridVerts = 0;

	header = (dheader_t *)buffer;
	fileBase = (byte *)header;

	i = LittleLong (header->version);
	if ( i != BSP_VERSION ) {
		ri.Error (ERR_DROP, "RE_LoadWorldMap: %s has wrong version number (%i should be %i)", 
			name, i, BSP_VERSION);
	}

	// swap all the lumps
	for (i=0 ; i<sizeof(dheader_t)/4 ; i++) {
		((int *)header)[i] = LittleLong ( ((int *)header)[i]);
	}

	// load into heap
	R_LoadShaders( &header->lumps[LUMP_SHADERS] );
	R_LoadLightmaps( &header->lumps[LUMP_LIGHTMAPS] );
	R_LoadPlanes (&header->lumps[LUMP_PLANES]);
	R_LoadFogs( &header->lumps[LUMP_FOGS], &header->lumps[LUMP_BRUSHES], &header->lumps[LUMP_BRUSHSIDES] );
	R_LoadSurfaces( &header->lumps[LUMP_SURFACES], &header->lumps[LUMP_DRAWVERTS], &header->lumps[LUMP_DRAWINDEXES] );
	R_LoadMarksurfaces (&header->lumps[LUMP_LEAFSURFACES]);
	R_LoadNodesAndLeafs (&header->lumps[LUMP_NODES], &header->lumps[LUMP_LEAFS]);
	R_LoadSubmodels (&header->lumps[LUMP_MODELS]);
	R_LoadVisibility( &header->lumps[LUMP_VISIBILITY] );
	R_LoadEntities( &header->lumps[LUMP_ENTITIES] );
	R_LoadLightGrid( &header->lumps[LUMP_LIGHTGRID] );

	// RTX
	if (glConfig.driverType == VULKAN && r_vertexLight->value == 2) {
		R_PreparePT();
	}


	s_worldData.dataSize = (byte *)ri.Hunk_Alloc(0, h_low) - startMarker;

	// only set tr.world now that we know the entire level has loaded properly
	tr.world = &s_worldData;


    ri.FS_FreeFile( buffer );
}
