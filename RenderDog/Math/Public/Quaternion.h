////////////////////////////////////////
// RenderDog <·,·>
// FileName: Quaternion.h
// Written by Xiang Weikang
////////////////////////////////////////

#pragma once

#include "Vector.h"

namespace RenderDog
{
	struct Quaternion
	{
	public:
		float x;
		float y;
		float z;
		float w;

	public:
		Quaternion() :
			x(0.0f),
			y(0.0f),
			z(0.0f),
			w(1.0f)
		{}

		~Quaternion() = default;

		Quaternion(float inX, float inY, float inZ, float inW) :
			x(inX),
			y(inY),
			z(inZ),
			w(inW)
		{}

		Quaternion& operator*=(const Quaternion& rhs);

		void		Normalize();
		float		Length() const;
		float		GetAngle() const;
		Vector3		GetRotateAxis() const;
	};

	Quaternion Normalize(const Quaternion& quat);
	//FIXME!!! Lerp的效果有些问题，需要查验
	Quaternion Lerp(const Quaternion& quat1, const Quaternion& quat2, float lerpFactor);
	Quaternion SLerp(const Quaternion& quat1, const Quaternion& quat2, float lerpFactor);

	const Quaternion operator*(const Quaternion& lhs, const Quaternion& rhs);

}// namespace RenderDog
