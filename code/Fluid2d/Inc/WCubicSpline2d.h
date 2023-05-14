#ifndef W_CUBE_SPLINE_2D_H
#define W_CUBE_SPLINE_2D_H

#include <glm/glm.hpp>
#include <vector>

namespace Fluid2d {

	class WCubicSpline2d {
	public:
		WCubicSpline2d() = delete;
		explicit WCubicSpline2d(float h);
		~WCubicSpline2d();

		float Value(float distance);

		glm::vec2 Grad(glm::vec2 radius);
		
	private:
		float CalculateValue(float distance);

		glm::vec2 CalculateGrad(glm::vec2 radius);

	private:
		float mH;
		float mH2;
		float mSigma;
		glm::uvec2 mBufferSize;
		std::vector<std::vector<glm::vec2>> mGradBuffer;
		std::vector<float> mValueBuffer;
	};

}




#endif // !W_CUBE_SPLINE_2D_H
