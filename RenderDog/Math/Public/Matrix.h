///////////////////////////////////
//RenderDog <·,·>
//FileName: Matrix.h
//Written by Xiang Weikang
///////////////////////////////////

#pragma once

#include "Vector.h"

namespace RenderDog
{
	///////////////////////////////////////////////////////////////////////////////////
	// ----------------------       Matrix4x4         -------------------------------//
	///////////////////////////////////////////////////////////////////////////////////

	class Matrix4x4
	{
	public:
		Matrix4x4();		//全零
		~Matrix4x4();

		Matrix4x4(const Matrix4x4& mat);
		Matrix4x4& operator=(const Matrix4x4& mat);

		Matrix4x4(float f00, float f01, float f02, float f03,
				  float f10, float f11, float f12, float f13,
				  float f20, float f21, float f22, float f23,
				  float f30, float f31, float f32, float f33);

		//行向量
		Matrix4x4(const Vector4& v0, const Vector4& v1, const Vector4& v2, const Vector4& v3);

		float		operator()(int r, int c) const { return m_fData[r][c]; }
		float&		operator()(int r, int c) { return m_fData[r][c]; }
		Vector4		GetRow(int r) const { return Vector4(m_fData[r][0], m_fData[r][1], m_fData[r][2], m_fData[r][3]); }
		Vector4		GetCol(int c) const { return Vector4(m_fData[0][c], m_fData[1][c], m_fData[2][c], m_fData[3][c]); }

		void		Identity();

		Vector3		GetTranslatePart();
		Vector3		GetScalePart();
		Matrix4x4	GetRotationPart();

	private:
		float		m_fData[4][4];
	};

	Vector4			operator*(const Vector4& vec, const Matrix4x4& mat);
	Matrix4x4		operator*(const Matrix4x4& matLhs, const Matrix4x4& matRhs);

	bool			operator==(const Matrix4x4& matLhs, const Matrix4x4& matRhs);

	Matrix4x4		GetIdentityMatrix();

	Matrix4x4		Transpose(const Matrix4x4& mat);

}// namespace RenderDog