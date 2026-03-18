#include "camera.hpp"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

#include "SceneNode.hpp"


Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch, float roll)
    : m_Front(glm::vec3(0.0f, 0.0f, -1.0f)), m_MovementSpeed(SPEED), m_MouseSensitivity(SENSITIVITY), m_Zoom(ZOOM), m_Aspect(ASPECT), m_ZNear(ZNEAR), m_ZFar(ZFAR)
{
    m_Position = position;

    m_WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    m_WorldFront = glm::vec3(0.0f, 0.0f, -1.0f);
    m_WorldRight = glm::vec3(1.0f, 0.0f, 0.0f);

    glm::quat qYaw = glm::angleAxis(glm::radians(yaw), m_WorldUp);
    glm::quat qPitch = glm::angleAxis(glm::radians(pitch), m_WorldRight);

    m_Orientation = glm::normalize(qYaw * qPitch);

    updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() const
{
    if (m_IsOrbital) {
        return glm::lookAt(m_Position, m_Target, m_WorldUp);
    } else {
        return glm::lookAt(m_Position, m_Position + m_Front, m_Up);
    }
}

glm::mat4 Camera::GetProjectiveMatrix() const {
    return glm::perspective(glm::radians(m_Zoom),m_Aspect, m_ZNear,m_ZFar);
}

void Camera::ProcessKeyboard(Camera_Movement direction) {
    float velocity = m_MovementSpeed * Time::DeltaTime;
    if (direction == Camera_Movement::FORWARD)
        m_Position += m_Front * velocity;
    if (direction == Camera_Movement::BACKWARD)
        m_Position -= m_Front * velocity;
    if (direction == Camera_Movement::LEFT)
        m_Position -= m_Right * velocity;
    if (direction == Camera_Movement::RIGHT)
        m_Position += m_Right * velocity;
    if (direction == Camera_Movement::UP)
        m_Position += m_WorldUp * velocity;
    if (direction == Camera_Movement::DOWN)
        m_Position -= m_WorldUp * velocity;
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset) {
    xoffset *= m_MouseSensitivity;
    yoffset *= m_MouseSensitivity;

    // 水平旋转：绕世界垂直轴旋转
    glm::quat qYaw = glm::angleAxis(glm::radians(-xoffset), m_WorldUp);

    // 垂直旋转：绕相机当前的右轴(m_Right)旋转
    glm::quat qPitch = glm::angleAxis(glm::radians(yoffset), m_WorldRight);

    // 更新四元数：全局在左，局部在右
    m_Orientation = glm::normalize(qYaw * m_Orientation * qPitch);

    updateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset)
{
    m_Zoom -= (float)yoffset;
    if (m_Zoom < 1.0f)
        m_Zoom = 1.0f;
    if (m_Zoom > 45.0f)
        m_Zoom = 45.0f;
}

void Camera::updateCameraVectors() {
    // 通过四元数旋转原始向量来得到新的方向
    m_Front = m_Orientation * m_WorldFront;
    m_Up    = m_Orientation * m_WorldUp;
    m_Right = m_Orientation * m_WorldRight;
    m_eulerAngle = glm::degrees(glm::eulerAngles(m_Orientation));
}

// 轨道模式
void Camera::EnableOrbitalMode(glm::vec3 targetPoint, float radius, float angle) {
    m_IsOrbital = true;
    m_Target = targetPoint;
    float clampedAngle = std::clamp(angle, -89.0f, 89.0f);
    m_angle = glm::radians(clampedAngle);
    m_OrbitalRadius = radius;
}

void Camera::EnableOrbitalMode(SceneNode* targetNode, float distanceMultiplier, float elevationAngle) {
    m_IsOrbital = true;
    m_OrbitalTarget = targetNode;
    m_DistanceMultiplier = distanceMultiplier;
    m_angle = elevationAngle;
    // m_OrbitalAngle = 0.0f;
}

// 关闭轨道模式
void Camera::DisableOrbitalMode() {
    m_IsOrbital = false;
    m_OrbitalTarget = nullptr;

    // 更新回四元数的旋转逻辑
    glm::mat4 viewMatrix = glm::lookAt(m_Position, m_Target, m_WorldUp);
    glm::mat3 rotationMatrix = glm::transpose(glm::mat3(viewMatrix));
    m_Orientation = glm::quat_cast(rotationMatrix);
    m_Orientation = glm::normalize(m_Orientation);

    // 更新相应的方向向量
    updateCameraVectors();
}

// 更新轨道位置
void Camera::UpdateOrbital(float deltaTime) {
    if (!m_IsOrbital || !m_OrbitalTarget) return;

    const glm::mat4& targetWorldMat = m_OrbitalTarget->GetWorldMatrix();
    glm::vec3 currentTargetPos = glm::vec3(targetWorldMat[3]);

    float targetScale = glm::length(glm::vec3(targetWorldMat[0]));
    float dynamicRadius = targetScale * m_DistanceMultiplier;

    m_OrbitalAngle += m_OrbitalSpeed * deltaTime;

    float angleRad = glm::radians(m_angle);

    float camX = sin(m_OrbitalAngle) * dynamicRadius;
    float camZ = cos(m_OrbitalAngle) * dynamicRadius;
    float camY = dynamicRadius * tan(angleRad);

    m_Position = currentTargetPos + glm::vec3(camX, camY, camZ);

    m_Front = glm::normalize(currentTargetPos - m_Position);
    m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
    m_Up    = glm::normalize(glm::cross(m_Right, m_Front));
}

// 改变轨道速度
void Camera::ChangeOrbitalSpeed(float delta) {
    m_OrbitalSpeed += delta;
}

void Camera::EnableFollowMode(SceneNode* targetNode, float distance, float height) {
    m_IsFollowing = true;
    m_IsOrbital = false; // 确保关闭其他模式
    m_FollowTarget = targetNode;
    m_FollowDistance = distance;
    m_FollowHeight = height;
}

// 关闭跟随模式
void Camera::DisableFollowMode() {
    m_IsFollowing = false;
    m_FollowTarget = nullptr;
}

// 更新跟随相机的位置和朝向
void Camera::UpdateFollow(float deltaTime) {
    if (!m_IsFollowing || !m_FollowTarget) return;

    // 获取玩家节点的世界矩阵
    const glm::mat4& targetWorldMat = m_FollowTarget->GetWorldMatrix();

    // 提取玩家位置
    glm::vec3 playerPos = glm::vec3(targetWorldMat[3]);

    // 提取玩家的前方向量
    glm::vec3 playerFront = -glm::normalize(glm::vec3(targetWorldMat[2]));

    // 计算相机的理想位置：玩家位置 - 玩家前方 * 距离 + 向上高度
    glm::vec3 cameraPos = playerPos - (playerFront * m_FollowDistance) + glm::vec3(0.0f, m_FollowHeight, 0.0f);

    // 更新相机的实际位置
    m_Position = cameraPos;

    // 让相机看向玩家
    glm::vec3 lookAtTarget = playerPos + glm::vec3(0.0f, 1.0f, 0.0f);

    m_Front = glm::normalize(lookAtTarget - m_Position);
    m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
    m_Up    = glm::normalize(glm::cross(m_Right, m_Front));
}