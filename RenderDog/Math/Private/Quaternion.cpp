////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Quaternion.cpp
//Written by Xiang Weikang
////////////////////////////////////////

#include "Quaternion.h"
#include "Utility.h"

#include <cmath>

namespace RenderDog
{
	void Quaternion::Normalize()
	{
		float sqrSum = x * x + y * y + z * z + w * w;
		if (std::abs(sqrSum - 0.0f) < RD_FLT_EPSILON)
		{
			x = y = z = 0;
			w = 1;
		}

		float invSqrt = 1.0f / std::sqrt(sqrSum);
		x *= invSqrt;
		y *= invSqrt;
		z *= invSqrt;
		w *= invSqrt;
	}

	Quaternion Normalize(const Quaternion& quat)
	{
		Quaternion result = quat;

		result.Normalize();

		return result;
	}

	Quaternion Lerp(const Quaternion& lhs, const Quaternion& rhs, float lerpFactor)
	{
		float x = lhs.x * (1.0f - lerpFactor) + rhs.x * lerpFactor;
		float y = lhs.y * (1.0f - lerpFactor) + rhs.y * lerpFactor;
		float z = lhs.z * (1.0f - lerpFactor) + rhs.z * lerpFactor;
		float w = lhs.w * (1.0f - lerpFactor) + rhs.w * lerpFactor;

		return Quaternion(x, y, z, w);
	}

}// namespace RenderDog