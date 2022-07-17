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

		Vector2(const Vector2&) = default;
		Vector2& operator=(const Vector2&) = default;

		Vector2(Vector2&&) = default;
		Vector2& operator=(Vector2&&) = default;

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

	Vector2		operator+(const Vector2& lhs, const Vector2& rhs);
	Vector2		operator-(const Vector2& lhs, const Vector2& rhs);
	Vector2		operator*(const Vector2& lhs, const Vector2& rhs);
	Vector2		operator*(const Vector2& v, float f);
	Vector2		operator*(float f, const Vector2& v);
	Vector2		operator/(const Vector2& v, float f);

	bool		operator==(const Vector2& lhs, const Vector2& rhs);
	bool		operator!=(const Vector2& lhs, const Vector2& rhs);
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

		Vector3(const Vector3&) = default;
		Vector3& operator=(const Vector3&) = default;

		Vector3(Vector3&&) = default;
		Vector3& operator=(Vector3&&) = default;

		explicit Vector3(float f) :
			x(f), 
			y(f), 
			z(f)
		{}

		Vector3(float x, float y, float z) :
			x(x), 
			y(y), 
			z(z)
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

	Vector3		operator+(const Vector3& lhs, const Vector3& rhs);
	Vector3		operator-(const Vector3& lhs, const Vector3& rhs);
	Vector3		operator*(const Vector3& lhs, const Vector3& rhs);
	Vector3		operator*(float f, const Vector3& rhs);
	Vector3		operator*(const Vector3& lhs, float f);
	Vector3		operator/(const Vector3& lhs, float f);

	bool		operator==(const Vector3& lhs, const Vector3& rhs);
	bool		operator!=(const Vector3& lhs, const Vector3& rhs);

	Vector3		Normalize(const Vector3& v);
	float		DotProduct(const Vector3& lhs, const Vector3& rhs);
	Vector3		CrossProduct(const Vector3& lhs, const Vector3& rhs);
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

		Vector4(const Vector4&) = default;
		Vector4& operator=(const Vector4&) = default;

		Vector4(Vector4&&) = default;
		Vector4& operator=(Vector4&&) = default;

		explicit Vector4(float f):
			x(f), 
			y(f), 
			z(f), 
			w(f)
		{}

		Vector4(float x, float y, float z, float w) :
			x(x), 
			y(y), 
			z(z),
			w(w)
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

	Vector4		Normalize(const Vector4& vec);

	Vector4		operator+(const Vector4& lhs, const Vector4& rhs);
	Vector4		operator-(const Vector4& lhs, const Vector4& rhs);
	Vector4		operator*(const Vector4& lhs, const Vector4& rhs);
	Vector4		operator*(const Vector4& lhs, float f);
	Vector4		operator*(float f, const Vector4& rhs);
	Vector4		operator/(const Vector4& lhs, float rhs);

	bool		operator==(const Vector4& lhs, const Vector4& rhs);
	bool		operator!=(const Vector4& lhs, const Vector4& rhs);
#pragma endregion Vector4

} // End namespace RenderDog