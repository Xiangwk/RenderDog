////////////////////////////////////////
//	RenderDog <¡¤,¡¤>
//	FileName: Bounding.h
//	Written by Xiang Weikang
//	Desc: Define Bounding Box, Bounding Sphere etc.
////////////////////////////////////////

#pragma once

#include "Vector.h"

namespace RenderDog
{
	struct BoundingSphere
	{
		Vector3	center;
		float	radius;

		BoundingSphere() :
			center(0.0f, 0.0f, 0.0f),
			radius(100.0f)
		{}
	};

}// namespace RenderDog
