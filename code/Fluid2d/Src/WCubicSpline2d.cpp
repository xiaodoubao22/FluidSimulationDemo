#include "WCubicSpline2d.h"
#include <glm/ext/scalar_constants.hpp>
#include <iostream>

namespace Fluid2d {

	WCubicSpline2d::WCubicSpline2d(float h) {
		mH = h;
		mH2 = h * h;
		mSigma = 40.0 / (7.0 * glm::pi<float>() * mH2);

		mBufferSize = glm::uvec2(128, 128);
		mGradBuffer = std::vector<std::vector<glm::vec2>>(mBufferSize.x, std::vector<glm::vec2>(mBufferSize.y));
		mValueBuffer = std::vector<float>(mBufferSize.x);

		for (int i = 0; i < mBufferSize.x; i++) {
			for (int j = 0; j < mBufferSize.y; j++) {
				float x = ((float)i + 0.5f) * mH / mBufferSize.x;
				float y = ((float)j + 0.5f) * mH / mBufferSize.y;
				glm::vec2 radius(x, y);
				mGradBuffer[i][j] = CalculateGrad(radius);
			}
		}

		for (int i = 0; i < mBufferSize.x; i++) {
			float distance = ((float)i + 0.5f) * mH / mBufferSize.x;
			mValueBuffer[i] = CalculateValue(distance);
		}
	}

	WCubicSpline2d::~WCubicSpline2d() {
		
	}

	float WCubicSpline2d::Value(float distance) {
		float res = 0;
		int i = (std::abs(distance) * mBufferSize.x / mH);
		if (i >= mBufferSize.x) {
			return res;
		}
		res = mValueBuffer[i];
		return res;
	}

	glm::vec2 WCubicSpline2d::Grad(glm::vec2 radius) {
		glm::vec2 res(0.0f, 0.0f);

		int i = (std::abs(radius.x) * mBufferSize.x / mH);
		int j = (std::abs(radius.y) * mBufferSize.x / mH);

		if (i >= mBufferSize.x || j >= mBufferSize.y) {
			return res;
		}

		res = mGradBuffer[i][j];

		if (radius.x < 0) {
			res.x = -res.x;
		}
		if (radius.y < 0) {
			res.y = -res.y;
		}

		return res;
	}

	float WCubicSpline2d::CalculateValue(float distance) {
		float r = std::abs(distance);
		float q = r / mH;
		float q2 = q * q;
		float q3 = q * q2;
		float res = 0.0f;
		if (q < 0.5f) {
			res = 6.0f * (q3 - q2) + 1.0f;
			res *= mSigma;
			return res;
		}
		else if (q >= 0.5f && q < 1.0f) {
			res = 1.0f - q;
			res = std::pow(res, 3) * 2.0f;
			res *= mSigma;
			return res;
		}
		return res;
	}

	glm::vec2 WCubicSpline2d::CalculateGrad(glm::vec2 radius) {
		glm::vec2 res(0.0f, 0.0f);
		float distance = glm::length(radius);
		if (distance < 1e-5) {
			return res;
		}

		float q = distance / mH;
		glm::vec2 qGrad = radius / (mH * distance);

		if (q < 0.5f) {
			res = 6.0f * (3.0f * q * q - 2.0f * q) * mSigma * qGrad;
			return res;
		}
		else if (q >= 0.5 && q < 1.0f) {
			res = -6.0f * std::powf(1.0f - q, 2) * mSigma * qGrad;
			return res;
		}
		return res;
	}

}
