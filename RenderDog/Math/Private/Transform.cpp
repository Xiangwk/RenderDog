///////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Transform.cpp
//Written by Xiang Weikang
///////////////////////////////////

#include "Transform.h"
#include "Quaternion.h"
#include "Utility.h"

namespace RenderDog
{
	Matrix4x4 GetTranslationMatrix(float xOffset, float yOffset, float zOffset)
	{
		Matrix4x4 translateMatrix;
		translateMatrix.Identity();

		translateMatrix(3, 0) = xOffset;
		translateMatrix(3, 1) = yOffset;
		translateMatrix(3, 2) = zOffset;

		return translateMatrix;
	}

	Matrix4x4 GetScaleMatrix(float xScale, float yScale, float zScale)
	{
		Matrix4x4 scaleMatrix;
		scaleMatrix.Identity();

		scaleMatrix(0, 0) = xScale;
		scaleMatrix(1, 1) = yScale;
		scaleMatrix(2, 2) = zScale;

		return scaleMatrix;
	}

	Matrix4x4 GetRotationMatrix(float fAngle, const Vector3& rotAxis)
	{
		float radian = AngleToRadians(fAngle);
		float sinA = std::sinf(radian);
		float cosA = std::cosf(radian);

		Vector3 normRotAxis = Normalize(rotAxis);

		Matrix4x4 rotateMatrix;
		rotateMatrix.Identity();

		rotateMatrix(0, 0) = cosA + (1.0f - cosA) * normRotAxis.x * normRotAxis.x;
		rotateMatrix(1, 0) = (1.0f - cosA) * normRotAxis.x * normRotAxis.y - sinA * normRotAxis.z;
		rotateMatrix(2, 0) = (1.0f - cosA) * normRotAxis.x * normRotAxis.z + sinA * normRotAxis.y;

		rotateMatrix(0, 1) = (1.0f - cosA) * normRotAxis.x * normRotAxis.y + sinA * normRotAxis.z;
		rotateMatrix(1, 1) = cosA + (1.0f - cosA) * normRotAxis.y * normRotAxis.y;
		rotateMatrix(2, 1) = (1.0f - cosA) * normRotAxis.y * normRotAxis.z - sinA * normRotAxis.x;
		
		rotateMatrix(0, 2) = (1.0f - cosA) * normRotAxis.x * normRotAxis.z - sinA * normRotAxis.y;
		rotateMatrix(1, 2) = (1.0f - cosA) * normRotAxis.y * normRotAxis.z + sinA * normRotAxis.x;
		rotateMatrix(2, 2) = cosA + (1.0f - cosA) * normRotAxis.z * normRotAxis.z;

		return rotateMatrix;
	}

	Matrix4x4 GetRotationMatrix(float xAngle, float yAngle, float zAngle)
	{
		Matrix4x4 result = GetRotationMatrix(xAngle, Vector3(1.0f, 0.0f, 0.0f));
		result = result * GetRotationMatrix(yAngle, Vector3(0.0f, 1.0f, 0.0));
		result = result * GetRotationMatrix(zAngle, Vector3(0.0f, 0.0f, 1.0f));

		return result;
	}

	Matrix4x4 GetRotationMatrix(const Quaternion& quat)
	{
		Quaternion normalizeQuat = Normalize(quat);

		float x = normalizeQuat.x;
		float y = normalizeQuat.y;
		float z = normalizeQuat.z;
		float w = normalizeQuat.w;

		Matrix4x4 rotationMatrix;
		rotationMatrix(0, 0) = 1.0f - 2.0f * y * y - 2.0f * z * z;
		rotationMatrix(0, 1) = 2.0f * x * y + 2.0f * w * z;
		rotationMatrix(0, 2) = 2.0f * x * z - 2.0f * w * y;
		rotationMatrix(0, 3) = 0.0f;

		rotationMatrix(1, 0) = 2.0f * x * y - 2.0f * w * z;
		rotationMatrix(1, 1) = 1.0f - 2.0f * x * x - 2.0f * z * z;
		rotationMatrix(1, 2) = 2.0f * y * z + 2.0f * w * x;
		rotationMatrix(1, 3) = 0.0f;

		rotationMatrix(2, 0) = 2.0f * x * z + 2.0f * w * y;
		rotationMatrix(2, 1) = 2.0f * y * z - 2.0f * w * x;
		rotationMatrix(2, 2) = 1.0f - 2.0f * x * x - 2.0f * y * y;
		rotationMatrix(2, 3) = 0.0f;

		rotationMatrix(3, 0) = 0.0f;
		rotationMatrix(3, 1) = 0.0f;
		rotationMatrix(3, 2) = 0.0f;
		rotationMatrix(3, 3) = 1.0f;

		return rotationMatrix;
	}

