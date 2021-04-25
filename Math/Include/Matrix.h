///////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Matrix.h
//Written by Xiang Weikang
///////////////////////////////////

#pragma once

#include "Vector.h"

namespace RenderDog
{
	class Matrix4x4
	{
	public:
		Matrix4x4() = default;
		~Matrix4x4() = default;

		Matrix4x4(const Matrix4x4& mat)
		{
			for (int i = 0; i < 4; ++i)
			{
				for (int j = 0; j < 4; ++j)
				{
					m_fData[i][j] = mat.m_fData[i][j];
				}
			}
		}

		Matrix4x4(float f00, float f01, float f02, float f03,
			float f10, float f11, float f12, float f13,
			float f20, float f21, float f22, float f23,
			float f30, float f31, float f32, float f33) :
			m00(f00), m01(f01), m02(f02), m03(f03),
			m10(f10), m11(f11), m12(f12), m13(f13),
			m20(f20), m21(f21), m22(f22), m23(f23),
			m30(f30), m31(f31), m32(f32), m33(f33)
		{}

		Matrix4x4(const Vector4& v0, const Vector4& v1, const Vector4& v2, const Vector4& v3) :
			m00(v0.x), m01(v0.y), m02(v0.z), m03(v0.w),
			m10(v1.x), m11(v1.y), m12(v1.z), m13(v1.w),
			m20(v2.x), m21(v2.y), m22(v2.z), m23(v2.w),
			m30(v3.x), m31(v3.y), m32(v3.z), m33(v3.w)
		{}

		float operator()(int r, int c) const { return m_fData[r][c]; }
		float& operator()(int r, int c) { return m_fData[r][c]; }

	private:
		union
		{
			struct  
			{
				float m00, m01, m02, m03;
				float m10, m11, m12, m13;
				float m20, m21, m22, m23;
				float m30, m31, m32, m33;
			};
			float m_fData[4][4];
		};
	};

	Vector4 operator*(const Vector4& vec, const Matrix4x4& mat);

	Matrix4x4 GetIdentityMatrix();
}