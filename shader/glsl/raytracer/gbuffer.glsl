layout(binding = BINDING_OFFSET_GBUFFER_POS, set = 0, rgba32f) uniform image2D posGBuffer;
layout(binding = BINDING_OFFSET_GBUFFER_ALBEDO, set = 0, rgba32f) uniform image2D albedoGBuffer;
layout(binding = BINDING_OFFSET_GBUFFER_NORMAL, set = 0, rgba32f) uniform image2D normalGBuffer;
layout(binding = BINDING_OFFSET_GBUFFER_REFLECTION, set = 0, rgba32f) uniform image2D reflectionGBuffer;
layout(binding = BINDING_OFFSET_GBUFFER_OBJECT, set = 0, rgba32ui) uniform uimage2D objectGBuffer;
layout(binding = BINDING_OFFSET_GBUFFER_MOTION, set = 0, rgba32f) uniform image2D motionGBuffer;
layout(binding = BINDING_OFFSET_GBUFFER_VIEW_DIR, set = 0, rgba32f) uniform image2D viewGBuffer;