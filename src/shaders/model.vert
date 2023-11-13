#version 450
#extension GL_GOOGLE_include_directive : require
#include "../common/common_definitions.h"

layout(set = 0, binding = 0) uniform UBO {
	CameraMatrices cameraData;
};

layout(std140, set = 0, binding = 1) readonly buffer storageBuffer {
	mat4 model[];
} ObjectData;

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in vec2 vertexTexCoord;
layout(location = 3) in vec3 vertexNormal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragNormal;

void main()
{
	gl_Position = cameraData.viewProjection * ObjectData.model[gl_InstanceIndex] * vec4(vertexPosition, 1.f);
	fragColor = vertexColor;
	fragTexCoord = vertexTexCoord;
	fragNormal = normalize((ObjectData.model[gl_InstanceIndex] * vec4(vertexNormal, 0.f)).xyz);
}