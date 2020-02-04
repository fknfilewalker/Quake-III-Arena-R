layout(binding = BINDING_OFFSET_GBUFFER_POS, set = 0, rgba32f) uniform image2D posGBuffer;
layout(binding = BINDING_OFFSET_GBUFFER_ALBEDO, set = 0, rgba32f) uniform image2D albedoGBuffer;
layout(binding = BINDING_OFFSET_GBUFFER_NORMAL, set = 0, rgba32f) uniform image2D normalGBuffer;
layout(binding = BINDING_OFFSET_GBUFFER_MATERIAL, set = 0, rgba32ui) uniform uimage2D materialGBuffer;
layout(binding = BINDING_OFFSET_GBUFFER_MOTION, set = 0, rgba32f) uniform image2D motionGBuffer;