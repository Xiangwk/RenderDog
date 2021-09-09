///////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Matrix.cpp
//Written by Xiang Weikang
///////////////////////////////////

#include "Matrix.h"

namespace RenderDog
{
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
}