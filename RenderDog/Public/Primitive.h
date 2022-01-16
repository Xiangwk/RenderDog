////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Primitive.h
//Written by Xiang Weikang
//Desc: Primitive Interface
////////////////////////////////////////

#pragma once

#include "Buffer.h"
#include "Shader.h"

namespace RenderDog
{
	struct PrimitiveRenderParam
	{
		IVertexBuffer*	pVB;
		IIndexBuffer*	pIB;

		IShader*		pVS;
		IShader*		pPS;

		PrimitiveRenderParam() :
			pVB(nullptr),
			pIB(nullptr),
			pVS(nullptr),
			pPS(nullptr)
		{}
	};

	class IPrimitiveRenderer
	{
	public:
		virtual ~IPrimitiveRenderer() = default;

		virtual void	Render(const PrimitiveRenderParam& renderParam) = 0;
	};

	class IPrimitive
	{
	public:
		virtual ~IPrimitive() = default;

		virtual void Render(IPrimitiveRenderer* pPrimitiveRenderer) = 0;
	};

}// namespace RenderDog