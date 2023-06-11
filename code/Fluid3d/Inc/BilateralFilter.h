#ifndef BILATERAL_FILTER_H
#define BILATERAL_FILTER_H

#include <vector>
#include <glm/glm.hpp>

namespace Fluid3d {
	class BilaterialFilter {
	public:
		BilaterialFilter(float_t sigma1, float_t sigma2);
		~BilaterialFilter();

		void PreCalculate(int32_t halfKernelSize);
		float_t CalculateWeight(float_t d1, float_t d2);

		float_t* GetWeightBuffer();
		glm::ivec2 GetWeightBufferSize();

	public:
		static std::vector<glm::ivec2> GenerateIndexes(int32_t halfkernelSize);

	private:
		std::vector<float_t> mWeightBuffer;
		glm::ivec2 mBufferSize;
	public:
		float_t mSigma1;
		float_t mSigma2;
	};
}


#endif // !BILATERAL_FILTER_H#pragma once
