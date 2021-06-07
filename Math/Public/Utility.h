#pragma once

#include <cstdint>
#include "Vector.h"

namespace RenderDog
{
	extern const float fEpsilon;

	bool floatEqual(float lhs, float rhs, float epsilon);

	uint32_t ConvertColorToUInt32(const Vector4& color);
	Vector4 ConvertRGBAColorToARGBColor(const Vector4& RGBA);
}