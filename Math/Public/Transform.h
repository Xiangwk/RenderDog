///////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Transform.h
//Written by Xiang Weikang
///////////////////////////////////

#pragma once

#include "Matrix.h"

namespace RenderDog
{
	Matrix4x4 GetTranslationMatrix(float xOffset, float yOffset, float zOffset);

	Matrix4x4 GetScaleMatrix(float xScale, float yScale, float zScale);

	Matrix4x4 GetRotationMatrix(float angle, const Vector3& rotAxis);

	Matrix4x4 GetLookAtMatrixLH(const Vector3& eyePos, const Vector3& focusPos, const Vector3& upDir);

	Matrix4x4 GetPerspProjectionMatrixLH(float fov, float aspectRatio, float near, float farPlane);
}