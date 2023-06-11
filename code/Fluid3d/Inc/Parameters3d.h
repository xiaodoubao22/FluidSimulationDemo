#ifndef PARAMETERS_3D_H
#define PARAMETERS_3D_H

namespace Fluid3d {
	namespace Para3d {
		const float supportRadius = 0.025;
		const float particalRadius = 0.005;
		const float gravity = 9.8f;
		const float density0 = 1000.0f;
		const float stiffness = 5.0f;
		const float exponent = 7.0f;
		const float viscosity = 4e-6f;
		const float dt = 2e-4;
		const int substep = 25;
	}
}

#endif // !PARAMETERS_3D_H