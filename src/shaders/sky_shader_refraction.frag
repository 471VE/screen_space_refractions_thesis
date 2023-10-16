#version 450

layout(location = 0) in vec3 rayDirection;

layout(set = 0, binding = 0) uniform CameraVectors {
	vec4 forwards;
	vec4 right;
	vec4 up;
	vec4 position;
} cameraData;

layout(set = 1, binding = 0) uniform samplerCube material;

layout(location = 0) out vec4 outColor;

	
#define MAX_STEPS 100
#define MAX_DIST 10.f
#define SURF_DIST 0.001f
#define MAX_TIR_COUNT 5


float sdBox(vec3 p, vec3 s) {
  p = abs(p) - s;
	return length(max(p, 0.f)) + min(max(p.x, max(p.y, p.z)), 0.f);
}

float sdSphere( vec3 p, float s )
{
  return length(p) - s;
}

float sdCylinder(vec3 p, vec3 a, vec3 b, float r)
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

float sdCone(vec3 p, vec2 c, float h)
{
  vec2 q = h*vec2(c.x/c.y,-1.0);
    
  vec2 w = vec2( length(p.xz), p.y );
  vec2 a = w - q*clamp( dot(w,q)/dot(q,q), 0.0, 1.0 );
  vec2 b = w - q*vec2( clamp( w.x/q.x, 0.0, 1.0 ), 1.0 );
  float k = sign( q.y );
  float d = min(dot( a, a ),dot(b, b));
  float s = max( k*(w.x*q.y-w.y*q.x),k*(w.y-q.y)  );
  return sqrt(d)*sign(s);
}

float sdBoxFrame(vec3 p, vec3 b, float e)
{
  p = abs(p) - b;
  vec3 q = abs(p + e) - e;
  return min(min(
    length(max(vec3(p.x, q.y, q.z), 0.f)) + min(max(p.x, max(q.y, q.z)), 0.f),
    length(max(vec3(q.x, p.y, q.z), 0.f)) + min(max(q.x, max(p.y, q.z)), 0.f)),
    length(max(vec3(q.x, q.y, p.z), 0.f)) + min(max(q.x, max(q.y, p.z)), 0.f));
}

float GetDist(vec3 p) {
  float d = sdBox(p, vec3(0.5));
  // float d = sdSphere(p, 0.5);
  // float d = sdCylinder(p, vec3(-0.2, -0.2, -0.0), vec3(0.0, 0.2, 0.2), 0.25);
  // float d = sdCylinder(p, vec3(-0.0, -0.2, -0.0), vec3(0.0, 0.2, 0.0), 0.25);
  // float d = sdCone(p - vec3(0.f, 0.5f, 0.f), vec2(sin(3.14f / 9.f), cos(3.14f / 9.f)), 1.f);
  // float d = sdBoxFrame(p, vec3(0.5f), 0.05f);
  
  return d;
}

float RayMarch(vec3 ro, vec3 rd, float side) {
	float dO = 0.f;
    
  for(int i = 0; i < MAX_STEPS; i++) {
    vec3 p = ro + rd * dO;
    float dS = GetDist(p)*side;
    dO += dS;
    if(dO > MAX_DIST || abs(dS) < SURF_DIST)
      break;
  }
  
  return dO;
}

vec3 GetNormal(vec3 p) {
	float d = GetDist(p);
  vec2 e = vec2(0.001f, 0.f);
  
  vec3 n = d - vec3(
    GetDist(p - e.xyy),
    GetDist(p - e.yxy),
    GetDist(p - e.yyx)
  );
  
  return normalize(n);
}

void main()
{
  vec3 color = texture(material, rayDirection).rgb;   
  float d = RayMarch(cameraData.position.xyz, rayDirection, 1.f); // outside of object
  
  float IOR = 1.45f; // index of refraction
  
  if (d < MAX_DIST)
  {
    float distanceInside = 0.f;

    vec3 p = cameraData.position.xyz + rayDirection * d; // 3d hit position
    vec3 n = GetNormal(p); // normal of surface... orientation
    
    vec3 rdIn = refract(rayDirection, n, 1.f/IOR); // ray dir when entering
    
    vec3 pEnter = p - n * SURF_DIST * 3.f;
    float dIn = RayMarch(pEnter, rdIn, -1.f); // inside the object    
    distanceInside += dIn;

    vec3 pExit = pEnter + rdIn * dIn; // 3d position of exit
    vec3 nExit = -GetNormal(pExit); 
    
    vec3 rdOut = refract(rdIn, nExit, IOR);

    bool totalInternalReflection = dot(rdOut, rdOut) == 0.f;
    if (totalInternalReflection)
    {
      for (int i = 0; i < MAX_TIR_COUNT; i++)
      {
        if (totalInternalReflection)
        {
          rdOut = reflect(rdIn, nExit);
          dIn = RayMarch(pExit, rdIn, -1.f);
          distanceInside += dIn;
          pExit += rdOut * dIn;
          nExit = -GetNormal(pExit);
          rdOut = refract(rdIn, nExit, IOR);
        }
        else
          break;
        totalInternalReflection = dot(rdOut, rdOut) == 0.f;
      }
      if (totalInternalReflection)
        rdOut = reflect(rdIn, n);
        // rdOut = vec3(0.f);
    }
    
    color = texture(material, rdOut).rgb;
  }  
  outColor = vec4(color, 1.f);
}