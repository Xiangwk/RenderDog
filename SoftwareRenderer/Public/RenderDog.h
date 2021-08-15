#pragma once

#include <windows.h>
#include <cstdint>

namespace RenderDog
{
	class Texture2D;
	class RenderTargetView;
	class ShaderResourceView;
	class DepthStencilView;
	class VertexBuffer;
	class IndexBuffer;
	class VertexShader;
	class PixelShader;
	class Vector4;
	class Matrix4x4;
	class DirectionalLight;

	struct Texture2DDesc;
	struct RenderTargetDesc;
	struct VertexBufferDesc;
	struct IndexBufferDesc;
	struct Viewport;

	enum class RD_FORMAT
	{
		UNKNOWN			= 0,
		R8G8B8A8_UNORM	= 1
	};

	class IUnknown
	{
	public:
		virtual void AddRef() = 0;
		virtual void Release() = 0;
	};

	class IDevice : public IUnknown
	{
	public:
		virtual bool CreateTexture2D(const Texture2DDesc* pDesc, Texture2D** ppTexture) = 0;
		virtual bool CreateRenderTargetView(Texture2D* pTexture, const RenderTargetDesc* pDesc, RenderTargetView** ppRenderTarget) = 0;
		virtual bool CreateDepthStencilView(Texture2D* pTexture, DepthStencilView** ppDepthStencil) = 0;
		virtual bool CreateVertexBuffer(const VertexBufferDesc& vbDesc, VertexBuffer** ppVertexBuffer) = 0;
		virtual bool CreateIndexBuffer(const IndexBufferDesc& ibDesc, IndexBuffer** ppIndexBuffer) = 0;
		virtual bool CreateVertexShader(VertexShader** ppVertexShader) = 0;
		virtual bool CreatePixelShader(PixelShader** ppPixelShader) = 0;
	};

	enum class PrimitiveTopology
	{
		LINE_LIST,
		TRIANGLE_LIST
	};

	class IDeviceContext : public IUnknown
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
		virtual bool CheckDrawPixelTwice() = 0;
#endif
		virtual void DrawLineWithDDA(float fPos1X, float fPos1Y, float fPos2X, float fPos2Y, const Vector4& lineColor) = 0;
	};

	struct SwapChainDesc
	{
		uint32_t	Width;
		uint32_t	Height;
		RD_FORMAT	Format;
		HWND		OutputWindow;
	};

	class ISwapChain : public IUnknown
	{
	public:
		virtual bool GetBuffer(Texture2D** ppSurface) = 0;
		virtual void Present() = 0;
	};


	bool CreateDeviceAndSwapChain(IDevice** pDevice, IDeviceContext** pDeviceContext, ISwapChain** ppSwapChain, const SwapChainDesc* pSwapChainDesc);
}