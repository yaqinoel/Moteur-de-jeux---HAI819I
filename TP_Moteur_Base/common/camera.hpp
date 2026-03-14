#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Time.hpp"

// 定义相机移动方向的枚举
enum Camera_Movement { FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN };

// 默认相机参数常量
const float YAW = 0.0f;
const float PITCH = -45.0f;
const float ROLL = 0.0f;
const float SPEED = 50.0f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;
const float ASPECT = 4.0f / 3.0f;
const float ZNEAR = 0.1f;
const float ZFAR = 1000.f;

class Camera {
public:
  // 相机属性
  glm::vec3 m_Position;
  glm::vec3 m_Front;
  glm::vec3 m_Up;
  glm::vec3 m_Right;
  glm::vec3 m_WorldUp;
  glm::vec3 m_WorldFront;
  glm::vec3 m_WorldRight;

  // 欧拉角
  glm::vec3 m_eulerAngle;
  float m_Yaw;
  float m_Pitch;
  float m_Roll;

  // 四元数相机旋转
  glm::quat m_Orientation;

  // 相机选项
  float m_MovementSpeed;
  float m_MouseSensitivity;
  float m_Zoom;
  float m_Aspect;
  float m_ZNear;
  float m_ZFar;

  // 轨道模式参数
  bool m_IsOrbital = false;
  float m_OrbitalAngle = 0.0f;
  float m_OrbitalRadius = 2.0f;
  float m_OrbitalSpeed = 0.5f;
  float m_angle = 45.f;
  glm::vec3 m_Target;

  // 构造函数声明
  Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
         glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW,
         float pitch = PITCH, float roll = ROLL);

  glm::mat4 GetViewMatrix() const;

  glm::mat4 GetProjectiveMatrix() const;

  void ProcessKeyboard(Camera_Movement direction);

  void ProcessMouseMovement(float xoffset, float yoffset);

  void ProcessMouseScroll(float yoffset);

  // 轨道模式
  void EnableOrbitalMode(glm::vec3 targetPoint, float radius, float angle);

  // 关闭轨道模式
  void DisableOrbitalMode();

  // 更新轨道
  void UpdateOrbital(float deltaTime);

  // 改变轨道速度
  void ChangeOrbitalSpeed(float delta);

private:
  void updateCameraVectors();
};

#endif
