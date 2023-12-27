#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in float width;
layout(location = 4) in vec3 refractedVector;
layout(location = 5) in vec3 reflectedVector;
layout(location = 6) in float fresnelFactor;

layout(set = 1, binding = 0) uniform samplerCube material;

layout(location = 0) out vec4 outColor;

void main()
{
	outColor = fresnelFactor * texture(material, reflectedVector);
	// if (dot(refractedVector, refractedVector) > 0.2f)
		outColor += (1.f - fresnelFactor) * texture(material, normalize(refractedVector));
}
