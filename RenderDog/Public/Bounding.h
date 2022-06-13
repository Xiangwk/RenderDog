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
	struct AABB
	{
		Vector3 minPoint;
		Vector3 maxPoint;

		AABB() :
			minPoint(FLT_MAX),
			maxPoint(FLT_MIN)
		{}

		void Reset() { minPoint = Vector3(FLT_MAX); maxPoint = Vector3(FLT_MIN); }
	};

	struct BoundingSphere
	{
		Vector3	center;
		float	radius;

		BoundingSphere() :
			center(0.0f, 0.0f, 0.0f),
			radius(0.0f)
		{}

		void Reset() { center = Vector3(0.0f, 0.0f, 0.0f); radius = 0.0f; }
	};

}// namespace RenderDog