	Matrix4x4 GetLookAtMatrixLH(const Vector3& eyePos, const Vector3& focusPos, const Vector3& upDir)
	{
		Vector3 camZAxis = focusPos - eyePos;
		camZAxis = Normalize(camZAxis);

		Vector3 camXAxis = CrossProduct(upDir, camZAxis);
		camXAxis = Normalize(camXAxis);

		Vector3 camYAxis = CrossProduct(camZAxis, camXAxis);

		Vector4 vec0(camXAxis.x, camYAxis.x, camZAxis.x, 0.0f);
		Vector4 vec1(camXAxis.y, camYAxis.y, camZAxis.y, 0.0f);
		Vector4 vec2(camXAxis.z, camYAxis.z, camZAxis.z, 0.0f);
		Vector4 vec3(DotProduct(-eyePos, camXAxis), DotProduct(-eyePos, camYAxis), DotProduct(-eyePos, camZAxis), 1.0f);

		return Matrix4x4(vec0, vec1, vec2, vec3);
	}

	Matrix4x4 GetPerspectiveMatrixLH(float fov, float aspectRatio, float near, float farPlane)
	{
		float radianHalfFov = 0.5f * AngleToRadians(fov);
		float sinHalfFov = std::sinf(radianHalfFov);
		float cosHalfFov = std::cosf(radianHalfFov);

		float heightRatio = cosHalfFov / sinHalfFov;
		float widthRatio = heightRatio / aspectRatio;
		float rangeRatio = farPlane / (farPlane - near);

		Matrix4x4 perspectiveMatrix;
		perspectiveMatrix(0, 0) = widthRatio;
		perspectiveMatrix(0, 1) = 0.0f;
		perspectiveMatrix(0, 2) = 0.0f;
		perspectiveMatrix(0, 3) = 0.0f;

		perspectiveMatrix(1, 0) = 0.0f;
		perspectiveMatrix(1, 1) = heightRatio;
		perspectiveMatrix(1, 2) = 0.0f;
		perspectiveMatrix(1, 3) = 0.0f;

		perspectiveMatrix(2, 0) = 0.0f;
		perspectiveMatrix(2, 1) = 0.0f;
		perspectiveMatrix(2, 2) = rangeRatio;
		perspectiveMatrix(2, 3) = 1.0f;

		perspectiveMatrix(3, 0) = 0.0f;
		perspectiveMatrix(3, 1) = 0.0f;
		perspectiveMatrix(3, 2) = -rangeRatio * near;
		perspectiveMatrix(3, 3) = 0.0f;

		return perspectiveMatrix;
	}

	Matrix4x4 GetOrthographicMatrixLH(float viewLeft, float viewRight, float viewBottom, float viewTop, float nearZ, float farZ)
	{
		Matrix4x4 orthoMatrix;
		orthoMatrix(0, 0) = 2.0f / (viewRight - viewLeft);
		orthoMatrix(0, 1) = 0.0f;
		orthoMatrix(0, 2) = 0.0f;
		orthoMatrix(0, 3) = 0.0f;

		orthoMatrix(1, 0) = 0.0f;
		orthoMatrix(1, 1) = 2.0f / (viewTop - viewBottom);
		orthoMatrix(1, 2) = 0.0f;
		orthoMatrix(1, 3) = 0.0f;

		orthoMatrix(2, 0) = 0.0f;
		orthoMatrix(2, 1) = 0.0f;
		orthoMatrix(2, 2) = 1.0f / (farZ - nearZ);
		orthoMatrix(2, 3) = 0.0f;

		orthoMatrix(3, 0) = -(viewRight + viewLeft) / (viewRight - viewLeft);
		orthoMatrix(3, 1) = -(viewTop + viewBottom) / (viewTop - viewBottom);
		orthoMatrix(3, 2) = -nearZ / (farZ - nearZ);
		orthoMatrix(3, 3) = 1.0f;

		return orthoMatrix;
	}

	Matrix4x4 GetTransformation(const Vector3& translation, const Vector3& scales, const Vector3& eulers)
	{
		Matrix4x4 scaleMatrix = GetScaleMatrix(scales.x, scales.y, scales.z);
		Matrix4x4 rotationMatrix = GetRotationMatrix(eulers.x, eulers.y, eulers.z);
		Matrix4x4 translationMatrix = GetTranslationMatrix(translation.x, translation.y, translation.z);

		return scaleMatrix * rotationMatrix * translationMatrix;
	}

	Matrix4x4 GetTransformation(const Vector3& translation, const Vector3& scales, const Quaternion& rotQuat)
	{
		Matrix4x4 scaleMatrix = GetScaleMatrix(scales.x, scales.y, scales.z);
		Matrix4x4 rotationMatrix = GetRotationMatrix(rotQuat);
		Matrix4x4 translationMatrix = GetTranslationMatrix(translation.x, translation.y, translation.z);

		return scaleMatrix * rotationMatrix * translationMatrix;
	}
}