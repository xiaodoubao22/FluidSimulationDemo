#include "RenderWidget.h"


#include <iostream>


RenderWidget::RenderWidget() {

}

RenderWidget::~RenderWidget() {
    Destroy();
}

int32_t RenderWidget::Init() {
    if (!CreateWindow()) {
        return -1;
    }
    
    std::string particalVertShaderPath = "../code/Fluid2d/Shaders/DrawParticals.vert";
    std::string particalFragShaderPath = "../code/Fluid2d/Shaders/DrawParticals.frag";
    mParticalShader = new Shader();
    mParticalShader->BuildFromFile(particalVertShaderPath, particalFragShaderPath);
    glViewport(0, 0, mWindowWidth, mWindowHeight);


    glGenVertexArrays(1, &mVaoParticals);
    glGenBuffers(1, &mPositionBuffer);
    glGenBuffers(1, &mDensityBuffer);
}

void RenderWidget::Update() {
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // 画粒子
    mParticalShader->Use();
    glEnable(GL_PROGRAM_POINT_SIZE);
    glBindVertexArray(mVaoParticals);
    glDrawArrays(GL_POINTS, 0, mParticalNum);

    // 显示FPS
    char title[128] = "";
    sprintf_s(title, "Fluid Simulation FPS=%.2f", CalculateFPS());
    glfwSetWindowTitle(mWindow, title);

    glfwSwapBuffers(mWindow);
}

int32_t RenderWidget::Destroy() {

    glDeleteVertexArrays(1, &mVaoParticals);
    glDeleteBuffers(1, &mPositionBuffer);
    glDeleteBuffers(1, &mDensityBuffer);
    delete mParticalShader;
    glfwTerminate();
    return 0;
}

bool RenderWidget::ShouldClose() {
    return glfwWindowShouldClose(mWindow);
}

void RenderWidget::ProcessInput() {
    if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(mWindow, true);
    }

    return;
}

void RenderWidget::PollEvents() {
    glfwPollEvents();
}

void RenderWidget::LoadVertexes(Fluid2d::ParticalSystem& ps) {
    glBindVertexArray(mVaoParticals);
    glBindBuffer(GL_ARRAY_BUFFER, mPositionBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * ps.mPositions.size(), ps.mPositions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, mDensityBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * ps.mDensity.size(), ps.mDensity.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    mParticalNum = ps.mPositions.size();
}

bool RenderWidget::CreateWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    

    // 创建窗口
    mWindow = glfwCreateWindow(mWindowWidth, mWindowHeight, "Fluid Simulation", NULL, NULL);
    if (mWindow == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwSetWindowPos(mWindow, 100, 100);
    glfwMakeContextCurrent(mWindow);

    // 注册回调函数
    glfwSetWindowUserPointer(mWindow, this);
    glfwSetFramebufferSizeCallback(mWindow, ResizeCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    return true;
}

float RenderWidget::CalculateFPS() {
    auto nowTime = std::chrono::system_clock::now();
    auto deltaTime = nowTime - mUpdateTime;
    mUpdateTime = nowTime;
    auto durMS = std::chrono::duration_cast<std::chrono::milliseconds>(deltaTime).count();
    float fps = 1000.0f / durMS;
    return fps;
}

void RenderWidget::ResizeCallback(GLFWwindow* window, int width, int height) {
    // 找到this指针
    auto thisPtr = reinterpret_cast<RenderWidget*>(glfwGetWindowUserPointer(window));
    glViewport(0, 0, width, height);
}
