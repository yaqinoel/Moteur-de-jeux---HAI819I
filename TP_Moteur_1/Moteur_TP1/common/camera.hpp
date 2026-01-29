#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "Time.hpp"

// 定义相机移动方向的枚举
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// 默认相机参数常量
const float YAW         = -90.0f;
const float PITCH       =  0.0f;
const float SPEED       =  2.5f;
const float SENSITIVITY =  0.1f;
const float ZOOM        =  45.0f;
const float ASPECT      =  4.0f / 3.0f;
const float ZNEAR       =  0.1f;
const float ZFAR        =  100.f;

class Camera
{
public:
    // 相机属性
    glm::vec3 m_Position;
    glm::vec3 m_Front;
    glm::vec3 m_Up;
    glm::vec3 m_Right;
    glm::vec3 m_WorldUp;

    // 欧拉角
    float m_Yaw;
    float m_Pitch;

    // 相机选项
    float m_MovementSpeed;
    float m_MouseSensitivity;
    float m_Zoom;
    float m_Aspect;
    float m_ZNear;
    float m_ZFar;

    // 构造函数声明
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);

    // 标量构造函数声明
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectiveMatrix() const;
    void ProcessKeyboard(Camera_Movement direction);
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
    void ProcessMouseScroll(float yoffset);

private:
    void updateCameraVectors();
};

#endif