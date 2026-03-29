#include "camera.hpp"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

#include "SceneNode.hpp"


Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch, float roll)
    : m_Front(glm::vec3(0.0f, 0.0f, -1.0f)), m_MovementSpeed(SPEED), m_MouseSensitivity(SENSITIVITY), m_Zoom(ZOOM), m_Aspect(ASPECT), m_ZNear(ZNEAR), m_ZFar(ZFAR)
{
    m_Position = position;
    m_WorldUp = up;
    m_WorldFront = glm::vec3(0.0f, 0.0f, -1.0f);
    m_WorldRight = glm::vec3(1.0f, 0.0f, 0.0f);

    // 直接初始化 vec3
    m_eulerAngle = glm::vec3(pitch, yaw, 0.0f);

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

    // 鼠标水平滑动改变 Yaw (Y分量)，垂直滑动改变 Pitch (X分量)
    m_eulerAngle.y += xoffset;
    m_eulerAngle.x += yoffset;

    // 调用更新
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
    if (m_eulerAngle.x > 89.0f)  m_eulerAngle.x = 89.0f;
    if (m_eulerAngle.x < -89.0f) m_eulerAngle.x = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(m_eulerAngle.y)) * cos(glm::radians(m_eulerAngle.x));
    front.y = sin(glm::radians(m_eulerAngle.x));
    front.z = sin(glm::radians(m_eulerAngle.y)) * cos(glm::radians(m_eulerAngle.x));
    m_Front = glm::normalize(front);

    m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
    m_Up    = glm::normalize(glm::cross(m_Right, m_Front));

    glm::mat3 rotMat(m_Right, m_Up, -m_Front);
    m_Orientation = glm::quat_cast(rotMat);
}

void Camera::updateCameraVectorsByEulerAngle() {
    m_Pitch = m_eulerAngle.x;
    m_Yaw   = m_eulerAngle.y;

    if (m_Pitch > 89.0f) m_Pitch = 89.0f;
    if (m_Pitch < -89.0f) m_Pitch = -89.0f;
    m_eulerAngle.x = m_Pitch; // 将限制后的值写回 UI

    glm::vec3 front;
    front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    front.y = sin(glm::radians(m_Pitch));
    front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    m_Front = glm::normalize(front);

    m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
    m_Up    = glm::normalize(glm::cross(m_Right, m_Front));

    glm::mat3 rotMat(m_Right, m_Up, -m_Front);
    m_Orientation = glm::quat_cast(rotMat);
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

void Camera::SetupInput(InputManager &inputManager) {
    inputManager.BindHold("FreeMode", GLFW_KEY_W, [this]() { this->ProcessKeyboard(Camera_Movement::FORWARD); });
    inputManager.BindHold("FreeMode", GLFW_KEY_S, [this]() { this->ProcessKeyboard(Camera_Movement::BACKWARD); });
    inputManager.BindHold("FreeMode", GLFW_KEY_A, [this]() { this->ProcessKeyboard(Camera_Movement::LEFT); });
    inputManager.BindHold("FreeMode", GLFW_KEY_D, [this]() { this->ProcessKeyboard(Camera_Movement::RIGHT); });
    inputManager.BindHold("FreeMode", GLFW_KEY_Q, [this]() { this->ProcessKeyboard(Camera_Movement::UP); });
    inputManager.BindHold("FreeMode", GLFW_KEY_E, [this]() { this->ProcessKeyboard(Camera_Movement::DOWN); });

    inputManager.BindPress("OrbitalMode", GLFW_KEY_UP, [this]() { this->ChangeOrbitalSpeed(0.1f); });
    inputManager.BindPress("OrbitalMode", GLFW_KEY_DOWN, [this]() { this->ChangeOrbitalSpeed(-0.1f); });
}
