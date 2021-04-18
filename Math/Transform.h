///////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Transform.h
//Written by Xiang Weikang
///////////////////////////////////

#pragma once

#include "Matrix.h"

namespace RenderDog
{
	Matrix4x4 GetTranslationMatrix(float xOffset, float yOffset, float zOffset)
	{
		Matrix4x4 translateMat = GetIdentityMatrix();
		translateMat(3, 0) = xOffset;
		translateMat(3, 1) = yOffset;
		translateMat(3, 2) = zOffset;

		return translateMat;
	}

	Matrix4x4 GetScaleMatrix(float xScale, float yScale, float zScale)
	{
		Matrix4x4 scaleMat = GetIdentityMatrix();
		scaleMat(0, 0) = xScale;
		scaleMat(1, 1) = yScale;
		scaleMat(2, 2) = zScale;

		return scaleMat;
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

	Matrix4x4 GetPerspProjectionMatrixLH(float Fov, float AspectRatio, float Near, float Far)
	{
		float RadianHalfFov = 0.5f * Fov / 180.0f * 3.1415926f;
		float SinHalfFov = std::sin(RadianHalfFov);
		float CosHalfFov = std::cos(RadianHalfFov);

		float HeightRatio = CosHalfFov / SinHalfFov;
		float WidthRatio = HeightRatio / AspectRatio;
		float RangeRatio = Far / (Far - Near);

		Matrix4x4 Result;
		Result(0, 0) = WidthRatio;
		Result(0, 1) = 0.0f;
		Result(0, 2) = 0.0f;
		Result(0, 3) = 0.0f;

		Result(1, 0) = 0.0f;
		Result(1, 1) = HeightRatio;
		Result(1, 2) = 0.0f;
		Result(1, 3) = 0.0f;

		Result(2, 0) = 0.0f;
		Result(2, 1) = 0.0f;
		Result(2, 2) = RangeRatio;
		Result(2, 3) = 1.0f;

		Result(3, 0) = 0.0f;
		Result(3, 1) = 0.0f;
		Result(3, 2) = -RangeRatio * Near;
		Result(3, 3) = 0.0f;

		return Result;
	}
}