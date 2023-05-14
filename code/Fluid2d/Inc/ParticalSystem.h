#ifndef PARTICAL_SYSTEM_H
#define PARTICAL_SYSTEM_H

#include <vector>
#include <glm/glm.hpp>
#include "Global.h"

namespace Fluid2d {

    struct NeighborInfo {
        int index;
        float distance;
        float distance2;
        glm::vec2 radius;
    };

    class ParticalSystem {
    public:
        ParticalSystem();
        ~ParticalSystem();

        void SetContainerSize(glm::vec2 corner, glm::vec2 size);

        int32_t AddFluidBlock(glm::vec2 corner, glm::vec2 size, glm::vec2 v0);

        void SearchNeighbors();

        size_t GetBlockIdByPosition(glm::vec2 position);

        void BuildBlockStructure();


    public:
        // 粒子参数
        float mSupportRadius = 0.025;    // 支撑半径
        float mSupportRadius2 = mSupportRadius * mSupportRadius;
        float mParticalRadius = 0.005;   // 粒子半径
        float mParticalDiameter = 2.0 * mParticalRadius;
        float mVolume = 0.8 * mParticalDiameter * mParticalDiameter;    // 体积
        float mMass = Glb::density0 * mVolume;  // 质量
        float mViscosity = 0.01;            // 粘度系数
        float mExponent = 7.0f;              // 压力指数
        int mStiffness = 50.0f;            // 刚度

        std::vector<glm::vec2> mPositions;
        std::vector<glm::vec2> mAccleration;
        std::vector<glm::vec2> mVelocity;
        std::vector<float> mDensity;
        std::vector<float> mPressure;
        std::vector<std::vector<NeighborInfo>> mNeighbors;

        // 容器参数
        glm::vec2 mLowerBound = glm::vec2(-1.0f, -1.0f);
        glm::vec2 mUpperBound = glm::vec2(1.0f, 1.0f);
        std::vector<std::vector<int>> mBlocks;
        glm::vec2 mBlockSize = glm::vec2(0.5, 0.5);
        uint32_t mBlockRowNum = 4;
        uint32_t mBlockColNum = 4;

    };

}



#endif // !PARTICAL_SYSTEM_H

