#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;

layout(set = 1, binding = 0) uniform sampler2D material;

layout(location = 0) out vec4 outColor;

const vec4 sunColor = vec4(0.9f, 0.95f, 1.f, 1.f);
const vec3 sunDirection = normalize(vec3(1.f, -1.f, -1.f));

void main() {
	outColor = sunColor * max(0.f, dot(fragNormal, -sunDirection)) * vec4(fragColor, 1.f) * texture(material, fragTexCoord);
}