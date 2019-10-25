
hitAttributeNV vec2 hitAttribute;
vec3 getBarycentricCoordinates()
{
	return vec3(1.0f - hitAttribute.x - hitAttribute.y, hitAttribute.x, hitAttribute.y);
}

