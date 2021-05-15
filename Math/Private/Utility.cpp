#include "Utility.h"

#include <cmath>

namespace RenderDog
{
	const float fEpsilon = 0.000001f;

	bool floatEqual(float lhs, float rhs, float epsilon)
	{
		return std::abs(lhs - rhs) < epsilon;
	}

	uint32_t ConvertFloatColorToUInt32(const float* color)
	{
		return (uint32_t)(255 * color[0]) << 16 | (uint32_t)(255 * color[1]) << 8 | (uint32_t)(255 * color[2]);
	}
}