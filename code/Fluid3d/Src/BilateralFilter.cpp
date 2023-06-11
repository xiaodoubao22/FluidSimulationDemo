#include "BilateralFilter.h"

namespace Fluid3d {
	BilaterialFilter::BilaterialFilter(float_t sigma1, float_t sigma2) {
		mSigma1 = sigma1;
		mSigma2 = sigma2;
	}

	BilaterialFilter::~BilaterialFilter() {

	}

	void BilaterialFilter::PreCalculate(int32_t halfKernelSize) {
		// ‘§º∆À„»®÷ÿ[0, 2¶“]
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

	float_t BilaterialFilter::CalculateWeight(float_t d1, float_t d2) {
		return std::exp(-std::pow(d1, 2) / (2.0 * std::pow(mSigma1, 2)) - std::pow(d2, 2) / (2.0 * std::pow(mSigma2, 2)));
	}

	float_t* BilaterialFilter::GetWeightBuffer() {
		return mWeightBuffer.data();
	}

	glm::ivec2 BilaterialFilter::GetWeightBufferSize() {
		return mBufferSize;
	}

	std::vector<glm::ivec2> BilaterialFilter::GenerateIndexes(int32_t halfkernelSize) {
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


