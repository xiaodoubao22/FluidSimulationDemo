#include "RenderWidget.h"
#include "Global.h"

#include <iostream>
#include <fstream>


const unsigned int TEXTURE_WIDTH = 1000, TEXTURE_HEIGHT = 1000;

const glm::vec3 vertexes[] = {
    glm::vec3(0.0, 0.0, 0.0),
    glm::vec3(1.0, 0.0, 0.0),
    glm::vec3(0.0, 1.0, 0.0),
    glm::vec3(0.0, 0.0, 1.0)
};

const GLuint indices[] = {
    0, 1, 0, 2, 0, 3
};

namespace Fluid3d {
    RenderWidget::RenderWidget() {

    }

    RenderWidget::~RenderWidget() {
        Destroy();
    }

    int32_t RenderWidget::Init() {
        if (!CreateWindow()) {
            return -1;
        }

        BuildShaders();
        GenerateBuffers();  // 生成所有Buffer
        GenerateTextures(); // 生成所有纹理
        MakeVaoParticals(); // 生成画粒子的vao
        MakeVaoCoord(); // 生成画坐标系的vao

        glGenVertexArrays(1, &mVaoNull);
        glEnable(GL_MULTISAMPLE);
    }

    void RenderWidget::UploadUniforms(Fluid3d::ParticalSystem3D& ps) {
        mComputeShader->Use();
        mComputeShader->SetUVec3("blockNum", ps.mBlockNum);
        mComputeShader->SetVec3("blockSize", ps.mBlockSize);
        mComputeShader->SetVec3("containerLowerBound", ps.mLowerBound);
        mComputeShader->SetVec3("containerUpperBound", ps.mUpperBound);
        glUniform1iv(glGetUniformLocation(mComputeShader->GetId(), "blockIdOffs"), 27, ps.mBlockIdOffs.data());
        mComputeShader->SetFloat("gDensity0", Para3d::density0);
        mComputeShader->SetFloat("gVolume", ps.mVolume);
        mComputeShader->SetFloat("gMass", 0.5);
        mComputeShader->SetFloat("gStiffness", ps.mStiffness);
        mComputeShader->SetFloat("gExponent", ps.mExponent);
        mComputeShader->SetFloat("gViscosity", ps.mViscosity);
        mComputeShader->UnUse();

        glBindTexture(GL_TEXTURE_1D, mTexKernelBuffer);
        glTexImage1D(GL_TEXTURE_1D, 0, GL_RG32F, ps.mW.GetBufferSize(), 0, GL_RG, GL_FLOAT, ps.mW.GetData());
        glBindTexture(GL_TEXTURE_1D, 0);
    }

    void RenderWidget::UploadParticalInfo(Fluid3d::ParticalSystem3D& ps) {
        // 更新装粒子信息的buffer
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, mBufferParticals);
        glBufferData(GL_SHADER_STORAGE_BUFFER, ps.mParticalInfos.size() * sizeof(ParticalInfo3d), ps.mParticalInfos.data(), GL_STATIC_DRAW);
        
