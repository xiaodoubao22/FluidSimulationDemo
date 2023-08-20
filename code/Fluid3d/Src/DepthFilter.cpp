#include "DepthFilter.h"

namespace Fluid3d {
    DepthFilter::DepthFilter() {
    }

    DepthFilter::~DepthFilter() {
        delete mBlurZ;
    }

    void DepthFilter::Create(float_t sigma1, float_t sigma2) {
        mSigma1 = sigma1;
        mSigma2 = sigma2;
        PreCalculate();
        BuildShader();
        Uploadbuffers();
    }

    void DepthFilter::Destroy() {

    }

    void DepthFilter::Filter(GLuint& bufferA, GLuint& bufferB, glm::ivec2 imageSize) {
        // 模糊深度
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mTexDepthFilter);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mBufferKernelIndexs5x5);
        mBlurZ->Use();
        mBlurZ->SetInt("indexesSize", 25);
        mBlurZ->SetFloat("sigma1", mSigma1);
        mBlurZ->SetFloat("sigma2", mSigma2);
        for (int i = 0; i < 5; i++) {
            mBlurZ->SetInt("filterInterval", std::pow(2, i));
            glBindImageTexture(0, bufferA, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
            glBindImageTexture(1, bufferB, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
            glDispatchCompute(imageSize.x / 32 + 1, imageSize.y / 32 + 1, 1);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            std::swap(bufferA, bufferB);
        }

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mBufferKernelIndexs9x9);
        mBlurZ->SetInt("indexesSize", 81);
        mBlurZ->SetInt("filterInterval", 1);
        glBindImageTexture(0, bufferA, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
        glBindImageTexture(1, bufferB, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
        glDispatchCompute(imageSize.x / 32 + 1, imageSize.y / 32 + 1, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }

    void DepthFilter::PreCalculate() {
        // 预计算权重[0, 2σ]
        mBufferSize = glm::ivec2(128, 128);
        mWeightBuffer.resize(mBufferSize.x * mBufferSize.y);
        int p = 0;
        for (int i = 0; i < mBufferSize.y; i++) {
            for (int j = 0; j < mBufferSize.x; j++) {
                float_t d1 = (float(j) / mBufferSize.x) * 3.0 * mSigma1;
                float_t d2 = (float(i) / mBufferSize.y) * 3.0 * mSigma2;
                mWeightBuffer[p] = CalculateWeight(d1, d2);
                p++;
            }
        }
    }

    void DepthFilter::BuildShader() {
        mBlurZ = new Glb::ComputeShader("BlurZ");
        std::string blurZPath = "../code/Fluid3d/Shaders/BlurZ.comp";
        mBlurZ->BuildFromFile(blurZPath);
    }

    void DepthFilter::Uploadbuffers() {
        // 传入纹理 
        glGenTextures(1, &mTexDepthFilter);
        glBindTexture(GL_TEXTURE_2D, mTexDepthFilter);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, mBufferSize.x, mBufferSize.y, 0, GL_RED, GL_FLOAT, mWeightBuffer.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        // 索引传入Buffer
        std::vector<glm::ivec2> kernelIndexes = GenerateIndexes(2);
        glGenBuffers(1, &mBufferKernelIndexs5x5);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, mBufferKernelIndexs5x5);
        glBufferData(GL_SHADER_STORAGE_BUFFER, kernelIndexes.size() * sizeof(glm::ivec2), kernelIndexes.data(), GL_STATIC_DRAW);

        kernelIndexes = GenerateIndexes(4);
        glGenBuffers(1, &mBufferKernelIndexs9x9);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, mBufferKernelIndexs9x9);
        glBufferData(GL_SHADER_STORAGE_BUFFER, kernelIndexes.size() * sizeof(glm::ivec2), kernelIndexes.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    float_t DepthFilter::CalculateWeight(float_t d1, float_t d2) {
        return std::exp(-std::pow(d1, 2) / (2.0 * std::pow(mSigma1, 2)) - std::pow(d2, 2) / (2.0 * std::pow(mSigma2, 2)));
    }


    std::vector<glm::ivec2> DepthFilter::GenerateIndexes(int32_t halfkernelSize) {
        std::vector<glm::ivec2> kernelIndexes = std::vector<glm::ivec2>(std::pow(halfkernelSize * 2 + 1, 2));
        int p = 0;
        for (int j = -halfkernelSize; j <= halfkernelSize; j++) {
            for (int i = -halfkernelSize; i <= halfkernelSize; i++) {
                kernelIndexes[p] = glm::ivec2(i, j);
                p++;
            }
        }
        return kernelIndexes;
    }

}


