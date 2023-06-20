#ifndef PARAMETERS_H
#define PARAMETERS_H

namespace Fluid2d {
    namespace Para {
        const float gravity = 9.8f;
        const float density0 = 1000.0f;
        const float stiffness = 50.0f;
        const float exponent = 7.0f;
        const float viscosity = 0.05f;
        const float dt = 2e-4;
        const int substep = 10;
    }
}

#endif // !PARAMETERS_H
