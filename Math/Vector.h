///////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Vector.h
//Written by Xiang Weikang
///////////////////////////////////

#pragma once

#include <cmath>

namespace RenderDog
{
	class Vector3
	{
	public:
		Vector3() = default;
		~Vector3() = default;

		Vector3(const Vector3&) = default;
		Vector3& operator=(const Vector3&) = default;

		explicit Vector3(float f) :
			x(f), y(f), z(f)
		{}

		Vector3(float x, float y, float z) :
			x(x), y(y), z(z)
		{}

		float Length() const
		{
			return std::sqrt(x * x + y * y + z * z);
		}

		Vector3 operator-() const
		{
			return Vector3(-x, -y, -z);
		}

	public:
		float x;
		float y;
		float z;
	};

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

	class Vector4
	{
	public:
		Vector4() = default;
		~Vector4() = default;

		Vector4(const Vector4&) = default;
		Vector4& operator=(const Vector4&) = default;

		explicit Vector4(float f):
			x(f), y(f), z(f), w(f)
		{}

		Vector4(float x, float y, float z, float w) :
			x(x), y(y), z(z), w(w)
		{}

		Vector4(const Vector3& v3, float f) :
			x(v3.x), y(v3.y), z(v3.z), w(f)
		{}

	public:
		float x;
		float y;
		float z;
		float w;
	};

	Vector4 operator+(const Vector4& lhs, const Vector4& rhs)
	{
		return Vector4(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w);
	}

	Vector4 operator-(const Vector4& lhs, const Vector4& rhs)
	{
		return Vector4(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w);
	}
}