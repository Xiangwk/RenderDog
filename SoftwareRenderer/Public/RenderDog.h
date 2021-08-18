#pragma once

#include <windows.h>
#include <cstdint>

#include "Texture.h"

namespace RenderDog
{
	class IDevice;
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

#pragma region Enum
	enum class PrimitiveTopology
	{
		LINE_LIST,
		TRIANGLE_LIST
	};

	enum class RESOURCE_DIMENSION
	{
		UNKNOWN = 0,
		BUFFER = 1,
		TEXTURE1D = 2,
		TEXTURE2D = 3,
		TEXTURE3D = 4
	};
#pragma endregion Enum

#pragma region Description
	struct SwapChainDesc
	{
		uint32_t	width;
		uint32_t	height;
		RD_FORMAT	format;
		HWND		hOutputWindow;

		SwapChainDesc() :
			width(0),
			height(0),
			format(RD_FORMAT::UNKNOWN),
			hOutputWindow(nullptr)
		{}

		SwapChainDesc(const SwapChainDesc& desc) :
			width(desc.width),
			height(desc.height),
			format(desc.format),
			hOutputWindow(desc.hOutputWindow)
		{}

		SwapChainDesc& operator=(const SwapChainDesc& desc)
		{
			width = desc.width;
			height = desc.height;
			format = desc.format;
			hOutputWindow = desc.hOutputWindow;

			return *this;
		}
	};

	struct Texture2DDesc
	{
		uint32_t		width;
		uint32_t		height;
		RD_FORMAT		format;
	};
#pragma endregion Description

#pragma region Interface

	class IUnknown
	{
	public:
		virtual void AddRef() = 0;
		virtual void Release() = 0;
	};

	class IDeviceChild : public IUnknown
	{
	public:
		virtual void GetDevice(IDevice** ppDevice) = 0;
		virtual void GetPrivateData(uint32_t* pDataSize, void* pData) = 0;
		virtual void SetPrivateData(uint32_t DataSize, const void* pData) = 0;
	};

	class IResource : public IDeviceChild
	{
	public:
		virtual void GetType(RESOURCE_DIMENSION* pResDimension) = 0;
	};

	class ITexture2D : public IResource
	{
	public:
		virtual void GetDesc(Texture2DDesc* pDesc) = 0;
	};

#pragma region Device
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

	class IDeviceContext : public IDeviceChild
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
#pragma endregion Device

	class ISwapChain : public IUnknown
	{
	public:
		virtual bool GetBuffer(void** ppSurface) = 0;
		virtual void GetDesc(SwapChainDesc* pDesc) = 0;
		virtual void Present() = 0;
	};
#pragma endregion Interface

	bool CreateDeviceAndSwapChain(IDevice** pDevice, IDeviceContext** pDeviceContext, ISwapChain** ppSwapChain, const SwapChainDesc* pSwapChainDesc);
}