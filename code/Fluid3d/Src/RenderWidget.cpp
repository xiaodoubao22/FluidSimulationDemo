#include "RenderWidget.h"
#include "Global.h"

#include <iostream>
#include <fstream>
#include <thread>

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

std::vector<float_t> floorVertices = {
    // vertex           texCoord
     1.0f,  1.0f, 0.0f, 1.0, 1.0,
    -1.0f,  1.0f, 0.0f, 0.0, 1.0,
    -1.0f, -1.0f, 0.0f, 0.0, 0.0,
     1.0f,  1.0f, 0.0f, 1.0, 1.0,
    -1.0f, -1.0f, 0.0f, 0.0, 0.0,
     1.0f, -1.0f, 0.0f, 1.0, 0.0,
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

        GenerateFrameBuffers();
        GenerateBuffers();
        GenerateTextures();

        InitFilters();
        LoadSkyBox();
        LoadMaterials();
        MakeVertexArrays(); // 生成画粒子的vao

        glGenVertexArrays(1, &mVaoNull);
        glEnable(GL_MULTISAMPLE);
    }

    void RenderWidget::UploadUniforms(Fluid3d::ParticalSystem3D& ps) {
        mComputeParticals->Use();
        mComputeParticals->SetUVec3("blockNum", ps.mBlockNum);
        mComputeParticals->SetVec3("blockSize", ps.mBlockSize);
        mComputeParticals->SetVec3("containerLowerBound", ps.mLowerBound);
        mComputeParticals->SetVec3("containerUpperBound", ps.mUpperBound);
        glUniform1iv(glGetUniformLocation(mComputeParticals->GetId(), "blockIdOffs"), ps.mBlockIdOffs.size(), ps.mBlockIdOffs.data());

        mComputeParticals->SetFloat("gSupportRadius", Para3d::supportRadius);
        mComputeParticals->SetFloat("gDensity0", Para3d::density0);
        mComputeParticals->SetFloat("gVolume", ps.mVolume);
        mComputeParticals->SetFloat("gMass", 0.5);
        mComputeParticals->SetFloat("gStiffness", ps.mStiffness);
        mComputeParticals->SetFloat("gExponent", ps.mExponent);
        mComputeParticals->SetFloat("gViscosity", ps.mViscosity);
        mComputeParticals->UnUse();

        glBindTexture(GL_TEXTURE_1D, mTexKernelBuffer);
        glTexImage1D(GL_TEXTURE_1D, 0, GL_RG32F, ps.mW.GetBufferSize(), 0, GL_RG, GL_FLOAT, ps.mW.GetData());
        glBindTexture(GL_TEXTURE_1D, 0);
    }

    void RenderWidget::UploadParticalInfo(Fluid3d::ParticalSystem3D& ps) {
        // 申请装粒子信息的buffer
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, mBufferParticals);
        glBufferData(GL_SHADER_STORAGE_BUFFER, ps.mParticalInfos.size() * sizeof(ParticalInfo3d), ps.mParticalInfos.data(), GL_DYNAMIC_COPY);

        // 申请block区间buffer
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, mBufferBlocks);
        glBufferData(GL_SHADER_STORAGE_BUFFER, ps.mBlockExtens.size() * sizeof(glm::uvec2), ps.mBlockExtens.data(), GL_DYNAMIC_COPY);
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
            glBindImageTexture(0, mTestTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
            mComputeParticals->Use();
            //mComputeParticals->SetVec3("gGravityDir", -mCamera.GetUp());
            mComputeParticals->SetVec3("gGravityDir", glm::vec3(0.0, 0.0, -1.0));
            for (int pass = 0; pass <= 1; pass++) {
                mComputeParticals->SetUInt("pass", pass);
                glDispatchCompute(mParticalNum, 1, 1);
                glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            }
            mComputeParticals->UnUse();
        }
    }

    void RenderWidget::Update() {
        DrawParticals();
        UpdateFPS();    // 显示FPS
        glfwSwapBuffers(mWindow);   // 交换前后缓冲
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
        mComputeParticals = new Glb::ComputeShader();
        std::vector<std::string> computeShaderpaths = {
            std::string("../code/Fluid3d/Shaders/ComputeParticals.comp"),
        };
        mComputeParticals->BuildFromFiles(computeShaderpaths);
        mComputeParticals->Use();
        glUniform1i(glGetUniformLocation(mComputeParticals->GetId(), "kernelBuffer"), 1);
        mComputeParticals->UnUse();

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

        mPointSpriteZValue = new Glb::Shader();
        std::string pointSpriteZValueVertPath = "../code/Fluid3d/Shaders/PointSprite.vert";
        std::string pointSpriteZValueGeomPath = "../code/Fluid3d/Shaders/PointSprite.geom";
        std::string pointSpriteZValueFragPath = "../code/Fluid3d/Shaders/PointSpriteZValue.frag";
        mPointSpriteZValue->BuildFromFile(pointSpriteZValueVertPath, pointSpriteZValueFragPath, pointSpriteZValueGeomPath);

        mPointSpriteThickness = new Glb::Shader();
        std::string pointSpriteThicknessVertPath = "../code/Fluid3d/Shaders/PointSprite.vert";
        std::string pointSpriteThicknessGeomPath = "../code/Fluid3d/Shaders/PointSprite.geom";
        std::string pointSpriteThicknessFragPath = "../code/Fluid3d/Shaders/PointSpriteThickness.frag";
        mPointSpriteThickness->BuildFromFile(pointSpriteThicknessVertPath, pointSpriteThicknessFragPath, pointSpriteThicknessGeomPath);

        mComputeNormal = new Glb::ComputeShader();
        std::string computeNormalPath = "../code/Fluid3d/Shaders/ComputeNormal.comp";
        mComputeNormal->BuildFromFile(computeNormalPath);

        mBlurZ = new Glb::ComputeShader();
        std::string blurZPath = "../code/Fluid3d/Shaders/BlurZ.comp";
        mBlurZ->BuildFromFile(blurZPath);
        mBlurZ->Use();
        mBlurZ->SetInt("weightBuffer", 0);
        mBlurZ->UnUse();

        mDrawFluidColor = new Glb::Shader();
        std::string drawFluidColorVertPath = "../code/Fluid3d/Shaders/DrawFluidColor.vert";
        std::string drawFluidColorFragPath = "../code/Fluid3d/Shaders/DrawFluidColor.frag";
        mDrawFluidColor->BuildFromFile(drawFluidColorVertPath, drawFluidColorFragPath);
        mDrawFluidColor->Use();
        glUniform1i(glGetUniformLocation(mDrawFluidColor->GetId(), "textureNormal"), 0);
        glUniform1i(glGetUniformLocation(mDrawFluidColor->GetId(), "skybox"), 1);
        mDrawFluidColor->UnUse();

        mDrawModel = new Glb::Shader();
        std::string drawModelVertPath = "../code/Fluid3d/Shaders/DrawModel.vert";
        std::string drawModelFragPath = "../code/Fluid3d/Shaders/DrawModel.frag";
        mDrawModel->BuildFromFile(drawModelVertPath, drawModelFragPath);

    }

    void RenderWidget::InitFilters() {
        mDepthFilter = new BilaterialFilter(8.0, 0.025);
        mDepthFilter->PreCalculate(2);
        
        // 传入纹理 
        glBindTexture(GL_TEXTURE_2D, mTexDepthFilter);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 
            mDepthFilter->GetWeightBufferSize().x, mDepthFilter->GetWeightBufferSize().y, 
            0, GL_RED, GL_FLOAT,
            mDepthFilter->GetWeightBuffer());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        // 索引传入Buffer
        std::vector<glm::ivec2> kernelIndexes = BilaterialFilter::GenerateIndexes(2);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, mBufferKernelIndexs5x5);
        glBufferData(GL_SHADER_STORAGE_BUFFER, kernelIndexes.size() * sizeof(glm::ivec2), kernelIndexes.data(), GL_STATIC_DRAW);

        kernelIndexes = BilaterialFilter::GenerateIndexes(4);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, mBufferKernelIndexs9x9);
        glBufferData(GL_SHADER_STORAGE_BUFFER, kernelIndexes.size() * sizeof(glm::ivec2), kernelIndexes.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    }

    void RenderWidget::GenerateFrameBuffers() {
        // depth framebuffer
        glGenFramebuffers(1, &mFboDepth);

        glGenTextures(1, &mTexZBuffer);
        glBindTexture(GL_TEXTURE_2D, mTexZBuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, mWindowWidth, mWindowHeight, 0, GL_RED, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0);

        glGenRenderbuffers(1, &mRboDepthBuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, mRboDepthBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, mWindowWidth, mWindowHeight);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, mFboDepth);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexZBuffer, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mRboDepthBuffer);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "ERROR: mFboDepth is not complete!" << std::endl;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // thickness framebuffer
        glGenFramebuffers(1, &mFboThickness);

        glGenTextures(1, &mTexThicknessBuffer);
        glBindTexture(GL_TEXTURE_2D, mTexThicknessBuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, mWindowWidth, mWindowHeight, 0, GL_RED, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, mFboThickness);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexThicknessBuffer, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mRboDepthBuffer);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "ERROR: mFboThickness is not complete!" << std::endl;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void RenderWidget::GenerateBuffers() {
        glGenBuffers(1, &mCoordVertBuffer);     // coord vbo
        glGenBuffers(1, &mBufferParticals);     // ssbo
        glGenBuffers(1, &mBufferBlocks);
        glGenBuffers(1, &mBufferKernelIndexs5x5);
        glGenBuffers(1, &mBufferKernelIndexs9x9);
        glGenBuffers(1, &mBufferFloor);
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
        glBindTexture(GL_TEXTURE_2D, 0);

        // 核函数纹理
        glGenTextures(1, &mTexKernelBuffer);
        glBindTexture(GL_TEXTURE_1D, mTexKernelBuffer);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_1D, 0);

        // 模糊Z后的坐标图
        glGenTextures(1, &mTexZBlurTempBuffer);
        glBindTexture(GL_TEXTURE_2D, mTexZBlurTempBuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, mWindowWidth, mWindowHeight, 0, GL_RED, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0);

        // DepthFilterBuffer
        glGenTextures(1, &mTexDepthFilter);

    }

    void RenderWidget::LoadSkyBox() {
        mSkyBox = new Glb::SkyBox();
        mSkyBox->Create();
        std::vector<std::string> paths
        {
            "../resources/skybox/right.jpg",
            "../resources/skybox/left.jpg",
            "../resources/skybox/top.jpg",
            "../resources/skybox/bottom.jpg",
            "../resources/skybox/front.jpg",
            "../resources/skybox/back.jpg"
        };
        mSkyBox->LoadImages(paths);
        mSkyBox->BuildShader();
    }

    void RenderWidget::LoadMaterials() {
        mSlabWhite = new Material();
        mSlabWhite->Create();
        std::string albedoPath = "../resources/SlabWhite/TexturesCom_Marble_SlabWhite_1K_albedo.png";
        mSlabWhite->LoadTexures(albedoPath);
    }

    void RenderWidget::MakeVertexArrays() {
        glGenVertexArrays(1, &mVaoParticals);
        glBindVertexArray(mVaoParticals);
        glBindBuffer(GL_ARRAY_BUFFER, mBufferParticals);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ParticalInfo3d), (void*)offsetof(ParticalInfo3d, position));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ParticalInfo3d), (void*)offsetof(ParticalInfo3d, density));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);

        glGenVertexArrays(1, &mVaoCoord);
        glBindVertexArray(mVaoCoord);
        glBindBuffer(GL_ARRAY_BUFFER, mCoordVertBuffer);
        glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec3), vertexes, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);

        glGenVertexArrays(1, &mVaoFloor);
        glBindVertexArray(mVaoFloor);
        glBindBuffer(GL_ARRAY_BUFFER, mBufferFloor);
        glBufferData(GL_ARRAY_BUFFER, floorVertices.size() * sizeof(float_t), floorVertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(0));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float_t)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);
    }

    void RenderWidget::DrawParticals() {
        glFinish();
        // 以点的形式画粒子
        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
        //glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        //glEnable(GL_DEPTH_TEST);
        //glDepthFunc(GL_LEQUAL);
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //glEnable(GL_PROGRAM_POINT_SIZE);
        //mDrawColor3d->Use();
        //mDrawColor3d->SetMat4("view", mCamera.GetView());
        //mDrawColor3d->SetMat4("projection", mCamera.GetProjection());
        //glBindVertexArray(mVaoCoord);
        //glDrawElements(GL_LINES, 6, GL_UNSIGNED_INT, indices);
        //glBindVertexArray(mVaoParticals);
        //glDrawArrays(GL_POINTS, 0, mParticalNum);
        //mSkyBox->Draw(mWindow, mVaoNull, mCamera.GetView(), mCamera.GetProjection());
        //mDrawColor3d->UnUse();

        // 预处理
        glBindFramebuffer(GL_FRAMEBUFFER, mFboDepth);
        glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_PROGRAM_POINT_SIZE);

        // 画深度图
        mPointSpriteZValue->Use();
        mPointSpriteZValue->SetMat4("view", mCamera.GetView());
        mPointSpriteZValue->SetMat4("projection", mCamera.GetProjection());
        mPointSpriteZValue->SetFloat("particalRadius", 0.01f);
        mPointSpriteZValue->SetVec3("cameraUp", mCamera.GetUp());
        mPointSpriteZValue->SetVec3("cameraRight", mCamera.GetRight());
        mPointSpriteZValue->SetVec3("cameraFront", mCamera.GetFront());
        glBindVertexArray(mVaoParticals);
        glDrawArrays(GL_POINTS, 0, mParticalNum);
        mPointSpriteZValue->UnUse();

        // 画厚度图
        glBindFramebuffer(GL_FRAMEBUFFER, mFboThickness);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        mPointSpriteThickness->Use();
        mPointSpriteThickness->SetMat4("view", mCamera.GetView());
        mPointSpriteThickness->SetMat4("projection", mCamera.GetProjection());
        mPointSpriteThickness->SetFloat("particalRadius", 0.01f);
        mPointSpriteThickness->SetVec3("cameraUp", mCamera.GetUp());
        mPointSpriteThickness->SetVec3("cameraRight", mCamera.GetRight());
        mPointSpriteThickness->SetVec3("cameraFront", mCamera.GetFront());
        glBindVertexArray(mVaoParticals);
        glDrawArrays(GL_POINTS, 0, mParticalNum);
        mPointSpriteThickness->UnUse();
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);

        // 模糊深度
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mTexDepthFilter);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mBufferKernelIndexs5x5);
        mBlurZ->Use();
        mBlurZ->SetInt("indexesSize", 25);
        mBlurZ->SetFloat("sigma1", mDepthFilter->mSigma1);
        mBlurZ->SetFloat("sigma2", mDepthFilter->mSigma2);
        GLuint bufferA = mTexZBuffer;
        GLuint bufferB = mTexZBlurTempBuffer;
        for (int i = 0; i < 4; i++) {
            mBlurZ->SetInt("filterInterval", std::pow(2, i));
            glBindImageTexture(0, bufferA, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
            glBindImageTexture(1, bufferB, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
            glDispatchCompute(mWindowWidth, mWindowHeight, 1);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            std::swap(bufferA, bufferB);
        }

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mBufferKernelIndexs9x9);
        mBlurZ->SetInt("indexesSize", 81);
        mBlurZ->SetInt("filterInterval", 1);
        glBindImageTexture(0, bufferA, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
        glBindImageTexture(1, bufferB, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
        glDispatchCompute(mWindowWidth, mWindowHeight, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        // 渲染
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 画地板
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mSlabWhite->mTexAlbedo);
        mDrawModel->Use();
        mDrawModel->SetMat4("view", mCamera.GetView());
        mDrawModel->SetMat4("projection", mCamera.GetProjection());
        glBindVertexArray(mVaoFloor);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        mDrawModel->UnUse();

        mSkyBox->Draw(mWindow, mVaoNull, mCamera.GetView(), mCamera.GetProjection());

        // 画流体
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, mSkyBox->GetId());
        glBindImageTexture(0, bufferB, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
        glBindImageTexture(1, mTexThicknessBuffer, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mBufferFloor);
        mDrawFluidColor->Use();
        mDrawFluidColor->SetMat4("camToWorldRot", glm::transpose(mCamera.GetView()));
        mDrawFluidColor->SetMat4("camToWorld", glm::inverse(mCamera.GetView()));
        mDrawFluidColor->SetMat4("projection", mCamera.GetProjection());
        glBindVertexArray(mVaoParticals);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
        mDrawFluidColor->UnUse();

        

//#define TEST_QUAD
#ifdef TEST_QUAD
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mTexThicknessBuffer);
        mScreenQuad->Use();
        glBindVertexArray(mVaoNull);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        mScreenQuad->UnUse();
#endif //TEST_QUAD
    }

    int32_t RenderWidget::Destroy() {
        delete mScreenQuad;
        delete mDrawColor3d;
        delete mComputeParticals;

        glDeleteVertexArrays(1, &mVaoNull);
        glDeleteVertexArrays(1, &mVaoParticals);
        glDeleteVertexArrays(1, &mVaoCoord);

        glDeleteBuffers(1, &mCoordVertBuffer);
        glDeleteBuffers(1, &mBufferParticals);
        glDeleteBuffers(1, &mBufferBlocks);

        glDeleteTextures(1, &mTestTexture);
        glDeleteTextures(1, &mTexKernelBuffer);

        glfwTerminate();
        return 0;
    }


}

