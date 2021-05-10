#include "Utility.h"

#include <cmath>

namespace RenderDog
{
	const float fEpsilon = 0.000001f;

	bool floatEqual(float lhs, float rhs, float epsilon)
	{
		return std::abs(lhs - rhs) < epsilon;
	}
}