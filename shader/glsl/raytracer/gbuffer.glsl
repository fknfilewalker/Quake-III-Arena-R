layout(binding = BINDING_OFFSET_GBUFFER_POS, set = 0, rgba32f) uniform image2D posGBuffer;
layout(binding = BINDING_OFFSET_GBUFFER_ALBEDO, set = 0, rgba32f) uniform image2D albedoGBuffer;
layout(binding = BINDING_OFFSET_GBUFFER_NORMAL, set = 0, rgba32f) uniform image2D normalGBuffer;
layout(binding = BINDING_OFFSET_GBUFFER_REFLECTION, set = 0, rgba32f) uniform image2D reflectionGBuffer;
layout(binding = BINDING_OFFSET_GBUFFER_DIRECT_ILLUMINATION, set = 0, rgba16f) uniform image2D directIlluminationGBuffer;
layout(binding = BINDING_OFFSET_GBUFFER_OBJECT, set = 0, rgba32f) uniform image2D objectGBuffer;
layout(binding = BINDING_OFFSET_GBUFFER_MOTION, set = 0, rgba32f) uniform image2D motionGBuffer;
layout(binding = BINDING_OFFSET_GBUFFER_VIEW_DIR, set = 0, rgba32f) uniform image2D viewGBuffer;
layout(binding = BINDING_OFFSET_GBUFFER_TRANSPARENT, set = 0, rgba32f) uniform image2D transparentGBuffer;

layout(binding = BINDING_OFFSET_GBUFFER_TEX_GRAD_0, set = 0, rgba16f) uniform image2D texGrad0GBuffer;
layout(binding = BINDING_OFFSET_GBUFFER_TEX_GRAD_1, set = 0, rgba16f) uniform image2D texGrad1GBuffer;
layout(binding = BINDING_OFFSET_GBUFFER_TEX_GRAD_2, set = 0, rgba16f) uniform image2D texGrad2GBuffer;
layout(binding = BINDING_OFFSET_GBUFFER_TEX_GRAD_3, set = 0, rgba16f) uniform image2D texGrad3GBuffer;

layout(binding = BINDING_OFFSET_GBUFFER_OBJECT_PREV, set = 0, rgba32f) uniform image2D objectGBufferPrev;
layout(binding = BINDING_OFFSET_GBUFFER_DIRECT_ILLUMINATION_PREV, set = 0, rgba16f) uniform image2D directIlluminationGBufferPrev;