////////////////////////////////////////
//RenderDog <��,��>
//FileName: Vertex.h
//Written by Xiang Weikang
////////////////////////////////////////

#pragma once

#include "Vector.h"

namespace RenderDog
{
	enum class VERTEX_TYPE
	{
		SIMPLE,
		STANDARD
	};

	struct SimpleVertex
	{
		Vector3 position;
		Vector4 color;

		SimpleVertex() = default;

		SimpleVertex(float px, float py, float pz,
					 float cx, float cy, float cz, float cw):
			position(px, py, pz),
			color(cx, cy, cz, cw)
		{}

		SimpleVertex(const Vector3& pos, const Vector4& col) :
			position(pos),
			color(col)
		{}
	};

	struct StandardVertex : public SimpleVertex
	{
		Vector3 normal;
		Vector4 tangent;    //w: hand party
		Vector2 texcoord;

		StandardVertex() = default;

		StandardVertex(float px, float py, float pz,
					float cx, float cy, float cz, float cw,
					float nx, float ny, float nz,
					float tx, float ty, float tz,
					float tcx, float tcy) :
			SimpleVertex(px, py, pz, cx, cy, cz, cw),
			normal(nx, ny, nz),
			tangent(tx, ty, tz, 1.0f),
			texcoord(tcx, tcy)
		{}

		StandardVertex(const Vector3& pos, const Vector4& color, const Vector3& norm, const Vector4& tan, const Vector2& tex) :
			SimpleVertex(pos, color),
			normal(norm),
			tangent(tan),
			texcoord(tex)
		{}
	};

}// namespace RenderDog
