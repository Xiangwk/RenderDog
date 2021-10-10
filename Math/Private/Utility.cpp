#include "Utility.h"
#include "Vector.h"

#include <cmath>
#include <float.h>

namespace RenderDog
{
	//const float RD_FLT_EPSILON = 1e-6f;
	const float RD_FLT_EPSILON = FLT_EPSILON;

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

	float GetArea2(const Vector3& pos1, const Vector3& pos2, const Vector3& pos3)
	{
		return pos1.x * pos2.y * pos3.z + pos1.y * pos2.z * pos3.x + pos1.z * pos2.x * pos3.y
			- pos1.z * pos2.y * pos3.x - pos1.y * pos2.x * pos3.z - pos1.x * pos2.z * pos3.y;
	}

	float Clamp(float value, float min, float max)
	{
		if (value < min)
		{
			return min;
		}
		else if (value > max)
		{
			return max;
		}
		else
		{
			return value;
		}
	}
}