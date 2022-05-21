///////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Utility.h
//Written by Xiang Weikang
///////////////////////////////////

#pragma once

#include <cstdint>

namespace RenderDog
{
	struct Vector3;
	struct Vector4;

	extern const float RD_FLT_EPSILON;
	extern const float RD_PI;

	bool		FloatEqual(float lhs, float rhs, float epsilon = RD_FLT_EPSILON);

	uint32_t	ConvertColorToUInt32(const Vector4& color);
	Vector4		ConvertRGBAColorToARGBColor(const Vector4& RGBA);

	float		GetArea2(const Vector3& Pos1, const Vector3& Pos2, const Vector3& Pos3);

	float		Clamp(float fValue, float fMin, float fMax);

	float		AngleToRadians(float angle);

}// namespace RenderDog