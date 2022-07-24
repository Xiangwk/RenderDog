////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Quaternion.h
//Written by Xiang Weikang
////////////////////////////////////////

#pragma once

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

		void Normalize();
	};

	Quaternion Normalize(const Quaternion& quat);
	Quaternion Lerp(const Quaternion& lhs, const Quaternion& rhs, float lerpFactor);

}// namespace RenderDog
