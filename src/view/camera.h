#pragma once
#include <tuple>
#include "../config.h"

class Camera
{
  public: 
    Camera();

    void moveForwards();
    void moveBackwards();
    void moveRight();
    void moveLeft();
    void moveUp();
    void moveDown();

    void move(float deltaTime);

    glm::mat4 getViewMat();
    void getTransformationMatrixColumns(glm::vec4& forwards, glm::vec4& right, glm::vec4& up, glm::vec4& pos);

    void resetSpeedVector();

  private:
    glm::vec3 camPos;
    glm::vec3 camLookAt;
    glm::vec3 camUp;

    float camSpeed = 0.01f;
    glm::vec3 camSpeedVector = { 0.f, 0.f, 0.f };

  private:
    bool shouldMove();
};
