#ifndef TIME_HPP
#define TIME_HPP

class Time {
public:
    // 静态变量：全项目共享这一份数据
    static float CurrentTime;
    static float DeltaTime;
    static int FPS;

    // 静态函数：每一帧调用一次
    static void Update();

    static void intialize();

private:
    static float m_LastFrameTime;
    static float m_LastFPSTime;
    static int m_FrameCount;

};

#endif  //TIME_HPP