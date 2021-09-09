#include "Utility.h"
#include "Vector.h"

#include <cmath>

namespace RenderDog
{
	const float RD_FLT_EPSILON = 1e-6f;

	bool FloatEqual(float lhs, float rhs, float epsilon)
	{
		return std::abs(lhs - rhs) <= epsilon;
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

	float GetArea2(const Vector3& Pos1, const Vector3& Pos2, const Vector3& Pos3)
	{
		return Pos1.x * Pos2.y * Pos3.z + Pos1.y * Pos2.z * Pos3.x + Pos1.z * Pos2.x * Pos3.y
			- Pos1.z * Pos2.y * Pos3.x - Pos1.y * Pos2.x * Pos3.z - Pos1.x * Pos2.z * Pos3.y;
	}

	float Clamp(float fValue, float fMin, float fMax)
	{
		if (fValue < fMin)
		{
			return fMin;
		}
		else if (fValue > fMax)
		{
			return fMax;
		}
		else
		{
			return fValue;
		}
	}
}