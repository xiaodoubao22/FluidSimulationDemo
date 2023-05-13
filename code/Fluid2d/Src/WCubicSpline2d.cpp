#include "WCubicSpline2d.h"
#include <glm/ext/scalar_constants.hpp>

namespace Fluid2d {

	WCubicSpline2d::WCubicSpline2d(float h) {
		mH = h;
		mH2 = h * h;
		mSigma = 40.0 / (7.0 * glm::pi<float>() * mH2);
	}
	WCubicSpline2d::~WCubicSpline2d() {

	}

	float WCubicSpline2d::Value(float distance) {
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

	float WCubicSpline2d::Diff(float distance) {
		float r = std::abs(distance);
		float q = r / mH;
		float q2 = q * q;
		float q3 = q * q2;
		float res = 0.0f;
		if (q < 0.5) {
			res = 6.0f * (3.0f * q2 - 2.0f * q) / mH;
			res *= mSigma;
			return res;
		}
		else if (q >= 0.5 && q < 1.0f) {
			res = -6.0 * std::pow(1.0f - q, 2) / mH;
			res *= mSigma;
			return res;
		}
		return res;
	}

	glm::vec2 WCubicSpline2d::Grad(glm::vec2 radius) {
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
