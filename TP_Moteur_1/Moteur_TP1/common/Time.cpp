#include "Time.hpp"

float Time::DeltaTime = 0.0f;
float Time::m_LastFrameTime = 0.0f;

void Time::intialize() {
    m_LastFrameTime = (float)glfwGetTime();
}

void Time::Update() {
    float currentFrame = (float)glfwGetTime();

    // 计算两帧时间差
    DeltaTime = currentFrame - m_LastFrameTime;

    // 更新上一帧时间
    m_LastFrameTime = currentFrame;
}