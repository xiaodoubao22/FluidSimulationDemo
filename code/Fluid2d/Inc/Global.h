#ifndef GLOBAL_H
#define GLOBAL_H
#include <chrono>

namespace Glb {
	const float gravity = 9.8f;
	const float density0 = 1000.0f;
	const float stiffness = 50.0f;
	const float exponent = 7.0f;
	const float viscosity = 0.05f;
	const float dt = 2e-4;


	class Timer {
	private:
		std::chrono::system_clock::time_point mStartPoint;
	public:
		void Start() {
			mStartPoint = std::chrono::system_clock::now();
		}

		int GetTime() {
			auto dur = std::chrono::system_clock::now() - mStartPoint;
			return std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
		}
	};
}


#endif // !GLOBAL_H




