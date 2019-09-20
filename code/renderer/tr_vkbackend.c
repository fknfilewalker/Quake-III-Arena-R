
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

static void VK_Cull(int cullType) {

	if (cullType == CT_TWO_SIDED)
	{
		vk_d.state.cullMode = VK_CULL_MODE_NONE;
		//qglDisable( GL_CULL_FACE );
	}
	else
	{
		//qglEnable( GL_CULL_FACE );

		if (cullType == CT_FRONT_SIDED)
		{
			if (backEnd.viewParms.isMirror)
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
			if (backEnd.viewParms.isMirror)
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

static float	s_flipMatrix[16] = {
	// convert from our coordinate system (looking down X)
	// to OpenGL's coordinate system (looking down -Z)
	0, 0, -1, 0,
	-1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 0, 1
};
static void R_DrawElements( int numIndexes, const glIndex_t *indexes ) {
    int        primitives;
    
    primitives = r_primitives->integer;

    VK_UploadBufferDataOffset(&vk_d.indexbuffer, vk_d.offsetIdx * sizeof(uint32_t), numIndexes * sizeof(uint32_t), (void*) &indexes[0]);
    
	//if (backEnd.viewParms.isPortal) return;
    //int aaa = pStage->bundle[0].image[0]->index;
	//if (qtrue /*&& backEnd.viewParms.isPortal*/) {
	//	//ri.Printf(PRINT_ALL, "%f %f %f\n", backEnd.viewParms.or.origin[0],
	//	//	backEnd.viewParms. or .origin[1],
	//	//	backEnd.viewParms. or .origin[2]);
	//	if (vk_d.accelerationStructures.init && vk_d.drawMirror) {
	//		float invView[16];
	//		float invProj[16];

	//		float	viewerMatrix[16];
	//		float	viewerMatrix2[16];
	//		vec3_t	origin;

	//		VectorCopy(backEnd.viewParms. or .origin, origin);

	//		viewerMatrix[0] = backEnd.viewParms. or .axis[0][0];
	//		viewerMatrix[4] = backEnd.viewParms. or .axis[0][1];
	//		viewerMatrix[8] = backEnd.viewParms. or .axis[0][2];
	//		viewerMatrix[12] = -origin[0] * viewerMatrix[0] + -origin[1] * viewerMatrix[4] + -origin[2] * viewerMatrix[8];

	//		viewerMatrix[1] = backEnd.viewParms. or .axis[1][0];
	//		viewerMatrix[5] = backEnd.viewParms. or .axis[1][1];
	//		viewerMatrix[9] = backEnd.viewParms. or .axis[1][2];
	//		viewerMatrix[13] = -origin[0] * viewerMatrix[1] + -origin[1] * viewerMatrix[5] + -origin[2] * viewerMatrix[9];

	//		viewerMatrix[2] = backEnd.viewParms. or .axis[2][0];
	//		viewerMatrix[6] = backEnd.viewParms. or .axis[2][1];
	//		viewerMatrix[10] = backEnd.viewParms. or .axis[2][2];
	//		viewerMatrix[14] = -origin[0] * viewerMatrix[2] + -origin[1] * viewerMatrix[6] + -origin[2] * viewerMatrix[10];

	//		viewerMatrix[3] = 0;
	//		viewerMatrix[7] = 0;
	//		viewerMatrix[11] = 0;
	//		viewerMatrix[15] = 1;

	//		myGlMultMatrix(viewerMatrix, s_flipMatrix, viewerMatrix2);

	//		myGLInvertMatrix(&viewerMatrix2, &invView);
	//		myGLInvertMatrix(&vk_d.projectionMatrix, &invProj);


	//		VK_BindRayTracingPipeline(&vk_d.accelerationStructures.pipeline);
	//		VK_Bind2RayTracingDescriptorSets(&vk_d.accelerationStructures.pipeline, &vk_d.accelerationStructures.descriptor, &vk_d.imageDescriptor);

	//		float pos[3];
	//		pos[0] = backEnd.viewParms. or .origin[0];// -650;
	//		pos[1] = backEnd.viewParms. or .origin[1];//630;
	//		pos[2] = backEnd.viewParms. or .origin[2];//140;
	//		////tr.viewParms.or.origin;
	//		float direction[3];
	//		direction[0] = 1;// -650;
	//		direction[1] = 0;//630;
	//		direction[2] = 0;//140;
	//		////tr. or .modelMatrix
	//		//	//ri.Printf(PRINT_ALL, "%f %f %f\n", tr. or .modelMatrix[3],
	//		//		//tr. or .modelMatrix[7],
	//		//		//tr. or .modelMatrix[11]);
	//		VK_SetRayTracingPushConstant(&vk_d.accelerationStructures.pipeline, VK_SHADER_STAGE_RAYGEN_BIT_NV, 0 * sizeof(float), 16 * sizeof(float), &invView[0]);
	//		VK_SetRayTracingPushConstant(&vk_d.accelerationStructures.pipeline, VK_SHADER_STAGE_RAYGEN_BIT_NV, 16 * sizeof(float), 16 * sizeof(float), &invProj[0]);
	//		VK_SetRayTracingPushConstant(&vk_d.accelerationStructures.pipeline, VK_SHADER_STAGE_RAYGEN_BIT_NV, 32 * sizeof(float), sizeof(vec3_t), &origin);
	//		VK_SetRayTracingPushConstant(&vk_d.accelerationStructures.pipeline, VK_SHADER_STAGE_RAYGEN_BIT_NV, 36 * sizeof(float), sizeof(vec3_t), &backEnd.viewParms.portalPlane.normal);
	//		
	//		//VK_SetRayTracingPushConstant(&vk_d.accelerationStructures.pipeline, VK_SHADER_STAGE_RAYGEN_BIT_NV, 4 * sizeof(float), sizeof(vec3_t), &direction);
	//		VK_TraceRays();
	//		//VK_CopyImageToSwapchain(&vk_d.accelerationStructures.resultImage);
	//		vk_d.drawMirror = qfalse;
	//	}
	//	//return;
	// }
	

    //vkimage_t image = {0};
    
    //uint8_t data[4] = { 255,255,0,125 };
    //VK_CreateImage(&image, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, 1);
    //VK_UploadImageData(&image, 1, 1, data, 4, 0); // rows wise
    //VK_CreateSampler(&image, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
    
    //vkdescriptor_t d = { 0 };
    //VK_AddSampler(&d, 0, VK_SHADER_STAGE_FRAGMENT_BIT);
    //VK_SetSampler(&d, 0, VK_SHADER_STAGE_FRAGMENT_BIT, image.sampler, image.view);
    //VK_FinishDescriptor(&d);
	//VK_SetUpdateSize(&vk_d.imageDescriptor, 0, VK_SHADER_STAGE_FRAGMENT_BIT, tr.numImages);
	//VK_UpdateDescriptorSet(&vk_d.imageDescriptor);
	
	uint32_t index = VK_FindPipeline();
	vkpipeline_t p = { 0 };
    if (index == -1) {
        
        vkshader_t s = { 0 };
		if (vk_d.state.mul == qtrue) {
			if (vk_d.state.clip == qtrue) {
				VK_MultiTextureMulClipShader(&s);
			}
			else {
				VK_MultiTextureMulShader(&s);
			}
		}
		else if (vk_d.state.add == qtrue) {
			if (vk_d.state.clip == qtrue) {
				VK_MultiTextureAddClipShader(&s);
			}
			else {
				VK_MultiTextureAddShader(&s);
			}
		}
		else {
			if (vk_d.state.clip == qtrue) {
				VK_SingleTextureClipShader(&s);
			}
			else {
				VK_SingleTextureShader(&s);
			}
		}
		if (vk_d.state.add || vk_d.state.mul) {
			VK_SetDescriptorSet(&p, &vk_d.imageDescriptor);
			//VK_Set2DescriptorSets(&p, &vk_d.images[vk_d.currentTexture[0]].descriptor_set, &vk_d.images[vk_d.currentTexture[1]].descriptor_set);
		}
		else {
			VK_SetDescriptorSet(&p, &vk_d.imageDescriptor);
			//VK_SetDescriptorSet(&p, &vk_d.images[vk_d.currentTexture[0]].descriptor_set);
		}
        VK_SetShader(&p, &s);
        VK_AddBindingDescription(&p, 0, sizeof(vec4_t), VK_VERTEX_INPUT_RATE_VERTEX);
        VK_AddBindingDescription(&p, 1, sizeof(color4ub_t), VK_VERTEX_INPUT_RATE_VERTEX);
        VK_AddBindingDescription(&p, 2, sizeof(vec2_t), VK_VERTEX_INPUT_RATE_VERTEX);
        VK_AddAttributeDescription(&p, 0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 0 * sizeof(float));
        VK_AddAttributeDescription(&p, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, 0 * sizeof(float));
        VK_AddAttributeDescription(&p, 2, 2, VK_FORMAT_R32G32_SFLOAT, 0 * sizeof(float));
		if (vk_d.state.add || vk_d.state.mul) {
			VK_AddBindingDescription(&p, 3, sizeof(vec2_t), VK_VERTEX_INPUT_RATE_VERTEX);
			VK_AddAttributeDescription(&p, 3, 3, VK_FORMAT_R32G32_SFLOAT, 0 * sizeof(float));
		}

        VK_AddPushConstant(&p, VK_SHADER_STAGE_VERTEX_BIT, 0, 192);//sizeof(vk_d.mvp));
        VK_AddPushConstant(&p, VK_SHADER_STAGE_FRAGMENT_BIT, 192, 3 * sizeof(int));
        VK_FinishPipeline(&p);
        index = VK_AddPipeline(&p);

    }
	p = vk_d.pipelineList[index].pipeline;
    //        VK_BindAttribBuffer(&vk_d.vertexbuffer, 0, 0);//vk_d.offset * sizeof(vec4_t));
    //        VK_BindAttribBuffer(&vk_d.colorbuffer, 1, 0);//vk_d.offset * sizeof(color4ub_t));
    //        VK_BindAttribBuffer(&vk_d.uvbuffer, 2, 0);//vk_d.offset * sizeof(vec2_t));
	if (vk_d.state.add || vk_d.state.mul) {
		//VK_Bind2DescriptorSets(&p, &vk_d.images[vk_d.currentTexture[0]].descriptor_set, &vk_d.images[vk_d.currentTexture[1]].descriptor_set);
		VK_Bind1DescriptorSet(&p, &vk_d.imageDescriptor);
	}
	else {
		VK_Bind1DescriptorSet(&p, &vk_d.imageDescriptor);
		//VK_Bind1DescriptorSet(&p, &vk_d.images[vk_d.currentTexture[0]].descriptor_set);
	}
    
    VK_SetPushConstant(&p, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(vk_d.mvp), &vk_d.mvp);
    VK_SetPushConstant(&p, VK_SHADER_STAGE_FRAGMENT_BIT, 192, sizeof(uint32_t), &vk_d.discardModeAlpha);
	VK_SetPushConstant(&p, VK_SHADER_STAGE_FRAGMENT_BIT, 196, sizeof(int), &vk_d.currentTexture[0]);
	VK_SetPushConstant(&p, VK_SHADER_STAGE_FRAGMENT_BIT, 200, sizeof(int), &vk_d.currentTexture[1]);

    if (vk_d.state.clip == qtrue) {
        VK_SetPushConstant(&p, VK_SHADER_STAGE_VERTEX_BIT, 64, sizeof(vk_d.modelViewMatrix), &vk_d.modelViewMatrix);
        VK_SetPushConstant(&p, VK_SHADER_STAGE_VERTEX_BIT, 128, sizeof(vk_d.clipPlane), &vk_d.clipPlane);
    }
	VK_BindPipeline(&p);
	//if (index != vk_d.currentPipeline) VK_BindPipeline(&p);

    VK_DrawIndexed(&vk_d.indexbuffer, numIndexes, vk_d.offsetIdx, vk_d.offset);

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
	api->Cull = VK_Cull;
	api->State = State;
	api->SetViewportAndScissor = SetViewportAndScissor;
	api->RB_Set2D = RB_Set2D;
    api->R_DrawElements = R_DrawElements;
}
