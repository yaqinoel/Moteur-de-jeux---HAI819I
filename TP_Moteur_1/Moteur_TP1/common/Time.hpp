#ifndef TIME_HPP
#define TIME_HPP

#include <GLFW/glfw3.h> // 需要用到 glfwGetTime()

class Time {
public:
    // 静态变量：全项目共享这一份数据
    static float DeltaTime;

    // 静态函数：每一帧调用一次
    static void Update();

    static void intialize();

private:
    static float m_LastFrameTime;
};

#endif  //TIME_HPP