#ifndef RUNGE_KUTTA_H
#define RUNGE_KUTTA_H

#include <cmath>
#include "vector.h"
#include "matrix.h"
#include "linklist.h"
#include "particle.h"
#include "eos.h"

namespace RK
{
	void bsq_second_order( double dx, EquationsOfMotion & eom, SystemState & system );
}

#endif
