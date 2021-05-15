#pragma once

#include <cstdint>

namespace RenderDog
{
	extern const float fEpsilon;

	bool floatEqual(float lhs, float rhs, float epsilon);

	uint32_t ConvertFloatColorToUInt32(const float* color);
}