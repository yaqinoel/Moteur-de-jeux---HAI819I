#include "Time.hpp"
// Include GLFW
#include <GLFW/glfw3.h>

float Time::CurrentTime = 0.0f;
float Time::DeltaTime = 0.0f;
float Time::m_LastFrameTime = 0.0f;
float Time::m_LastFPSTime = 0.0f;
int Time::m_FrameCount = 0;
int Time::FPS = 0;

void Time::intialize() {
    m_LastFrameTime = (float)glfwGetTime();
}

void Time::Update() {
    float currentFrame = (float)glfwGetTime();
    CurrentTime = currentFrame;

    // 计算两帧时间差
    DeltaTime = currentFrame - m_LastFrameTime;

    // 更新上一帧时间
    m_LastFrameTime = currentFrame;

    m_FrameCount++;

    // 更新FPS
    float timeSinceLastFPS = currentFrame - m_LastFPSTime;
    if (timeSinceLastFPS >= 0.25f) {
        FPS = (int)(m_FrameCount / timeSinceLastFPS);

        m_LastFPSTime = currentFrame;
        m_FrameCount = 0;
    }

}

