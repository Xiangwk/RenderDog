#include "Vector.h"

namespace RenderDog
{
	Vector2 operator+(const Vector2& lhs, const Vector2& rhs)
	{
		return Vector2(lhs.x + rhs.x, lhs.y + rhs.y);
	}

	Vector2 operator*(const Vector2& v, float f)
	{
		return Vector2(v.x * f, v.y * f);
	}

	Vector2 operator*(float f, const Vector2& v)
	{
		return v * f;
	}

	Vector2 operator/(const Vector2& v, float f)
	{
		return Vector2(v.x / f, v.y / f);
	}

	Vector3 operator+(const Vector3& lhs, const Vector3& rhs)
	{
		return Vector3(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
	}

	Vector3 operator-(const Vector3& lhs, const Vector3& rhs)
	{
		return Vector3(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
	}

	Vector3 operator*(const Vector3& lhs, const Vector3& rhs)
	{
		return Vector3(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z);
	}

	Vector3 operator*(float f, const Vector3& rhs)
	{
		return Vector3(f * rhs.x, f * rhs.y, f * rhs.z);
	}

	Vector3 operator*(const Vector3& lhs, float f)
	{
		return f * lhs;
	}

	Vector3 operator/(const Vector3& lhs, float f)
	{
		return Vector3(lhs.x / f, lhs.y / f, lhs.z / f);
	}

	Vector3 Normalize(const Vector3& vec)
	{
		float len = vec.Length();
		if (std::abs(len - 0.0f) < 0.000001f)
		{
			return Vector3(0);
		}

		float invLen = 1.0f / len;

		return Vector3(vec.x * invLen, vec.y * invLen, vec.z * invLen);
	}

	float DotProduct(const Vector3& lhs, const Vector3& rhs)
	{
		return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
	}

	Vector3 CrossProduct(const Vector3& lhs, const Vector3& rhs)
	{
		Vector3 result;
		result.x = lhs.y * rhs.z - lhs.z * rhs.y;
		result.y = lhs.z * rhs.x - lhs.x * rhs.z;
		result.z = lhs.x * rhs.y - lhs.y * rhs.x;

		return result;
	}

	Vector4 operator+(const Vector4& lhs, const Vector4& rhs)
	{
		return Vector4(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w);
	}

	Vector4 operator-(const Vector4& lhs, const Vector4& rhs)
	{
		return Vector4(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w);
	}

	Vector4 operator*(const Vector4& lhs, float f)
	{
		return Vector4(lhs.x * f, lhs.y * f, lhs.z * f, lhs.w * f);
	}

	Vector4 operator*(float f, const Vector4& rhs)
	{
		return rhs * f;
	}

	Vector4 operator/(const Vector4& lhs, float rhs)
	{
		return Vector4(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs, lhs.w / rhs);
	}
}