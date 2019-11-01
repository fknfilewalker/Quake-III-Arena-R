struct RayPayload {
  vec4 color;
  vec4 normal;
  uint blendFunc;
  uint transparent;
  float distance;
  bool miss;
  uint cullMask;
  uint depth;
};