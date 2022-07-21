////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Quaternion.h
//Written by Xiang Weikang
////////////////////////////////////////

#pragma once

namespace RenderDog
{
	class Quaternion
	{
	public:
		Quaternion() :
			x(0.0f),
			y(0.0f),
			z(0.0f),
			w(0.0f)
		{}

		~Quaternion() = default;

		Quaternion(float inX, float inY, float inZ, float inW) :
			x(inX),
			y(inY),
			z(inZ),
			w(inW)
		{}

		void Normalize();


	private:
		float x;
		float y;
		float z;
		float w;
	};

}// RenderDog
