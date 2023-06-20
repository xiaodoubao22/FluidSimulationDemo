#ifndef DEPTH_FILTER_H
#define DEPTH_FILTER_H

#include <vector>
#include <glm/glm.hpp>
#include "ComputeShader.h"

namespace Fluid3d {
    class DepthFilter {
    public:
        DepthFilter();
        ~DepthFilter();

        void Create(float_t sigma1, float_t sigma2);
        void Destroy();
        void Filter(GLuint& bufferA, GLuint& bufferB, glm::ivec2 imageSize);

    private:
        void PreCalculate();
        void BuildShader();
        void Uploadbuffers();
        float_t CalculateWeight(float_t d1, float_t d2);
        std::vector<glm::ivec2> GenerateIndexes(int32_t halfkernelSize);

    private:
        std::vector<float_t> mWeightBuffer;
        glm::ivec2 mBufferSize;

        float_t mSigma1;
        float_t mSigma2;
        int32_t mHalfKernelSize;


        Glb::ComputeShader* mBlurZ = nullptr;

        GLuint mTexDepthFilter = 0;
        GLuint mBufferKernelIndexs5x5 = 0;
        GLuint mBufferKernelIndexs9x9 = 0;
    };
}


#endif // !DEPTH_FILTER_H
