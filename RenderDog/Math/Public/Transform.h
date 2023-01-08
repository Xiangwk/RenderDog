///////////////////////////////////
//RenderDog <·,·>
//FileName: Transform.h
//Written by Xiang Weikang
///////////////////////////////////

#pragma once

namespace RenderDog
{
	struct	Vector3;
	struct	Quaternion;
	class	Matrix4x4;

	Matrix4x4	GetTranslationMatrix(float xOffset, float yOffset, float zOffset);

	Matrix4x4	GetScaleMatrix(float xScale, float yScale, float zScale);

	Matrix4x4	GetRotationMatrix(float angle, const Vector3& rotAxis);
	Matrix4x4	GetRotationMatrix(float xAngle, float yAngle, float zAngle);
	Matrix4x4	GetRotationMatrix(const Quaternion& quat);

				//LH表示使用左手系
	Matrix4x4	GetLookAtMatrixLH(const Vector3& eyePos, const Vector3& focusPos, const Vector3& upDir);

	Matrix4x4	GetPerspectiveMatrixLH(float fov, float aspectRatio, float near, float farPlane);
	Matrix4x4	GetOrthographicMatrixLH(float viewLeft, float viewRight, float viewBottom, float viewTop, float nearZ, float farZ);

	Matrix4x4	GetTransformMatrix(const Vector3& translation, const Vector3& scales, const Vector3& eulers);
	Matrix4x4	GetTransformMatrix(const Vector3& translation, const Vector3& scales, const Quaternion& rotQuat);

}// namespace RenderDog