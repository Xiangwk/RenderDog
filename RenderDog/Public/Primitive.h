////////////////////////////////////////
//RenderDog <��,��>
//FileName: Primitive.h
//Written by Xiang Weikang
//Desc: Primitive Interface
////////////////////////////////////////

#pragma once

#include "Buffer.h"
#include "Shader.h"

namespace RenderDog
{
	class	ITexture2D;
	class	ISamplerState;
	struct	BoundingSphere;

	enum class PRIMITIVE_TYPE
	{
		SIMPLE_PRI = 0,
		OPAQUE_PRI
	};

	struct PrimitiveRenderParam
	{
		IVertexBuffer*		pVB;
		IIndexBuffer*		pIB;
		IConstantBuffer*	pGlobalCB;
		IConstantBuffer*	pPerObjCB;
		IConstantBuffer*	pLightingCB;

		IShader*			pVS;
		IShader*			pPS;

		PrimitiveRenderParam() :
			pVB(nullptr),
			pIB(nullptr),
			pGlobalCB(nullptr),
			pPerObjCB(nullptr),
			pLightingCB(nullptr),
			pVS(nullptr),
			pPS(nullptr)
		{}
	};

	class IPrimitiveRenderer
	{
	public:
		virtual ~IPrimitiveRenderer() = default;

		virtual IConstantBuffer*	GetVSConstantBuffer() = 0;
		virtual IConstantBuffer*	GetLightingConstantbuffer() = 0;

		virtual void				Render(const PrimitiveRenderParam& renderParam, ITexture2D* pDiffuseTexture, ISamplerState* pSampler) = 0;
	};

	class IPrimitive
	{
	public:
		virtual ~IPrimitive() = default;

		virtual void				Render(IPrimitiveRenderer* pPrimitiveRenderer) = 0;
		
		virtual PRIMITIVE_TYPE		GetPriType() const = 0;
		virtual BoundingSphere		GetBoundingSphere() const = 0;
	};

}// namespace RenderDog