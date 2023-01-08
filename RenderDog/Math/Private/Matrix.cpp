///////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Matrix.cpp
//Written by Xiang Weikang
///////////////////////////////////

#include "Matrix.h"

namespace RenderDog
{
	///////////////////////////////////////////////////////////////////////////////////
	// ----------------------       Matrix4x4         -------------------------------//
	///////////////////////////////////////////////////////////////////////////////////

	Matrix4x4::Matrix4x4()
	{
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				m_fData[i][j] = 0.0f;
			}
		}
	}

	Matrix4x4::~Matrix4x4()
	{}

	Matrix4x4::Matrix4x4(const Matrix4x4& mat)
	{
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				m_fData[i][j] = mat(i, j);
			}
		}
	}

	Matrix4x4& Matrix4x4::operator=(const Matrix4x4& mat)
	{
		if (this == &mat)
		{
			return *this;
		}

		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				m_fData[i][j] = mat(i, j);
			}
		}

		return *this;
	}

	Matrix4x4::Matrix4x4(float f00, float f01, float f02, float f03,
		float f10, float f11, float f12, float f13,
		float f20, float f21, float f22, float f23,
		float f30, float f31, float f32, float f33)
	{
		m_fData[0][0] = f00;
		m_fData[0][1] = f01;
		m_fData[0][2] = f02;
		m_fData[0][3] = f03;

		m_fData[1][0] = f10;
		m_fData[1][1] = f11;
		m_fData[1][2] = f12;
		m_fData[1][3] = f13;

		m_fData[2][0] = f20;
		m_fData[2][1] = f21;
		m_fData[2][2] = f22;
		m_fData[2][3] = f23;

		m_fData[3][0] = f30;
		m_fData[3][1] = f31;
		m_fData[3][2] = f32;
		m_fData[3][3] = f33;
	}

	Matrix4x4::Matrix4x4(const Vector4& v0, const Vector4& v1, const Vector4& v2, const Vector4& v3)
	{
		m_fData[0][0] = v0.x;
		m_fData[0][1] = v0.y;
		m_fData[0][2] = v0.z;
		m_fData[0][3] = v0.w;

		m_fData[1][0] = v1.x;
		m_fData[1][1] = v1.y;
		m_fData[1][2] = v1.z;
		m_fData[1][3] = v1.w;

		m_fData[2][0] = v2.x;
		m_fData[2][1] = v2.y;
		m_fData[2][2] = v2.z;
		m_fData[2][3] = v2.w;

		m_fData[3][0] = v3.x;
		m_fData[3][1] = v3.y;
		m_fData[3][2] = v3.z;
		m_fData[3][3] = v3.w;
	}

	void Matrix4x4::Identity()
	{
		m_fData[0][0] = 1.0f;
		m_fData[1][1] = 1.0f;
		m_fData[2][2] = 1.0f;
		m_fData[3][3] = 1.0f;

		m_fData[0][1] = 0.0f;
		m_fData[0][2] = 0.0f;
		m_fData[0][3] = 0.0f;

		m_fData[1][0] = 0.0f;
		m_fData[1][2] = 0.0f;
		m_fData[1][3] = 0.0f;
		
		m_fData[2][0] = 0.0f;
		m_fData[2][1] = 0.0f;
		m_fData[2][3] = 0.0f;
		
		m_fData[3][0] = 0.0f;
		m_fData[3][1] = 0.0f;
		m_fData[3][2] = 0.0f;
	}

	Vector3	Matrix4x4::GetTranslatePart()
	{
		return Vector3(m_fData[3][0], m_fData[3][1], m_fData[3][2]);
	}

	Vector3	Matrix4x4::GetScalePart()
	{
		Vector3 row0(m_fData[0][0], m_fData[0][1], m_fData[0][2]);
		float scale0 = row0.Length();

		Vector3 row1(m_fData[1][0], m_fData[1][1], m_fData[1][2]);
		float scale1 = row1.Length();

		Vector3 row2(m_fData[2][0], m_fData[2][1], m_fData[2][2]);
		float scale2 = row2.Length();

		return Vector3(scale0, scale1, scale2);
	}

	Matrix4x4 Matrix4x4::GetRotationPart()
	{
		Matrix4x4 rotateMatrix = *this;

		Vector3 scalePart = GetScalePart();

		rotateMatrix.m_fData[0][0] /= scalePart.x;
		rotateMatrix.m_fData[0][1] /= scalePart.x;
		rotateMatrix.m_fData[0][2] /= scalePart.x;

		rotateMatrix.m_fData[1][0] /= scalePart.y;
		rotateMatrix.m_fData[1][1] /= scalePart.y;
		rotateMatrix.m_fData[1][2] /= scalePart.y;

		rotateMatrix.m_fData[2][0] /= scalePart.z;
		rotateMatrix.m_fData[2][1] /= scalePart.z;
		rotateMatrix.m_fData[2][2] /= scalePart.z;

		rotateMatrix.m_fData[3][0] = 0.0f;
		rotateMatrix.m_fData[3][1] = 0.0f;
		rotateMatrix.m_fData[3][2] = 0.0f;

		return rotateMatrix;
	}

	Vector4 operator*(const Vector4& vec, const Matrix4x4& mat)
	{
		Vector4 result;
		result.x = vec.x * mat(0, 0) + vec.y * mat(1, 0) + vec.z * mat(2, 0) + vec.w * mat(3, 0);
		result.y = vec.x * mat(0, 1) + vec.y * mat(1, 1) + vec.z * mat(2, 1) + vec.w * mat(3, 1);
		result.z = vec.x * mat(0, 2) + vec.y * mat(1, 2) + vec.z * mat(2, 2) + vec.w * mat(3, 2);
		result.w = vec.x * mat(0, 3) + vec.y * mat(1, 3) + vec.z * mat(2, 3) + vec.w * mat(3, 3);

		return result;
	}

	Matrix4x4 operator*(const Matrix4x4& matLhs, const Matrix4x4& matRhs)
	{
		Matrix4x4 matResult = GetIdentityMatrix();

		float l00 = matLhs(0, 0), r00 = matRhs(0, 0);
		float l01 = matLhs(0, 1), r01 = matRhs(0, 1);
		float l02 = matLhs(0, 2), r02 = matRhs(0, 2);
		float l03 = matLhs(0, 3), r03 = matRhs(0, 3);

		float l10 = matLhs(1, 0), r10 = matRhs(1, 0);
		float l11 = matLhs(1, 1), r11 = matRhs(1, 1);
		float l12 = matLhs(1, 2), r12 = matRhs(1, 2);
		float l13 = matLhs(1, 3), r13 = matRhs(1, 3);

		float l20 = matLhs(2, 0), r20 = matRhs(2, 0);
		float l21 = matLhs(2, 1), r21 = matRhs(2, 1);
		float l22 = matLhs(2, 2), r22 = matRhs(2, 2);
		float l23 = matLhs(2, 3), r23 = matRhs(2, 3);

		float l30 = matLhs(3, 0), r30 = matRhs(3, 0);
		float l31 = matLhs(3, 1), r31 = matRhs(3, 1);
		float l32 = matLhs(3, 2), r32 = matRhs(3, 2);
		float l33 = matLhs(3, 3), r33 = matRhs(3, 3);

		matResult(0, 0) = l00 * r00 + l01 * r10 + l02 * r20 + l03 * r30;
		matResult(0, 1) = l00 * r01 + l01 * r11 + l02 * r21 + l03 * r31;
		matResult(0, 2) = l00 * r02 + l01 * r12 + l02 * r22 + l03 * r32;
		matResult(0, 3) = l00 * r03 + l01 * r13 + l02 * r23 + l03 * r33;

		matResult(1, 0) = l10 * r00 + l11 * r10 + l12 * r20 + l13 * r30;
		matResult(1, 1) = l10 * r01 + l11 * r11 + l12 * r21 + l13 * r31;
		matResult(1, 2) = l10 * r02 + l11 * r12 + l12 * r22 + l13 * r32;
		matResult(1, 3) = l10 * r03 + l11 * r13 + l12 * r23 + l13 * r33;

		matResult(2, 0) = l20 * r00 + l21 * r10 + l22 * r20 + l23 * r30;
		matResult(2, 1) = l20 * r01 + l21 * r11 + l22 * r21 + l23 * r31;
		matResult(2, 2) = l20 * r02 + l21 * r12 + l22 * r22 + l23 * r32;
		matResult(2, 3) = l20 * r03 + l21 * r13 + l22 * r23 + l23 * r33;

		matResult(3, 0) = l30 * r00 + l31 * r10 + l32 * r20 + l33 * r30;
		matResult(3, 1) = l30 * r01 + l31 * r11 + l32 * r21 + l33 * r31;
		matResult(3, 2) = l30 * r02 + l31 * r12 + l32 * r22 + l33 * r32;
		matResult(3, 3) = l30 * r03 + l31 * r13 + l32 * r23 + l33 * r33;

		return matResult;
	}

	bool operator==(const Matrix4x4& matLhs, const Matrix4x4& matRhs)
	{
		for (int i = 0; i < 4; ++i)
		{
			for(int j = 0; j < 4; ++j)
			{
				if (matLhs(i, j) != matRhs(i, j))
				{
					return false;
				}
			}
		}

		return true;
	}

	Matrix4x4 GetIdentityMatrix()
	{
		Matrix4x4 identityMat;
		identityMat(0, 0) = 1.0f;
		identityMat(0, 1) = 0.0f;
		identityMat(0, 2) = 0.0f;
		identityMat(0, 3) = 0.0f;

		identityMat(1, 0) = 0.0f;
		identityMat(1, 1) = 1.0f;
		identityMat(1, 2) = 0.0f;
		identityMat(1, 3) = 0.0f;

		identityMat(2, 0) = 0.0f;
		identityMat(2, 1) = 0.0f;
		identityMat(2, 2) = 1.0f;
		identityMat(2, 3) = 0.0f;

		identityMat(3, 0) = 0.0f;
		identityMat(3, 1) = 0.0f;
		identityMat(3, 2) = 0.0f;
		identityMat(3, 3) = 1.0f;

		return identityMat;
	}

	Matrix4x4 Transpose(const Matrix4x4& mat)
	{
		Matrix4x4 result = GetIdentityMatrix();
		result(0, 0) = mat(0, 0);
		result(0, 1) = mat(1, 0);
		result(0, 2) = mat(2, 0);
		result(0, 3) = mat(3, 0);

		result(1, 0) = mat(0, 1);
		result(1, 1) = mat(1, 1);
		result(1, 2) = mat(2, 1);
		result(1, 3) = mat(3, 1);

		result(2, 0) = mat(0, 2);
		result(2, 1) = mat(1, 2);
		result(2, 2) = mat(2, 2);
		result(2, 3) = mat(3, 2);

		result(3, 0) = mat(0, 3);
		result(3, 1) = mat(1, 3);
		result(3, 2) = mat(2, 3);
		result(3, 3) = mat(3, 3);
		
		return result;
	}
}// namespace RenderDog