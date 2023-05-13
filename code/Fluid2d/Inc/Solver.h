#ifndef SOLVER_H
#define SOLVER_H

#include "ParticalSystem.h"
#include "WCubicSpline2d.h"

namespace Fluid2d {

	class Solver {
	public:
		explicit Solver(Fluid2d::ParticalSystem& ps);
		~Solver();

		void Iterate();

	private:
		void UpdateDensityAndPressure();

		void InitAccleration();

		void UpdateViscosityAccleration();

		void UpdatePressureAccleration();

		void EulerIntegrate();

		void BoundaryCondition();

	private:
		ParticalSystem& mPs;
		WCubicSpline2d mW;

	};

}


#endif // !SOLVER_H

