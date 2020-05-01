layout(binding = BINDING_OFFSET_GBUFFER_POS, set = 0, rgba32f) uniform image2D posGBuffer;
layout(binding = BINDING_OFFSET_GBUFFER_ALBEDO, set = 0, rgba32f) uniform image2D albedoGBuffer;
layout(binding = BINDING_OFFSET_GBUFFER_NORMAL, set = 0, rgba32f) uniform image2D normalGBuffer;
layout(binding = BINDING_OFFSET_GBUFFER_REFLECTION, set = 0, rgba32f) uniform image2D reflectionGBuffer;
layout(binding = BINDING_OFFSET_GBUFFER_DIRECT_ILLUMINATION, set = 0, rgba16f) uniform image2D directIlluminationGBuffer;
layout(binding = BINDING_OFFSET_GBUFFER_OBJECT, set = 0, rgba32f) uniform image2D objectGBuffer;
layout(binding = BINDING_OFFSET_GBUFFER_MOTION, set = 0, rgba16f) uniform image2D motionGBuffer;
layout(binding = BINDING_OFFSET_GBUFFER_VIEW_DIR, set = 0, rgba32f) uniform image2D viewGBuffer;
layout(binding = BINDING_OFFSET_GBUFFER_TRANSPARENT, set = 0, rgba32f) uniform image2D transparentGBuffer;

layout(binding = BINDING_OFFSET_GBUFFER_COLOR, set = 0, rgba16f) uniform image2D IMG_GBUFFER_COLOR;
layout(binding = BINDING_OFFSET_GBUFFER_COLOR_PREV, set = 0, rgba16f) uniform image2D IMG_GBUFFER_COLOR_PREV;

layout(binding = BINDING_OFFSET_GBUFFER_OBJECT_PREV, set = 0, rgba32f) uniform image2D objectGBufferPrev;
layout(binding = BINDING_OFFSET_GBUFFER_DIRECT_ILLUMINATION_PREV, set = 0, rgba16f) uniform image2D directIlluminationGBufferPrev;
layout(binding = BINDING_OFFSET_GBUFFER_NORMAL_PREV, set = 0, rgba32f) uniform image2D normalGBufferPrev;
layout(binding = BINDING_OFFSET_GBUFFER_VIEW_DIR_PREV, set = 0, rgba32f) uniform image2D viewGBufferPrev;

layout(binding = BINDING_OFFSET_GBUFFER_DEPTH_NORMAL, set = 0, rg32f) uniform image2D IMG_GBUFFER_DEPTH_NORMAL;