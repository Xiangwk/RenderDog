///////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Vertex.h
//Written by Xiang Weikang
///////////////////////////////////

#pragma once

#include "Vector.h"

namespace RenderDog
{
	struct Vertex
	{
		Vertex() = default;
		~Vertex() = default;

		Vertex(const Vertex& v) = default;
		Vertex& operator=(const Vertex& v) = default;

		Vector3 vPosition;
		Vector3 vColor;
		Vector2 vUV;
	};
}


