#pragma once

#include <cstdint>
#include <vector>

#include "Resource.h"

#define DEBUG_RASTERIZATION 0

namespace RenderDog
{
	class VertexBuffer;
	class IndexBuffer;
	class VertexShader;
	class PixelShader;
	class Vector4;
	class Matrix4x4;
	class RenderTargetView;
	class DepthStencilView;
	class ShaderResourceView;
	class DirectionalLight;

	struct Viewport;
	struct Vertex;
	struct VSOutputVertex;

	enum class PrimitiveTopology
	{
		LINE_LIST,
		TRIANGLE_LIST
	};

	class IDeviceContext : public IResource
	{
	public:
		virtual void IASetVertexBuffer(VertexBuffer* pVB) = 0;
		virtual void IASetIndexBuffer(IndexBuffer* pIB) = 0;
		virtual void IASetPrimitiveTopology(PrimitiveTopology topology) = 0;

		virtual void VSSetShader(VertexShader* pVS) = 0;
		virtual void VSSetTransMats(const Matrix4x4* matWorld, const Matrix4x4* matView, const Matrix4x4* matProj) = 0;
		virtual void PSSetShader(PixelShader* pPS) = 0;
		virtual void PSSetShaderResource(ShaderResourceView* const* pSRV) = 0;
		virtual void PSSetMainLight(DirectionalLight* pLight) = 0;

		virtual void RSSetViewport(const Viewport* pVP) = 0;

		virtual void OMSetRenderTarget(RenderTargetView* pRenderTarget, DepthStencilView* pDepthStencil) = 0;
		virtual void ClearRenderTarget(RenderTargetView* pRenderTarget, const Vector4& clearColor) = 0;
		virtual void ClearDepthStencil(DepthStencilView* pDepthStencil, float fDepth) = 0;
		virtual void Draw() = 0;
		virtual void DrawIndex(uint32_t nIndexNum) = 0;

#if DEBUG_RASTERIZATION
		virtual bool CheckDrawPixelTiwce() = 0;
#endif
		virtual void DrawLineWithDDA(float fPos1X, float fPos1Y, float fPos2X, float fPos2Y, const Vector4& lineColor) = 0;
	};
}