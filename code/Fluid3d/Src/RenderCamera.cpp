#include "RenderCamera.h"
#include <iostream>

namespace Fluid3d {
	RenderCamera::RenderCamera() {
        mYaw = 0.0f;
        mPitch = 30.0f;

        mWorldUp = glm::vec3(0.0, 0.0, 1.0);
        mTargetPoint = glm::vec3(0.15, 0.15, 0.1);
        UpdateView();

        mAspect = 1.0f;
        mNearPlane = 0.1f;
        mFarPlane = 100.0f;
        mFovyDeg = 60.0f;
        mProjection = glm::perspective(glm::radians(mFovyDeg), mAspect, mNearPlane, mFarPlane);
	}

	RenderCamera::~RenderCamera() {
        
	}

    void RenderCamera::ProcessMove(glm::vec2 offset) {
        mTargetPoint -= offset.x * mSensitiveX * mRight;
        mTargetPoint += offset.y * mSensitiveY * mUp;
        UpdateView();
    }

    void RenderCamera::ProcessRotate(glm::vec2 offset) {
        mYaw = std::fmodf(mYaw - mSensitiveYaw * offset.x, 360.0f);
        mPitch = glm::clamp(mPitch + mSensitivePitch * offset.y, -89.9f, 89.9f);
        UpdateView();
    }

    void RenderCamera::ProcessScale(float offset) {
        mTargetPoint += offset * mSensitiveFront * mFront;
        UpdateView();
    }

    glm::mat4 RenderCamera::GetView() {
        return mView;
    }

    glm::mat4 RenderCamera::GetProjection() {
        return mProjection;
    }

    glm::vec3 RenderCamera::GetUp() {
        return mUp;
    }

    void RenderCamera::UpdateView() {
        mFront.x = std::cos(glm::radians(mPitch)) * std::cos(glm::radians(mYaw));
        mFront.y = std::cos(glm::radians(mPitch)) * std::sin(glm::radians(mYaw));
        mFront.z = std::sin(glm::radians(mPitch));
        mFront = -glm::normalize(mFront);

        mRight = glm::normalize(glm::cross(mFront, mWorldUp));
        mUp = glm::normalize(glm::cross(mRight, mFront));

        mPosition = mTargetPoint - mTargetDistance * mFront;
        mView = glm::lookAt(mPosition, mTargetPoint, mUp);
    }
}