        // 更新block区间buffer
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, mBufferBlocks);
        glBufferData(GL_SHADER_STORAGE_BUFFER, ps.mBlockExtens.size() * sizeof(glm::uvec2), ps.mBlockExtens.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        mParticalNum = ps.mParticalInfos.size();
    }

    void RenderWidget::DumpParticalInfo(Fluid3d::ParticalSystem3D& ps) {
        // 把粒子信息拷回CPU
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, mBufferParticals);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, mParticalNum * sizeof(ParticalInfo3d), (void*)ps.mParticalInfos.data());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    void RenderWidget::SolveParticals() {
        if (mParticalNum > 0) {
            glFinish();
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, mBufferParticals);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, mBufferBlocks);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_1D, mTexKernelBuffer);
            mComputeShader->Use();

            mComputeShader->SetUInt("pass", 0);
            mComputeShader->SetVec3("gGravityDir", -mCamera.GetUp());
            glDispatchCompute(mParticalNum, 1, 1);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            mComputeShader->SetUInt("pass", 1);
            glDispatchCompute(mParticalNum, 1, 1);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            mComputeShader->UnUse();
        }
    }

    void RenderWidget::Update() {
        DrawParticals();
        UpdateFPS();    // 显示FPS
        glfwSwapBuffers(mWindow);   // 交换前后缓冲
    }

    int32_t RenderWidget::Destroy() {
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

    void RenderWidget::ResizeCallback(GLFWwindow* window, int width, int height) {
        // 找到this指针
        auto thisPtr = reinterpret_cast<RenderWidget*>(glfwGetWindowUserPointer(window));
        glViewport(0, 0, width, height);
        thisPtr->mCamera.SetPerspective(float(width) / float(height));
    }

    void RenderWidget::CursorPosCallBack(GLFWwindow* window, double xpos, double ypos) {
        auto thisPtr = reinterpret_cast<RenderWidget*>(glfwGetWindowUserPointer(window));
        if (!(thisPtr->mLeftPressFlag || thisPtr->mRightPressFlag)) {
            return;
        }

        if (thisPtr->mFirstMouseFlag) {
            thisPtr->mLastX = xpos;
            thisPtr->mLastY = ypos;
            thisPtr->mFirstMouseFlag = false;
        }

        float xOffset = xpos - thisPtr->mLastX;
        float yOffset = ypos - thisPtr->mLastY;

        if (thisPtr->mLeftPressFlag) {
            thisPtr->mCamera.ProcessRotate(glm::vec2(xOffset, yOffset));
        }
        else if (thisPtr->mRightPressFlag) {
            thisPtr->mCamera.ProcessMove(glm::vec2(xOffset, yOffset));
        }
        

        thisPtr->mLastX = xpos;
        thisPtr->mLastY = ypos;
    }

    void RenderWidget::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
        auto thisPtr = reinterpret_cast<RenderWidget*>(glfwGetWindowUserPointer(window));
        if (action == GLFW_PRESS) {
            if (button == GLFW_MOUSE_BUTTON_LEFT) {
                thisPtr->mLeftPressFlag = true;
                thisPtr->mFirstMouseFlag = true;
            }
            else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
                thisPtr->mRightPressFlag = true;
                thisPtr->mFirstMouseFlag = true;
            }
        }
        else if (action == GLFW_RELEASE) {
            if (button == GLFW_MOUSE_BUTTON_LEFT) {
                thisPtr->mLeftPressFlag = false;
            }
            else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
                thisPtr->mRightPressFlag = false;
            }
        }
    }

    void RenderWidget::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
        auto thisPtr = reinterpret_cast<RenderWidget*>(glfwGetWindowUserPointer(window));
        thisPtr->mCamera.ProcessScale(static_cast<float>(yoffset));
    }

    bool RenderWidget::CreateWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);  // 版本
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_SAMPLES, 9);    // 多重采样

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
        glfwSetCursorPosCallback(mWindow, CursorPosCallBack);
        glfwSetMouseButtonCallback(mWindow, MouseButtonCallback);
        glfwSetScrollCallback(mWindow, ScrollCallback);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return false;
        }

        return true;
    }

    void RenderWidget::UpdateFPS() {
        // 计算FPS
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - mUpdateTime;
        mUpdateTime = currentTime;
        updateTitleTime += deltaTime;
        frameCount += 1.0f;

        // 更新FPS
        if (updateTitleTime > 1.0f) {
            char title[128] = "";
            sprintf_s(title, "Fluid Simulation FPS=%.2f", frameCount / updateTitleTime);
            glfwSetWindowTitle(mWindow, title);
            updateTitleTime = 0.0f;
            frameCount = 0.0f;
        }

        return;
    }

    void RenderWidget::BuildShaders() {
        mComputeShader = new Glb::ComputeShader();
        std::vector<std::string> computeShaderpaths = {
            std::string("../code/Fluid3d/Shaders/ComputeParticals.comp"),
        };
        mComputeShader->BuildFromFiles(computeShaderpaths);
        mComputeShader->Use();
        glUniform1i(glGetUniformLocation(mComputeShader->GetId(), "kernelBuffer"), 1);
        mComputeShader->UnUse();

        mScreenQuad = new Glb::Shader();
        std::string screenQuadVertPath = "../code/Fluid3d/Shaders/ScreenQuad.vert";
        std::string screenQuadFragPath = "../code/Fluid3d/Shaders/ScreenQuad.frag";
        mScreenQuad->BuildFromFile(screenQuadVertPath, screenQuadFragPath);
        mScreenQuad->Use();
        glUniform1i(glGetUniformLocation(mScreenQuad->GetId(), "tex"), 0);
        mScreenQuad->UnUse();

        mDrawColor3d = new Glb::Shader();
        std::string drawColorVertPath = "../code/Fluid3d/Shaders/DrawColor3d.vert";
        std::string drawColorFragPath = "../code/Fluid3d/Shaders/DrawColor3d.frag";
        mDrawColor3d->BuildFromFile(drawColorVertPath, drawColorFragPath);
    }

    void RenderWidget::GenerateBuffers() {
        glGenBuffers(1, &mCoordVertBuffer);     // coord vbo
        glGenBuffers(1, &mBufferParticals);     // ssbo
        glGenBuffers(1, &mBufferBlocks);
    }

    void RenderWidget::GenerateTextures() {
        // 测试用的纹理
        glGenTextures(1, &mTestTexture);
        glBindTexture(GL_TEXTURE_2D, mTestTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 100, 100, 0, GL_RGBA, GL_FLOAT, NULL);
        glBindImageTexture(0, mTestTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
        glBindTexture(GL_TEXTURE_2D, 0);

        glGenTextures(1, &mTexKernelBuffer);
        glBindTexture(GL_TEXTURE_1D, mTexKernelBuffer);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_1D, 0);
    }

    void RenderWidget::MakeVaoParticals() {
        glGenVertexArrays(1, &mVaoParticals);
        glBindVertexArray(mVaoParticals);
        glBindBuffer(GL_ARRAY_BUFFER, mBufferParticals);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ParticalInfo3d), (void*)offsetof(ParticalInfo3d, position));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ParticalInfo3d), (void*)offsetof(ParticalInfo3d, density));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);
    }

    void RenderWidget::MakeVaoCoord() {
        glGenVertexArrays(1, &mVaoCoord);
        glBindVertexArray(mVaoCoord);
        glBindBuffer(GL_ARRAY_BUFFER, mCoordVertBuffer);
        glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec3), vertexes, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
    }


    void RenderWidget::DrawParticals() {
        glFinish();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_PROGRAM_POINT_SIZE);

        mDrawColor3d->Use();
        mDrawColor3d->SetMat4("view", mCamera.GetView());
        mDrawColor3d->SetMat4("projection", mCamera.GetProjection());
        glBindVertexArray(mVaoCoord);
        glDrawElements(GL_LINES, 6, GL_UNSIGNED_INT, indices);
        glBindVertexArray(mVaoParticals);
        glDrawArrays(GL_POINTS, 0, mParticalNum);
        mDrawColor3d->UnUse();

//#define TEST_QUAD
#ifdef TEST_QUAD
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mTestTexture);
        mScreenQuad->Use();
        glBindVertexArray(mVaoNull);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        mScreenQuad->UnUse();
#endif
    }

}

