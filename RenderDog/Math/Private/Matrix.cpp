///////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Matrix.cpp
//Written by Xiang Weikang
///////////////////////////////////

#include "Matrix.h"

namespace RenderDog
{
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

	Vector4 operator*(const Vector4& vec, const Matrix4x4& mat)
	{
		Vector4 result;
		result.x = vec.x * mat(0, 0) + vec.y * mat(1, 0) + vec.z * mat(2, 0) + vec.w * mat(3, 0);
		result.y = vec.x * mat(0, 1) + vec.y * mat(1, 1) + vec.z * mat(2, 1) + vec.w * mat(3, 1);
		result.z = vec.x * mat(0, 2) + vec.y * mat(1, 2) + vec.z * mat(2, 2) + vec.w * mat(3, 2);
		result.w = vec.x * mat(0, 3) + vec.y * mat(1, 3) + vec.z * mat(2, 3) + vec.w * mat(3, 3);

		return result;
	}

	Matrix4x4 operator*(const Matrix4x4& matLHS, const Matrix4x4& matRHS)
	{
		Matrix4x4 matResult = GetIdentityMatrix();

		matResult(0, 0) = matLHS(0, 0) * matRHS(0, 0) + matLHS(0, 1) * matRHS(1, 0) + matLHS(0, 2) * matRHS(2, 0) + matLHS(0, 3) * matRHS(3, 0);
		matResult(0, 1) = matLHS(0, 0) * matRHS(0, 1) + matLHS(0, 1) * matRHS(1, 1) + matLHS(0, 2) * matRHS(2, 1) + matLHS(0, 3) * matRHS(3, 1);
		matResult(0, 2) = matLHS(0, 0) * matRHS(0, 2) + matLHS(0, 1) * matRHS(1, 2) + matLHS(0, 2) * matRHS(2, 2) + matLHS(0, 3) * matRHS(3, 2);
		matResult(0, 3) = matLHS(0, 0) * matRHS(0, 3) + matLHS(0, 1) * matRHS(1, 3) + matLHS(0, 2) * matRHS(2, 3) + matLHS(0, 3) * matRHS(3, 3);

		matResult(1, 0) = matLHS(1, 0) * matRHS(0, 0) + matLHS(1, 1) * matRHS(1, 0) + matLHS(1, 2) * matRHS(2, 0) + matLHS(1, 3) * matRHS(3, 0);
		matResult(1, 1) = matLHS(1, 0) * matRHS(0, 1) + matLHS(1, 1) * matRHS(1, 1) + matLHS(1, 2) * matRHS(2, 1) + matLHS(1, 3) * matRHS(3, 1);
		matResult(1, 2) = matLHS(1, 0) * matRHS(0, 2) + matLHS(1, 1) * matRHS(1, 2) + matLHS(1, 2) * matRHS(2, 2) + matLHS(1, 3) * matRHS(3, 2);
		matResult(1, 3) = matLHS(1, 0) * matRHS(0, 3) + matLHS(1, 1) * matRHS(1, 3) + matLHS(1, 2) * matRHS(2, 3) + matLHS(1, 3) * matRHS(3, 3);

		matResult(2, 0) = matLHS(2, 0) * matRHS(0, 0) + matLHS(2, 1) * matRHS(1, 0) + matLHS(2, 2) * matRHS(2, 0) + matLHS(2, 3) * matRHS(3, 0);
		matResult(2, 1) = matLHS(2, 0) * matRHS(0, 1) + matLHS(2, 1) * matRHS(1, 1) + matLHS(2, 2) * matRHS(2, 1) + matLHS(2, 3) * matRHS(3, 1);
		matResult(2, 2) = matLHS(2, 0) * matRHS(0, 2) + matLHS(2, 1) * matRHS(1, 2) + matLHS(2, 2) * matRHS(2, 2) + matLHS(2, 3) * matRHS(3, 2);
		matResult(2, 3) = matLHS(2, 0) * matRHS(0, 3) + matLHS(2, 1) * matRHS(1, 3) + matLHS(2, 2) * matRHS(2, 3) + matLHS(2, 3) * matRHS(3, 3);

		matResult(3, 0) = matLHS(3, 0) * matRHS(0, 0) + matLHS(3, 1) * matRHS(1, 0) + matLHS(3, 2) * matRHS(2, 0) + matLHS(3, 3) * matRHS(3, 0);
		matResult(3, 1) = matLHS(3, 0) * matRHS(0, 1) + matLHS(3, 1) * matRHS(1, 1) + matLHS(3, 2) * matRHS(2, 1) + matLHS(3, 3) * matRHS(3, 1);
		matResult(3, 2) = matLHS(3, 0) * matRHS(0, 2) + matLHS(3, 1) * matRHS(1, 2) + matLHS(3, 2) * matRHS(2, 2) + matLHS(3, 3) * matRHS(3, 2);
		matResult(3, 3) = matLHS(3, 0) * matRHS(0, 3) + matLHS(3, 1) * matRHS(1, 3) + matLHS(3, 2) * matRHS(2, 3) + matLHS(3, 3) * matRHS(3, 3);

		return matResult;
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
}