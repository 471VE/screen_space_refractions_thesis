#ifndef COMMON_DEFINITIONS_H
#define COMMON_DEFINITIONS_H

// GLSL-C++ datatype compatibility layer

#ifdef __cplusplus

#include <glm/glm.hpp>

using shader_uint  = uint32_t;
using shader_uvec2 = glm::uvec2;
using shader_uvec3 = glm::uvec3;

using shader_float = float;
using shader_vec2  = glm::vec2;
using shader_vec3  = glm::vec3;
using shader_vec4  = glm::vec4;
using shader_mat4  = glm::mat4;

// On a GPU, you might consider a single byte to be 32 bits, because nothing can be smaller
// than 32 bits, so a bool has to be 32 bits as well.
using shader_bool  = uint32_t;

#else

#define shader_uint  uint
#define shader_uvec2 uvec2

#define shader_float float
#define shader_vec2  vec2
#define shader_vec3  vec3
#define shader_vec4  vec4
#define shader_mat4  mat4

#define shader_bool  bool

#endif


#define IOR 1.45f // index of refraction

struct RenderParams
{
  shader_float aspectRatio;
  shader_uint distanceCalculationMode;
};

struct CameraVectors
{
	shader_vec4 forwards;
	shader_vec4 right;
	shader_vec4 up;
	shader_vec4 position;
};

struct CameraMatrices
{
  shader_mat4 view;
  shader_mat4 projection;
  shader_mat4 viewProjection;
};

#endif // COMMON_DEFINITIONS_H