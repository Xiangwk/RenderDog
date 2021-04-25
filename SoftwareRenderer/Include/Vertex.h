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

		Vector3 vPostion;
		Vector3 vColor;
	};
}


