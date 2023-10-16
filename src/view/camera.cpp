#include "camera.h"

Camera::Camera()
  : camPos(0.f, -5.f, 0.f)
  , camLookAt(0.f, 0.f, 0.f)
  , camUp(0.f, 0.f, 1.f)
{}

void Camera::moveForwards() { camSpeedVector.z += 1.f; }

void Camera::moveBackwards() { camSpeedVector.z -= 1.f; }

void Camera::moveRight() { camSpeedVector.x += 1.f; }

void Camera::moveLeft() { camSpeedVector.x -= 1.f; }

void Camera::moveUp() { camSpeedVector.y += 1.f; }

void Camera::moveDown() { camSpeedVector.y -= 1.f; }

static int sign(float x)
{
  return x > 0.f ? 1 : (x < 0.f ? -1 : 0);
}

void Camera::move(float deltaTime)
{
  if (!shouldMove())
    return;

  camSpeedVector = glm::normalize(camSpeedVector);

  glm::vec3 forwardsVector = camLookAt - camPos;
  // To not go through the object
  if (glm::length(forwardsVector) > 1.f || camSpeedVector.z < 0.f)
  {
    forwardsVector = glm::normalize(forwardsVector);
    camPos += camSpeed * camSpeedVector.z * deltaTime * forwardsVector;
  }

  float forwardsVectorLength = glm::length(camLookAt - camPos);

  glm::vec3 rightVector = glm::normalize(glm::cross(forwardsVector, camUp));
  camPos += camSpeed * camSpeedVector.x * deltaTime * rightVector;

  // A weird roundabout way to prevent camera from getting away when rotating around the "look at" point
  glm::vec3 newForwardsVector = glm::normalize(camLookAt - camPos);
  camPos = camLookAt - newForwardsVector * forwardsVectorLength;

  glm::vec3 upVector = glm::normalize(glm::cross(rightVector, forwardsVector));
  // To prevent constant switching of sides at the poles
  if (abs(glm::dot(upVector, camUp)) > 0.05f || sign(camSpeedVector.y) == sign(camLookAt.z - camPos.z)) 
    camPos += camSpeed * camSpeedVector.y * deltaTime * upVector;
}

glm::mat4 Camera::getViewMat() { return glm::lookAt(camPos, camLookAt, camUp); }

void Camera::getTransformationMatrixColumns(glm::vec4& forwards, glm::vec4& right, glm::vec4& up, glm::vec4& pos)
{
  glm::vec3 forwardsVector = glm::normalize(camLookAt - camPos);
  glm::vec3 rightVector = glm::normalize(glm::cross(forwardsVector, camUp));
  glm::vec3 upVector = glm::normalize(glm::cross(rightVector, forwardsVector));

  // Don't forget that Y-coordinate (not Z) is UP direction!
  forwards = {forwardsVector.x, forwardsVector.z, forwardsVector.y, 0.f};
  right = {rightVector.x, rightVector.z, rightVector.y, 0.f};
  up = {upVector.x, upVector.z, upVector.y, 0.f};
  pos = {camPos.x, camPos.z, camPos.y, 0.f};
}

void Camera::resetSpeedVector() { camSpeedVector = { 0.f, 0.f, 0.f }; }

bool Camera::shouldMove() { return (camSpeedVector.x || camSpeedVector.y || camSpeedVector.z); }