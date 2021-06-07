#include "Utility.h"

#include <cmath>

namespace RenderDog
{
	const float fEpsilon = 0.000001f;

	bool floatEqual(float lhs, float rhs, float epsilon)
	{
		return std::abs(lhs - rhs) < epsilon;
	}

	uint32_t ConvertColorToUInt32(const Vector4& color)
	{
		return (uint32_t)(255 * color.x) << 24 | (uint32_t)(255 * color.y) << 16 | (uint32_t)(255 * color.z) << 8 | (uint32_t)(255 * color.w);
	}

	Vector4 ConvertRGBAColorToARGBColor(const Vector4& RGBA)
	{
		Vector4 ARGB;
		ARGB.x = RGBA.w;
		ARGB.y = RGBA.x;
		ARGB.z = RGBA.y;
		ARGB.w = RGBA.z;

		return ARGB;
	}
}