#include "FluidShadowMap.h"
#include <iostream>
#include "Global.h"
#include "Parameters3d.h"

namespace Fluid3d {
    FluidShadowMap::FluidShadowMap() {

    }

    FluidShadowMap::~FluidShadowMap() {

    }

    void FluidShadowMap::SetImageSize(int32_t w, int32_t h) {
        mWidth = w;
        mHeight = h;
    }

    void FluidShadowMap::SetLightInfo(PointLight& light) {
        mLightViewFront = glm::normalize(light.dir);
        mLightViewRight = glm::normalize(glm::cross(mLightViewFront, Glb::Z_AXIS));
        mLightViewUp = glm::normalize(glm::cross(mLightViewRight, mLightViewFront));
        mLightView = glm::lookAt(light.pos, light.pos + light.dir, mLightViewUp);
        float_t aspect = float_t(mWidth) / mHeight;
        mLightProjection = glm::perspective(glm::radians(light.fovy), aspect, Para3d::zNear, Para3d::zFar);
    }

    void FluidShadowMap::SetIor(float_t ior) {
        mIor = ior;
    }

    void FluidShadowMap::Init() {
        CreateBuffers(mWidth, mHeight);
        CreateShaders();
        InitIntrinsic();
    }

    void FluidShadowMap::Destroy() {
        delete mPointSpriteZValue;
    }

    void FluidShadowMap::Update(GLuint vaoParticals, int32_t particalNum, DepthFilter* depthFilter) {
        // 渲染深度图
        glViewport(0, 0, mWidth, mHeight);
        glBindFramebuffer(GL_FRAMEBUFFER, mFbo);
        glClearColor(0.5f, 0.0f, 0.0f, 0.0f);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        mPointSpriteZValue->Use();
        glBindVertexArray(vaoParticals);
        glDrawArrays(GL_POINTS, 0, particalNum);
        mPointSpriteZValue->UnUse();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 平滑深度图
        mZBufferA = mTextureZBuffer;
        mZBufferB = mTextureTempZBuffer;
        depthFilter->Filter(mZBufferA, mZBufferB, glm::ivec2(mWidth, mHeight));
    }

    void FluidShadowMap::DrawCaustic(RenderCamera* camera, GLuint vaoNull, const glm::mat4& model) {
        // 渲染焦散图
        glViewport(0, 0, mWidth, mHeight);
        glBindFramebuffer(GL_FRAMEBUFFER, mFboCaustic);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glEnable(GL_PROGRAM_POINT_SIZE);

        glBindImageTexture(0, mZBufferB, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
        glBindVertexArray(vaoNull);

        mCausticMap->Use();
        mCausticMap->SetMat4("model", model);
        for (int i = 0; i < Glb::ORIGIN_COLORS.size(); i++) {
            mCausticMap->SetFloat("eta", 1.0 / (mIor + Para3d::IOR_BIAS * float(i - 1)));
            mCausticMap->SetVec3("photonColor", Glb::ORIGIN_COLORS[i] * Para3d::CAUSTIC_FACTOR);
            glDrawArrays(GL_POINTS, 0, mWidth * mHeight);
        }
        mCausticMap->UnUse();

        glBindVertexArray(0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
    }

    GLuint FluidShadowMap::GetShadowMap() {
        return mZBufferB;
    }

    GLuint FluidShadowMap::GetCausticMap() {
        return mTextureCausticMap;
    }

    void FluidShadowMap::CreateShaders() {
        mPointSpriteZValue = new Glb::Shader();
        std::string pointSpriteZValueVertPath = "../code/Fluid3d/Shaders/PointSprite.vert";
        std::string pointSpriteZValueGeomPath = "../code/Fluid3d/Shaders/PointSprite.geom";
        std::string pointSpriteZValueFragPath = "../code/Fluid3d/Shaders/PointSpriteZValue.frag";
        mPointSpriteZValue->BuildFromFile(pointSpriteZValueVertPath, pointSpriteZValueFragPath, pointSpriteZValueGeomPath);
        mPointSpriteZValue->Use();
        mPointSpriteZValue->SetMat4("view", mLightView);
        mPointSpriteZValue->SetMat4("projection", mLightProjection);
        mPointSpriteZValue->SetFloat("particalRadius", Para3d::particalDiameter);
        mPointSpriteZValue->SetVec3("cameraUp", mLightViewUp);
        mPointSpriteZValue->SetVec3("cameraRight", mLightViewRight);
        mPointSpriteZValue->SetVec3("cameraFront", mLightViewFront);
        mPointSpriteZValue->SetFloat("zFar", Para3d::zFar);
        mPointSpriteZValue->SetFloat("zNear", Para3d::zNear);
        mPointSpriteZValue->UnUse();

        mCausticMap = new Glb::Shader();
        std::string causticMapVertPath = "../code/Fluid3d/Shaders/CausticMap.vert";
        std::string causticMapFragPath = "../code/Fluid3d/Shaders/CausticMap.frag";
        mCausticMap->BuildFromFile(causticMapVertPath, causticMapFragPath);
        mCausticMap->Use();
        mCausticMap->SetMat4("lightView", mLightView);
        mCausticMap->SetMat4("lightProjection", mLightProjection);
        mCausticMap->SetMat4("lightToWorld", glm::inverse(mLightView));
        mCausticMap->SetMat4("lightToWorldRot", glm::mat4(glm::mat3(glm::inverse(mLightView))));
        mCausticMap->SetInt("imageWidth", mWidth);
        mCausticMap->SetInt("imageHeight", mHeight);
        mCausticMap->UnUse();
    }

    void FluidShadowMap::CreateBuffers(int32_t w, int32_t h) {
        glGenTextures(1, &mTextureZBuffer);
        glBindTexture(GL_TEXTURE_2D, mTextureZBuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, w, h, 0, GL_RED, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0);

        glGenTextures(1, &mTextureTempZBuffer);
        glBindTexture(GL_TEXTURE_2D, mTextureTempZBuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, w, h, 0, GL_RED, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0);

        mZBufferA = mTextureZBuffer;
        mZBufferB = mTextureTempZBuffer;

        glGenTextures(1, &mTextureCausticMap);
        glBindTexture(GL_TEXTURE_2D, mTextureCausticMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glm::vec4 borderColor = glm::vec4(0.0f);
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &borderColor.x);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0);

        glGenRenderbuffers(1, &mDepthStencil);
        glBindRenderbuffer(GL_RENDERBUFFER, mDepthStencil);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        glGenFramebuffers(1, &mFbo);
        glBindFramebuffer(GL_FRAMEBUFFER, mFbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextureZBuffer, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthStencil);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "ERROR: FluidShadowMap mFbo is not complete!" << std::endl;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glGenFramebuffers(1, &mFboCaustic);
        glBindFramebuffer(GL_FRAMEBUFFER, mFboCaustic);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextureCausticMap, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthStencil);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "ERROR: FluidShadowMap mFboCaustic is not complete!" << std::endl;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void FluidShadowMap::InitIntrinsic() {
        mCausticMap->Use();
        mCausticMap->SetVec4("lightIntrinsic", Glb::ProjToIntrinsic(mLightProjection, mWidth, mHeight));
        mCausticMap->UnUse();
    }
}