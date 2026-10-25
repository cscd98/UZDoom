#pragma once
#include "v_video.h"

class SystemBaseFrameBuffer : public DFrameBuffer
{
public:
    SystemBaseFrameBuffer(void *, bool);
    bool IsFullscreen() override;
    int GetClientWidth() override;
    int GetClientHeight() override;
    void ToggleFullscreen(bool) override;
    void SetWindowSize(int width, int height) override;

protected:
    SystemBaseFrameBuffer() = default;
};

class SystemGLFrameBuffer : public SystemBaseFrameBuffer
{
public:
    SystemGLFrameBuffer(void *, bool);
    ~SystemGLFrameBuffer();
    int GetClientWidth() override;
    int GetClientHeight() override;
    void SetVSync(bool) override;
    void SwapBuffers();

protected:
    SystemGLFrameBuffer() = default;
};
