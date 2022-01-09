////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: D3D11PrimitiveRenderer.h
//Written by Xiang Weikang
////////////////////////////////////////

#pragma once

#include <d3d11.h>

#include "Primitive.h"

namespace RenderDog
{
	class ID3D11PrimitiveRenderer : public IPrimitiveRenderer
	{
	public:
		virtual ~ID3D11PrimitiveRenderer() = default;

		virtual void Render(const PrimitiveRenderParam& renderParam) = 0;
	};

}// namespace RenderDog
