///////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Vector.h
//Written by Xiang Weikang
///////////////////////////////////

#pragma once

#include <cmath>

namespace RenderDog
{
	///////////////////////////////////////////////////////////////////////////////////
	// ----------------------       Vector2         ---------------------------------//
	///////////////////////////////////////////////////////////////////////////////////
#pragma region Vector2
	struct Vector2
	{
		float x;
		float y;

		Vector2() :
			x(0.0f),
			y(0.0f)
		{}

		~Vector2()
		{}

		Vector2(const Vector2& v) :
			x(v.x),
			y(v.y)
		{}

		Vector2& operator=(const Vector2& v)
		{
			x = v.x;
			y = v.y;

			return *this;
		}

		explicit Vector2(float f) :
			x(f),
			y(f)
		{}

		Vector2(float x, float y) :
			x(x), 
			y(y)
		{}

		Vector2 operator-() const
		{
			return Vector2(-x, -y);
		}

		Vector2& operator+=(const Vector2& rhs)
		{
			x += rhs.x;
			y += rhs.y;

			return *this;
		}

		Vector2& operator-=(const Vector2& rhs)
		{
			x -= rhs.x;
			y -= rhs.y;

			return *this;
		}

		Vector2& operator*=(float f)
		{
			x *= f;
			y *= f;

			return *this;
		}

		Vector2& operator*=(const Vector2& rhs)
		{
			x *= rhs.x;
			y *= rhs.y;

			return *this;
		}

		Vector2& operator/=(float f)
		{
			x /= f;
			y /= f;

			return *this;
		}
	};

	const Vector2	operator+(const Vector2& lhs, const Vector2& rhs);
	const Vector2	operator-(const Vector2& lhs, const Vector2& rhs);
	const Vector2	operator*(const Vector2& lhs, const Vector2& rhs);
	const Vector2	operator*(const Vector2& v, float f);
	const Vector2	operator*(float f, const Vector2& v);
	const Vector2	operator/(const Vector2& v, float f);

	bool			operator==(const Vector2& lhs, const Vector2& rhs);
	bool			operator!=(const Vector2& lhs, const Vector2& rhs);
#pragma endregion Vector2

	///////////////////////////////////////////////////////////////////////////////////
	// ----------------------       Vector3         ---------------------------------//
	///////////////////////////////////////////////////////////////////////////////////
#pragma region Vector3
	struct Vector3
	{
		float x;
		float y;
		float z;

		Vector3() :
			x(0.0f),
			y(0.0f),
			z(0.0f)
		{}

		~Vector3() = default;

		Vector3(const Vector3& v) :
			x(v.x),
			y(v.y),
			z(v.z)
		{}

		Vector3& operator=(const Vector3& v)
		{
			x = v.x;
			y = v.y;
			z = v.z;

			return *this;
		}

		explicit Vector3(float f) :
			x(f), 
			y(f), 
			z(f)
		{}

		Vector3(float inX, float inY, float inZ) :
			x(inX),
			y(inY),
			z(inZ)
		{}

		float Length() const
		{
			return std::sqrt(x * x + y * y + z * z);
		}

		Vector3 operator-() const
		{
			return Vector3(-x, -y, -z);
		}

		Vector3& operator+=(const Vector3& rhs)
		{
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;

			return *this;
		}

		Vector3& operator-=(const Vector3& rhs)
		{
			x -= rhs.x;
			y -= rhs.y;
			z -= rhs.z;

			return *this;
		}

		Vector3& operator*=(float f)
		{
			x *= f;
			y *= f;
			z *= f;

			return *this;
		}

		Vector3& operator*=(const Vector3& rhs)
		{
			x *= rhs.x;
			y *= rhs.y;
			z *= rhs.z;

			return *this;
		}

		Vector3& operator/=(float f)
		{
			x /= f;
			y /= f;
			z /= f;

			return *this;
		}

		float	Normalize(); //return length of this
	};

	const Vector3	operator+(const Vector3& lhs, const Vector3& rhs);
	const Vector3	operator-(const Vector3& lhs, const Vector3& rhs);
	const Vector3	operator*(const Vector3& lhs, const Vector3& rhs);
	const Vector3	operator*(float f, const Vector3& rhs);
	const Vector3	operator*(const Vector3& lhs, float f);
	const Vector3	operator/(const Vector3& lhs, float f);

	bool			operator==(const Vector3& lhs, const Vector3& rhs);
	bool			operator!=(const Vector3& lhs, const Vector3& rhs);

	Vector3		Normalize(const Vector3& v);
	float		DotProduct(const Vector3& lhs, const Vector3& rhs);
	Vector3		CrossProduct(const Vector3& lhs, const Vector3& rhs);

	Vector3		Lerp(const Vector3& lhs, const Vector3& rhs, float lerpFactor);
#pragma endregion Vector3

	///////////////////////////////////////////////////////////////////////////////////
	// ----------------------       Vector4         ---------------------------------//
	///////////////////////////////////////////////////////////////////////////////////
#pragma region Vector4
	struct Vector4
	{
		float x;
		float y;
		float z;
		float w;
	
		Vector4() :
			x(0.0f),
			y(0.0f),
			z(0.0f),
			w(0.0f)
		{}

		~Vector4() = default;

		Vector4(const Vector4& v) :
			x(v.x),
			y(v.y),
			z(v.z),
			w(v.w)
		{}

		Vector4& operator=(const Vector4& v)
		{
			x = v.x;
			y = v.y;
			z = v.z;
			w = v.w;

			return *this;
		}

		explicit Vector4(float f):
			x(f), 
			y(f), 
			z(f), 
			w(f)
		{}

		Vector4(float inX, float inY, float inZ, float inW) :
			x(inX),
			y(inY),
			z(inZ),
			w(inW)
		{}

		Vector4(const Vector3& v3, float f) :
			x(v3.x), 
			y(v3.y), 
			z(v3.z), 
			w(f)
		{}

		float	Length() const
		{
			return std::sqrt(x * x + y * y + z * z + w * w);
		}

		void	Normalize();
	};

	Vector4			Normalize(const Vector4& vec);

	const Vector4	operator+(const Vector4& lhs, const Vector4& rhs);
	const Vector4	operator-(const Vector4& lhs, const Vector4& rhs);
	const Vector4	operator*(const Vector4& lhs, const Vector4& rhs);
	const Vector4	operator*(const Vector4& lhs, float f);
	const Vector4	operator*(float f, const Vector4& rhs);
	const Vector4	operator/(const Vector4& lhs, float rhs);

	bool			operator==(const Vector4& lhs, const Vector4& rhs);
	bool			operator!=(const Vector4& lhs, const Vector4& rhs);

	Vector4			Lerp(const Vector4& lhs, const Vector4& rhs, float lerpFactor);
#pragma endregion Vector4

} // End namespace RenderDog