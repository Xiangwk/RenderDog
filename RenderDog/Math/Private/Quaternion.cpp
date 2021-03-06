////////////////////////////////////////
//RenderDog <??,??>
//FileName: Quaternion.cpp
//Written by Xiang Weikang
////////////////////////////////////////

#include "Quaternion.h"
#include "Utility.h"

#include <cmath>

namespace RenderDog
{
	Quaternion& Quaternion::operator*=(const Quaternion& rhs)
	{
		float resX = w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y;
		float resY = w * rhs.y - x * rhs.z + y * rhs.w + z * rhs.x;
		float resZ = w * rhs.z + x * rhs.y - y * rhs.x + z * rhs.w;
		float resW = w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z;

		x = resX;
		y = resY;
		z = resZ;
		w = resW;

		return *this;
	}

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

	float Quaternion::Length() const
	{
		return std::sqrt(x * x + y * y + z * z + w * w);
	}

	float Quaternion::GetAngle() const
	{
		return 2.0f * std::acosf(w);
	}

	Vector3 Quaternion::GetRotateAxis() const
	{
		Vector3 rotAxis(x, y, z);

		rotAxis.Normalize();

		return rotAxis;
	}

	Quaternion Normalize(const Quaternion& quat)
	{
		Quaternion result = quat;

		result.Normalize();

		return result;
	}

	Quaternion Lerp(const Quaternion& quat1, const Quaternion& quat2, float lerpFactor)
	{
		float x = quat1.x * (1.0f - lerpFactor) + quat2.x * lerpFactor;
		float y = quat1.y * (1.0f - lerpFactor) + quat2.y * lerpFactor;
		float z = quat1.z * (1.0f - lerpFactor) + quat2.z * lerpFactor;
		float w = quat1.w * (1.0f - lerpFactor) + quat2.w * lerpFactor;

		Quaternion result(x, y, z, w);

		return Normalize(result);
	}

	Quaternion SLerp(const Quaternion& quat1, const Quaternion& quat2, float lerpFactor)
	{
		float rawCosine = quat1.x * quat2.x + quat1.y * quat2.y + quat1.z * quat2.z + quat1.w * quat2.w;
		float cosine = rawCosine >= 0.0f ? rawCosine : -rawCosine;

		float scale0 = 0.0f;
		float scale1 = 0.0f;
		if (cosine < 0.9999f)
		{
			float omega = std::acosf(cosine);
			float invSin = 1.0f / std::sinf(omega);

			scale0 = std::sinf((1.0f - lerpFactor) * omega) * invSin;
			scale1 = std::sinf(lerpFactor * omega) * invSin;
		}
		else
		{
			scale0 = 1.0f - lerpFactor;
			scale1 = lerpFactor;
		}

		scale1 = rawCosine >= 0.0f ? scale1 : -scale1;

		Quaternion result;
		result.x = scale0 * quat1.x + scale1 * quat2.x;
		result.y = scale0 * quat1.y + scale1 * quat2.y;
		result.z = scale0 * quat1.z + scale1 * quat2.z;
		result.w = scale0 * quat1.w + scale1 * quat2.w;

		return Normalize(result);
	}

	const Quaternion operator*(const Quaternion& lhs, const Quaternion& rhs)
	{
		float resX = lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y;
		float resY = lhs.w * rhs.y - lhs.x * rhs.z + lhs.y * rhs.w + lhs.z * rhs.x;
		float resZ = lhs.w * rhs.z + lhs.x * rhs.y - lhs.y * rhs.x + lhs.z * rhs.w;
		float resW = lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z;

		return Quaternion(resX, resY, resZ, resW);
	}

}// namespace RenderDog