///////////////////////////////////////////////////////////
// RenderDog <��,��>
// FileName: Primitive.h
// Written by Xiang Weikang
// Desc: Primitive Interface��һ��PrimitiveΪ���Ƶ���С��Ԫ
///////////////////////////////////////////////////////////

#pragma once

#include "Buffer.h"
#include "Shader.h"

namespace RenderDog
{
	class	ITexture2D;
	class	ISamplerState;
	class   IMaterialInstance;
	struct	BoundingSphere;
	struct	AABB;

	enum class PRIMITIVE_TYPE
	{
		SIMPLE_PRI = 0,
		STATIC_PRI,
		SKIN_PRI
	};

	struct PrimitiveRenderParam
	{
		IVertexBuffer*		pVB;
		IIndexBuffer*		pIB;

		IShader*			pVS;
		IShader*			pShadowVS;
		ShaderPerObjParam	PerObjParam;

		IMaterialInstance*	pMtlIns;

		PrimitiveRenderParam() :
			pVB(nullptr),
			pIB(nullptr),
			pVS(nullptr),
			pShadowVS(nullptr),
			PerObjParam(),
			pMtlIns(nullptr)
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