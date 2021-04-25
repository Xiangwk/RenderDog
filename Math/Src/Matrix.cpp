#include "Matrix.h"

namespace RenderDog
{
	Vector4 operator*(const Vector4& vec, const Matrix4x4& mat)
	{
		Vector4 result;
		result.x = vec.x * mat(0, 0) + vec.y * mat(1, 0) + vec.z * mat(2, 0) + vec.w * mat(3, 0);
		result.y = vec.x * mat(0, 1) + vec.y * mat(1, 1) + vec.z * mat(2, 1) + vec.w * mat(3, 1);
		result.z = vec.x * mat(0, 2) + vec.y * mat(1, 2) + vec.z * mat(2, 2) + vec.w * mat(3, 2);
		result.w = vec.x * mat(0, 3) + vec.y * mat(1, 3) + vec.z * mat(2, 3) + vec.w * mat(3, 3);

		return result;
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