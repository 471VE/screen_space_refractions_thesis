#version 450
#extension GL_GOOGLE_include_directive : require
#include "../common/common_definitions.h"

layout(set = 0, binding = 0) uniform UBO {
	CameraMatrices cameraMatrices;
};

layout(set = 0, binding = 1) uniform CameraData {
	CameraVectors cameraVectors;
};

layout(std140, set = 0, binding = 2) readonly buffer storageBuffer {
	mat4 model[];
} ObjectData;

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in vec2 vertexTexCoord;
layout(location = 3) in vec3 vertexNormal;

layout(location = 4) in vec3 sphCoeffsWidth1;
layout(location = 5) in vec3 sphCoeffsWidth2;
layout(location = 6) in vec3 sphCoeffsWidth3;

layout(location = 7) in vec3 sphCoeffsX1;
layout(location = 8) in vec3 sphCoeffsX2;
layout(location = 9) in vec3 sphCoeffsX3;

layout(location = 10) in vec3 sphCoeffsY1;
layout(location = 11) in vec3 sphCoeffsY2;
layout(location = 12) in vec3 sphCoeffsY3;

layout(location = 13) in vec3 sphCoeffsZ1;
layout(location = 14) in vec3 sphCoeffsZ2;
layout(location = 15) in vec3 sphCoeffsZ3;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out float width;
layout(location = 4) out vec3 refractedVector;
layout(location = 5) out vec3 reflectedVector;
layout(location = 6) out float fresnelFactor;

#define UP vec3(0.f, 1.f, 0.f)

const float R0 = (IOR - 1.f) * (IOR - 1.f) / ((IOR + 1.f) * (IOR + 1.f));

// Implementation of spherical harmonics.
// Note that constant coefficients are already accounted for in expansion terms.
// A VERY IMPORTANT NOTE: notice how z-axis is UP direction.
float Y00 (vec3 dir) { return 1.f; }

float Y1m1(vec3 dir) { return dir.y; }
float Y10 (vec3 dir) { return dir.z; }
float Y11 (vec3 dir) { return dir.x; }

float Y2m2(vec3 dir) { return dir.x * dir.y; }
float Y2m1(vec3 dir) { return dir.y * dir.z; }
float Y20 (vec3 dir) { return 3.f * dir.z * dir.z - 1.f; }
float Y21 (vec3 dir) { return dir.x * dir.z; }
float Y22 (vec3 dir) { return dir.x * dir.x - dir.y * dir.y; }

float reconstruct_from_sh(vec3 rd, vec3 n, vec3 sphCoeffs1, vec3 sphCoeffs2, vec3 sphCoeffs3)
{
  // Constructing right-handed orthonormal basis.
  // Again, look at how z-axis is UP direction in the local coordinate system, and not y-direction.
  // This was done to preserve the common spherical harmonics definition for the sake of confusion avoidance.
  vec3 x_axis = (abs(dot(UP, n)) == 1.f) ? vec3(1.f, 0.f, 0.f) : cross(UP, n);
  vec3 y_axis = cross(n, x_axis);
  mat3 transform = mat3(x_axis, y_axis, n);
  
  // Here we go from object reference frame to vertex reference frame
  vec3 localDirection = rd * transform;
  return sphCoeffs1.x * Y00 (localDirection)

       + sphCoeffs1.y * Y1m1(localDirection)
       + sphCoeffs1.z * Y10 (localDirection)
       + sphCoeffs2.x * Y11 (localDirection)

       + sphCoeffs2.y * Y2m2(localDirection)
       + sphCoeffs2.z * Y2m1(localDirection)
       + sphCoeffs3.x * Y20 (localDirection)
       + sphCoeffs3.y * Y21 (localDirection)
       + sphCoeffs3.z * Y22 (localDirection);
}

float pow5(float x)
{
  float y = x * x;
  return x * y * y;
}

float get_fresnel_factor(float cosTheta)
{
  // Schlick's approximation for reflective Fresnel factor on an interface between two insulators.
  // This clamp BS is needed only for ray marching. Remove when proper ray tracing is implemented.
  return R0 + (1.f - R0) * pow5(1.f - clamp(cosTheta, 0.f, 1.f));
}

vec3 refract_safe(vec3 I, vec3 N, float eta)
{
  vec3 R = refract(I, N, eta);
  return dot(R, R) != 0.f ? normalize(R) : R;
}

void main()
{
  vec4 currentVertexPos = ObjectData.model[gl_InstanceIndex] * vec4(vertexPosition, 1.f);
	gl_Position = cameraMatrices.viewProjection * currentVertexPos;
	fragColor = vertexColor;
	fragTexCoord = vertexTexCoord;
	fragNormal = normalize((ObjectData.model[gl_InstanceIndex] * vec4(vertexNormal, 0.f)).xyz);
	vec3 rayDirection = normalize(currentVertexPos.xyz - cameraVectors.position.xyz);

  // We swith Y and Z-coordinates here to avoid many more calculations in fragment shader:
  reflectedVector = reflect(rayDirection, fragNormal).xzy;
  fresnelFactor = get_fresnel_factor(dot(-rayDirection, fragNormal));

	vec3 inRayDirection = refract_safe(rayDirection, fragNormal, 1.f / IOR);
	width = reconstruct_from_sh(inRayDirection, -fragNormal, sphCoeffsWidth1, sphCoeffsWidth2, sphCoeffsWidth3);

  // We swith Y and Z-coordinates here to avoid many more calculations in fragment shader:
  refractedVector.x = reconstruct_from_sh(inRayDirection, -fragNormal, sphCoeffsX1, sphCoeffsX2, sphCoeffsX3);
  refractedVector.z = reconstruct_from_sh(inRayDirection, -fragNormal, sphCoeffsY1, sphCoeffsY2, sphCoeffsY3);
  refractedVector.y = reconstruct_from_sh(inRayDirection, -fragNormal, sphCoeffsZ1, sphCoeffsZ2, sphCoeffsZ3);
}
