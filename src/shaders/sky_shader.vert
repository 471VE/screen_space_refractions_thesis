#version 450

layout(set = 0, binding = 0) uniform CameraVectors {
	vec4 forwards;
	vec4 right;
	vec4 up;
	vec4 position;
} cameraData;

layout(set = 0, binding = 1) uniform RenderParams {
	float aspectRatio;
	uint distanceCalculationMode;
} renderParams;

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
	// real aspect ratio should be passed here, not hardcoded constant!!!
	forwards = normalize(cameraData.forwards + pos.x * cameraData.right - 1.f / renderParams.aspectRatio * pos.y * cameraData.up).xyz;
}