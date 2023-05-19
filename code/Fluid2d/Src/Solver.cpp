#include "Solver.h"
#include "Global.h"
#include <iostream>
#include <algorithm>

namespace Fluid2d {

    Solver::Solver(ParticalSystem& ps) : mPs(ps), mW(ps.mSupportRadius)
    {

    }

    Solver::~Solver() {

    }

    void Solver::Iterate() {
        Glb::Timer timer;
        timer.Start();
        UpdateDensityAndPressure();
        InitAccleration();
        UpdateViscosityAccleration();
        UpdatePressureAccleration();
        EulerIntegrate();
        BoundaryCondition();
        std::cout << "solve time = " << timer.GetTime() << std::endl;
    }

    void Solver::UpdateDensityAndPressure() {
        mPs.mDensity = std::vector<float>(mPs.mPositions.size(), Glb::density0);
        mPs.mPressure = std::vector<float>(mPs.mPositions.size(), 0.0f);
        for (int i = 0; i < mPs.mPositions.size(); i++) {    // 对所有粒子
            if (mPs.mNeighbors.size() != 0) {    // 有邻居
                std::vector<NeighborInfo>& neighbors = mPs.mNeighbors[i];
                float density = 0;
                for (auto& nInfo : neighbors) {
                    density += mW.Value(nInfo.distance);
                }
                density *= (mPs.mVolume * Glb::density0);
                mPs.mDensity[i] = density;
                mPs.mDensity[i] = std::max(density, Glb::density0);        // 禁止膨胀
            }
            // 更新压强
            mPs.mPressure[i] = mPs.mStiffness* (std::powf(mPs.mDensity[i] / Glb::density0, mPs.mExponent) - 1.0f);
        }
        //int p = 0;
        //for (int i = 0; i < 60; i++) {
        //    for (int j = 0; j < 60; j++) {
        //        std::cout << mPs.mDensity[p] << " ";
        //        p++;
        //    }
        //    std::cout << std::endl;
        //}
    }

    void Solver::InitAccleration() {
        std::fill(mPs.mAccleration.begin() + mPs.mStartIndex, mPs.mAccleration.end(), glm::vec2(0.0f, -Glb::gravity));
        //mPs.mAccleration = std::vector<glm::vec2>(mPs.mPositions.size(), glm::vec2(0.0f, -Glb::gravity));
    }


    void Solver::UpdateViscosityAccleration() {
        float dim = 2.0f;
        float constFactor = 2.0f * (dim + 2.0f) * mPs.mViscosity;
        for (int i = mPs.mStartIndex; i < mPs.mPositions.size(); i++) {    // 对所有粒子
            if (mPs.mNeighbors.size() != 0) {    // 有邻居
                std::vector<NeighborInfo>& neighbors = mPs.mNeighbors[i];
                glm::vec2 viscosityForce(0.0f, 0.0f);
                for (auto& nInfo : neighbors) {
                    int j = nInfo.index;
                    float dotDvToRad = glm::dot(mPs.mVelocity[i] - mPs.mVelocity[j], nInfo.radius);

                    float denom = nInfo.distance2 + 0.01f * mPs.mSupportRadius2;
                    viscosityForce += (mPs.mMass / mPs.mDensity[j]) * dotDvToRad * mW.Grad(nInfo.radius) / denom;
                }
                viscosityForce *= constFactor;
                mPs.mAccleration[i] += viscosityForce;
            }
        }

        //int p = 0;
        //for (int i = 0; i < 60; i++) {
        //    for (int j = 0; j < 60; j++) {
        //        std::cout << "(" << mPs.mAccleration[p].x << "," << mPs.mAccleration[p].y << ") ";
        //        p++;
        //    }
        //    std::cout << std::endl;
        //}
    }

    void Solver::UpdatePressureAccleration() {
        std::vector<float> pressDivDens2(mPs.mPositions.size(), 0);        // p/(dens^2)
        for (int i = 0; i < mPs.mPositions.size(); i++) {
            pressDivDens2[i] = mPs.mPressure[i] / std::powf(mPs.mDensity[i], 2);
        }

        for (int i = mPs.mStartIndex; i < mPs.mPositions.size(); i++) {    // 对所有粒子
            if (mPs.mNeighbors.size() != 0) {    // 有邻居
                std::vector<NeighborInfo>& neighbors = mPs.mNeighbors[i];
                glm::vec2 pressureForce(0.0f, 0.0f);
                for (auto& nInfo : neighbors) {
                    int j = nInfo.index;
                    pressureForce += mPs.mDensity[j] * (pressDivDens2[i] + pressDivDens2[j]) * mW.Grad(nInfo.radius);
                }
                mPs.mAccleration[i] -= pressureForce * mPs.mVolume;
            }
        }

        //int p = 0;
        //float mx = 0;
        //float my = 0;
        //for (int i = 0; i < 60; i++) {
        //    for (int j = 0; j < 60; j++) {
        //        mx = std::max(mx, abs(mPs.mAccleration[p].x));
        //        my = std::max(my, abs(mPs.mAccleration[p].y));
        //        std::cout << "(" << mPs.mAccleration[p].x << "," << mPs.mAccleration[p].y << ") ";
        //        p++;
        //    }
        //    std::cout << std::endl;
        //}
        //std::cout << mx << " " << my << "...."  << std::endl;
    }

    void Solver::EulerIntegrate() {
        for (int i = mPs.mStartIndex; i < mPs.mPositions.size(); i++) {    // 对所有粒子
            mPs.mVelocity[i] += Glb::dt * mPs.mAccleration[i];
            mPs.mVelocity[i] = glm::clamp(mPs.mVelocity[i], glm::vec2(-100.0f), glm::vec2(100.0f));
            mPs.mPositions[i] += Glb::dt * mPs.mVelocity[i];
        }
    }

    void Solver::BoundaryCondition() {
        for (int i = 0; i < mPs.mPositions.size(); i++) {    // 对所有粒子
            glm::vec2& position = mPs.mPositions[i];
            bool invFlag = false;
            if (position.y < mPs.mLowerBound.y + mPs.mSupportRadius) {
                mPs.mVelocity[i].y = std::abs(mPs.mVelocity[i].y);
                invFlag = true;
            }

            if (position.y > mPs.mUpperBound.y - mPs.mSupportRadius) {
                mPs.mVelocity[i].y = -std::abs(mPs.mVelocity[i].y);
                invFlag = true;
            }

            if (position.x < mPs.mLowerBound.x + mPs.mSupportRadius) {
                mPs.mVelocity[i].x = std::abs(mPs.mVelocity[i].x);
                invFlag = true;
            }

            if (position.x > mPs.mUpperBound.x - mPs.mSupportRadius) {
                mPs.mVelocity[i].x = -std::abs(mPs.mVelocity[i].x);
                invFlag = true;
            }

            if (invFlag) {
                mPs.mPositions[i] += Glb::dt * mPs.mVelocity[i];
                mPs.mVelocity[i] = glm::clamp(mPs.mVelocity[i], glm::vec2(-100.0f), glm::vec2(100.0f));    // 速度限制
            }
        }
    }

}

