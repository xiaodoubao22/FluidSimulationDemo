#ifndef RENDER_WIDGET_H
#define RENDER_WIDGET_H


#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <chrono>
#include <vector>
#include "Shader.h"
#include <glm/glm.hpp>
#include "ParticalSystem.h"

class RenderWidget
{
public:
    RenderWidget();
    ~RenderWidget();

    int32_t Init();

    void Update();

    int32_t Destroy();

    bool ShouldClose();

    void ProcessInput();

    void PollEvents();

    void LoadVertexes(Fluid2d::ParticalSystem& ps);

private:
    bool CreateWindow();

    float CalculateFPS();

    static void ResizeCallback(GLFWwindow* window, int width, int height);

private:

    GLFWwindow* mWindow = nullptr;
    int mWindowWidth = 1000;
    int mWindowHeight = 1000;

    Shader* mParticalShader = nullptr;
    Shader* mSdfShader = nullptr;
    Shader* mMilkShader = nullptr;

    GLuint mVaoParticals = 0;
    GLuint mPositionBuffer = 0;
    GLuint mDensityBuffer = 0;

    GLuint mFboSdf = 0;
    GLuint mTextureSdf = 0;
    GLuint mRboSdf = 0;

    size_t mParticalNum = 0;

    std::chrono::system_clock::time_point mUpdateTime;
    

};


#endif // !RENDER_WIDGET_H

