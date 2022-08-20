///////////////////////////////////////////////////////////
// RenderDog <·,·>
// FileName: Primitive.h
// Written by Xiang Weikang
// Desc: Primitive Interface，一个Primitive为绘制的最小单元
///////////////////////////////////////////////////////////

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
		STATIC_PRI,
		SKIN_PRI
	};

	struct PrimitiveRenderParam
	{
		IVertexBuffer*		pVB;
		IIndexBuffer*		pIB;

		ITexture2D*			pDiffuseTexture;
		ISamplerState*		pDiffuseTextureSampler;

		ITexture2D*			pNormalTexture;
		ISamplerState*		pNormalTextureSampler;

		IShader*			pVS;
		IShader*			pShadowVS;
		ShaderPerObjParam	PerObjParam;

		PrimitiveRenderParam() :
			pVB(nullptr),
			pIB(nullptr),
			pDiffuseTexture(nullptr),
			pDiffuseTextureSampler(nullptr),
			pNormalTexture(nullptr),
			pNormalTextureSampler(nullptr),
			pVS(nullptr),
			pShadowVS(nullptr),
			PerObjParam()
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