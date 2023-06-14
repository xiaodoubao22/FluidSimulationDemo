#include "ShadowMap.h"
#include <iostream>

namespace Fluid3d {
	FluidShadowMap::FluidShadowMap() {

	}

	FluidShadowMap::~FluidShadowMap() {

	}

	void FluidShadowMap::Create(int32_t w, int32_t h, PointLight& light) {
		mPointSpriteZValue = new Glb::Shader();
		std::string pointSpriteZValueVertPath = "../code/Fluid3d/Shaders/PointSprite.vert";
		std::string pointSpriteZValueGeomPath = "../code/Fluid3d/Shaders/PointSprite.geom";
		std::string pointSpriteZValueFragPath = "../code/Fluid3d/Shaders/PointSpriteZValue.frag";
		mPointSpriteZValue->BuildFromFile(pointSpriteZValueVertPath, pointSpriteZValueFragPath, pointSpriteZValueGeomPath);

        mWidth = w;
        mHeight = h;
        glGenFramebuffers(1, &mFbo);

        glGenTextures(1, &mTextureZBuffer);
        glBindTexture(GL_TEXTURE_2D, mTextureZBuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, w, h, 0, GL_RED, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0);

        glGenRenderbuffers(1, &mDepthStencil);
        glBindRenderbuffer(GL_RENDERBUFFER, mDepthStencil);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, mFbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextureZBuffer, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthStencil);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "ERROR: FluidShadowMap mFbo is not complete!" << std::endl;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        mLightViewFront = glm::normalize(light.dir);
        mLightViewRight = glm::normalize(glm::cross(mLightViewFront, glm::vec3(0.0, 0.0, 1.0)));
        mLightViewUp = glm::normalize(glm::cross(mLightViewRight, mLightViewFront));
        mLightView = glm::lookAt(light.pos, light.pos + light.dir, mLightViewUp);
        mLightProjection = glm::perspective(glm::radians(light.fovy), 1.0f, 0.1f, 100.0f);
	}

	void FluidShadowMap::Destroy() {
        delete mPointSpriteZValue;
	}

	void FluidShadowMap::Update(GLuint vaoParticals, int32_t particalNum) {

        glBindFramebuffer(GL_FRAMEBUFFER, mFbo);
        glClearColor(0.5f, 0.0f, 0.0f, 0.0f);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mPointSpriteZValue->Use();
        mPointSpriteZValue->SetMat4("view", mLightView);
        mPointSpriteZValue->SetMat4("projection", mLightProjection);
        mPointSpriteZValue->SetFloat("particalRadius", 0.01f);
        mPointSpriteZValue->SetVec3("cameraUp", mLightViewUp);
        mPointSpriteZValue->SetVec3("cameraRight", mLightViewRight);
        mPointSpriteZValue->SetVec3("cameraFront", mLightViewFront);
        glBindVertexArray(vaoParticals);
        glDrawArrays(GL_POINTS, 0, particalNum);
        mPointSpriteZValue->UnUse();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

    GLuint FluidShadowMap::GetTextureId() {
        return mTextureZBuffer;
    }

}