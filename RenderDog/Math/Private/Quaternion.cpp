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

}// namespace RenderDog