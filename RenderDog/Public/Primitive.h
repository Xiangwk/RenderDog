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
	class	ITexture2D;
	class	ISamplerState;
	struct	BoundingSphere;
	struct	AABB;

	enum class PRIMITIVE_TYPE
	{
		SIMPLE_PRI = 0,
		OPAQUE_PRI
	};

	struct PrimitiveRenderParam
	{
		IVertexBuffer*		pVB;
		IIndexBuffer*		pIB;
		IConstantBuffer*	pPerObjCB;

		ITexture2D*			pDiffuseTexture;
		ISamplerState*		pDiffuseTextureSampler;

		ITexture2D*			pNormalTexture;
		ISamplerState*		pNormalTextureSampler;

		IShader*			pVS;
		IShader*			pPS;

		PrimitiveRenderParam() :
			pVB(nullptr),
			pIB(nullptr),
			pPerObjCB(nullptr),
			pDiffuseTexture(nullptr),
			pDiffuseTextureSampler(nullptr),
			pNormalTexture(nullptr),
			pNormalTextureSampler(nullptr),
			pVS(nullptr),
			pPS(nullptr)
		{}
	};

	class IPrimitiveRenderer
	{
	public:
		virtual ~IPrimitiveRenderer() = default;

		virtual void				Render(const PrimitiveRenderParam& renderParam) = 0;
	};

	class IPrimitive
	{
	public:
		virtual ~IPrimitive() = default;

		virtual void				Render(IPrimitiveRenderer* pPrimitiveRenderer) = 0;

		virtual PRIMITIVE_TYPE		GetPriType() const = 0;
		virtual const AABB&			GetAABB() const = 0;
	};

}// namespace RenderDog