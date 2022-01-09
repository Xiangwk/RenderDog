////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Vertex.h
//Written by Xiang Weikang
////////////////////////////////////////

#pragma once

#include "Vector.h"

namespace RenderDog
{
	struct SimpleVertex
	{
		Vector3 position;
		Vector3 color;

		SimpleVertex() = default;

		SimpleVertex(float px, float py, float pz,
					 float cx, float cy, float cz):
			position(px, py, pz),
			color(cx, cy, cz)
		{}
	};

	struct LocalVertex : public SimpleVertex
	{
		Vector3 normal;
		Vector4 tangent;    //w: hand party
		Vector2 texcoord;

		LocalVertex() = default;

		LocalVertex(float px, float py, float pz,
					float cx, float cy, float cz,
					float nx, float ny, float nz,
					float tx, float ty, float tz,
					float tcx, float tcy) :
			SimpleVertex(px, py, pz, cx, cy, cz),
			normal(nx, ny, nz),
			tangent(tx, ty, tz, 1.0f),
			texcoord(tcx, tcy)
		{}
	};

}// namespace RenderDog
