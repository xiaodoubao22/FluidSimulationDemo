#ifndef W_CUBE_SPLINE_2D_H
#define W_CUBE_SPLINE_2D_H

#include <glm/glm.hpp>

namespace Fluid2d {

	class WCubicSpline2d {
	public:
		WCubicSpline2d() = delete;
		explicit WCubicSpline2d(float h);
		~WCubicSpline2d();

		float Value(float distance);

		float Diff(float distance);

		glm::vec2 Grad(glm::vec2 radius);

	private:
		float mH;
		float mH2;
		float mSigma;
	};

}




#endif // !W_CUBE_SPLINE_2D_H
