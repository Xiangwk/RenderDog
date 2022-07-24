///////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Transform.h
//Written by Xiang Weikang
///////////////////////////////////

#pragma once

#include "Matrix.h"

namespace RenderDog
{
	struct Quaternion;

	Matrix4x4	GetTranslationMatrix(float xOffset, float yOffset, float zOffset);

	Matrix4x4	GetScaleMatrix(float xScale, float yScale, float zScale);

	Matrix4x4	GetRotationMatrix(float angle, const Vector3& rotAxis);
	Matrix4x4	GetRotationMatrix(float xAngle, float yAngle, float zAngle);
	Matrix4x4	GetRotationMatrix(const Quaternion& quat);

	Matrix4x4	GetLookAtMatrixLH(const Vector3& eyePos, const Vector3& focusPos, const Vector3& upDir);

	Matrix4x4	GetPerspectiveMatrixLH(float fov, float aspectRatio, float near, float farPlane);
	Matrix4x4	GetOrthographicMatrixLH(float viewLeft, float viewRight, float viewBottom, float viewTop, float nearZ, float farZ);

	Matrix4x4	GetTransformation(const Vector3& translation, const Vector3& scales, const Vector3& eulers);
	Matrix4x4	GetTransformation(const Vector3& translation, const Vector3& scales, const Quaternion& rotQuat);
}