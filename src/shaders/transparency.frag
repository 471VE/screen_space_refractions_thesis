#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in float width;

layout(set = 1, binding = 0) uniform sampler2D material;

layout(location = 0) out vec4 outColor;

void main()
{
	outColor = vec4(width, width, width, 1.f);
}
