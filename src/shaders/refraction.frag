#version 450
#extension GL_GOOGLE_include_directive : require
#include "../common/common_definitions.h"

layout(location = 0) in vec3 rayDirection;

layout(set = 0, binding = 0) uniform CameraData {
	CameraVectors cameraData;
};

layout(set = 0, binding = 1) uniform RenderData {
	RenderParams renderParams;
};

layout(set = 1, binding = 0) uniform samplerCube material;

layout(location = 0) out vec4 outColor;


#define MAX_STEPS 100
#define MAX_DIST 100.f
#define SURF_DIST 0.001f
#define GAMMA 2.2f
#define M_PI 3.1415926535897932384626433832795f
#define UP vec3(0.f, 1.f, 0.f)


const float R0 = (IOR - 1.f) * (IOR - 1.f) / ((IOR + 1.f) * (IOR + 1.f));
const float SPHERE_RADIUS = 1.f;

float sd_box(vec3 p, vec3 s)
{
  p = abs(p) - s;
	return length(max(p, 0.f)) + min(max(p.x, max(p.y, p.z)), 0.f);
}


float sd_sphere( vec3 p, float s )
{
  return length(p) - s;
}


float sd_cylinder(vec3 p, vec3 a, vec3 b, float r)
{
  vec3 pa = p - a;
  vec3 ba = b - a;
  float baba = dot(ba, ba);
  float paba = dot(pa, ba);

  float x = length(pa * baba - ba * paba) - r * baba;
  float y = abs(paba - baba * 0.5f) - baba * 0.5f;
  float x2 = x * x;
  float y2 = y * y * baba;
  float d = (max(x, y) < 0.f) ? -min(x2, y2) : (((x > 0.f) ? x2 : 0.f) + ((y > 0.f) ? y2 : 0.f));
  return sign(d) * sqrt(abs(d)) / baba;
}


float sd_cone(vec3 p, vec2 c, float h)
{
  vec2 q = h*vec2(c.x/c.y, -1.f);    
  vec2 w = vec2( length(p.xz), p.y );
  vec2 a = w - q*clamp( dot(w,q)/dot(q,q), 0.f, 1.f );
  vec2 b = w - q*vec2( clamp( w.x/q.x, 0.f, 1.f ), 1.f );
  float k = sign( q.y );
  float d = min(dot( a, a ),dot(b, b));
  float s = max( k*(w.x*q.y-w.y*q.x),k*(w.y-q.y)  );
  return sqrt(d)*sign(s);
}


float sd_box_frame(vec3 p, vec3 b, float e)
{
  p = abs(p) - b;
  vec3 q = abs(p + e) - e;
  return min(min(
    length(max(vec3(p.x, q.y, q.z), 0.f)) + min(max(p.x, max(q.y, q.z)), 0.f),
    length(max(vec3(q.x, p.y, q.z), 0.f)) + min(max(q.x, max(p.y, q.z)), 0.f)),
    length(max(vec3(q.x, q.y, p.z), 0.f)) + min(max(q.x, max(q.y, p.z)), 0.f));
}


float get_dist(vec3 p)
{
  // float d = sd_box(p, vec3(1.f));
  float d = sd_sphere(p, SPHERE_RADIUS);
  // float d = sd_cylinder(p, vec3(-0.2, -0.2, -0.f), vec3(0.f, 0.2, 0.2), 0.25);
  // float d = sd_cylinder(p, vec3(-0.f, -0.2, -0.f), vec3(0.f, 0.2, 0.f), 0.25);
  // float d = sd_cone(p - vec3(0.f, 0.5f, 0.f), vec2(sin(3.14f / 6.f), cos(3.14f / 6.f)), 1.f);
  
  return d;
}


float ray_march(vec3 ro, vec3 rd, float side)
{
	float dO = 0.f;
    
  for(int i = 0; i < MAX_STEPS; i++)
  {
    vec3 p = ro + rd * dO;
    float dS = get_dist(p)*side;
    dO += dS;
    if(dO > MAX_DIST || abs(dS) < SURF_DIST)
      break;
  }
  return dO;
}


vec3 get_normal(vec3 p)
{
	float d = get_dist(p);
  vec2 e = vec2(0.001f, 0.f);
  
  vec3 n = d - vec3(
    get_dist(p - e.xyy),
    get_dist(p - e.yxy),
    get_dist(p - e.yyx)
  );  
  return normalize(n);
}


vec3 sample_cubemap_linear_space(vec3 rd)
{
  if (dot(rd, rd) != 0.f)
    // Inverse gamma correction to sample light in linear brightness space
    return pow(texture(material, rd).rgb, vec3(GAMMA));
  return vec3(0.f);
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
  vec3 color = sample_cubemap_linear_space(rayDirection);

  if (renderParams.distanceCalculationMode == 1)
  {
    float dist = ray_march(cameraData.position.xzy, rayDirection, 1.f); // outside of object
    
    if (dist < MAX_DIST)
    {
      color = vec3(0.f);

      vec3 pos = cameraData.position.xzy + rayDirection * dist; // 3d hit position
      vec3 normal = get_normal(pos); // surface normal orientation

      vec3 reflectedRayDirection = reflect(rayDirection, normal);
      vec3 colorReflected = sample_cubemap_linear_space(reflectedRayDirection);

      float R = get_fresnel_factor(dot(-rayDirection, normal));
      float T = 1.f - R;
      color += R * colorReflected;
      
      vec3 inRayDirection = refract_safe(rayDirection, normal, 1.f/IOR); // ray direction when entering
      
      vec3 enterPoint = pos - normal * SURF_DIST * 3.f;

      float distanceInside = ray_march(enterPoint, inRayDirection, -1.f); // inside the object
      vec3 exitPoint = enterPoint + inRayDirection * distanceInside; // 3d position of exit
      vec3 exitNormal = -get_normal(exitPoint);
      vec3 outRayDirection = refract_safe(inRayDirection, exitNormal, IOR);

      vec3 colorRefracted = sample_cubemap_linear_space(outRayDirection);
      color += T * colorRefracted;
    }
  }

  // Gamma correction
  color = pow(color, vec3(1.f / GAMMA));
  outColor = vec4(color, 1.f);
}