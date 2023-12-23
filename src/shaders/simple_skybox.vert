#version 450
#extension GL_GOOGLE_include_directive : require
#include "../common/common_definitions.h"

layout(set = 0, binding = 0) uniform CameraData {
	CameraVectors cameraData;
};

layout(set = 0, binding = 1) uniform RenderData {
	RenderParams renderParams;
};

layout(location = 0) out vec3 forwards;

const vec2 screen_corners[6] = vec2[](
	vec2(-1.f, -1.f),
	vec2(-1.f,  1.f),
	vec2( 1.f,  1.f),
	vec2( 1.f,  1.f),
	vec2( 1.f, -1.f),
	vec2(-1.f, -1.f)
);

void main()
{
	vec2 pos = screen_corners[gl_VertexIndex];
	gl_Position = vec4(pos, 0.f, 1.f);
	forwards = normalize(cameraData.forwards + pos.x * cameraData.right - 1.f / renderParams.aspectRatio * pos.y * cameraData.up).xyz;
}