///////////////////////////////////
//RenderDog <·,·>
//FileName: SoftwareRender3D.h
//Written by Xiang Weikang
///////////////////////////////////

#pragma once

#include <windows.h>
#include <cstdint>

//#define RD_DEBUG_RASTERIZATION

namespace RenderDog
{
#pragma region Enum
	enum class SR_PRIMITIVE_TOPOLOGY
	{
		LINE_LIST,
		TRIANGLE_LIST
	};

	enum class SR_RESOURCE_DIMENSION
	{
		UNKNOWN = 0,
		BUFFER = 1,
		TEXTURE2D = 2,
	};

	enum class SR_RTV_DIMENSION
	{
		UNKNOWN = 0,
		BUFFER = 1,
		TEXTURE2D = 2,
	};

	enum class SR_DSV_DIMENSION
	{
		UNKNOWN = 0,
		TEXTURE2D = 1,
	};

	enum class SR_SRV_DIMENSION
	{
		UNKNOWN = 0,
		BUFFER = 1,
		TEXTURE2D = 2,
	};

	enum class SR_FORMAT
	{
		UNKNOWN = 0,
		R8G8B8A8_UNORM = 1,
		R32G32B32A32_FLOAT = 2,
		R32_FLOAT = 3
	};

	enum class SR_BIND_FLAG
	{
		BIND_UNKNOWN = 0,
		BIND_VERTEX_BUFFER = 1,
		BIND_INDEX_BUFFER = 2,
		BIND_CONSTANT_BUFFER = 3,
		BIND_SHADER_RESOURCE = 4,
		BIND_RENDER_TARGET = 5,
		BIND_DEPTH_STENCIL = 6
	};

	enum class SR_FILTER
	{
		POINT = 0,
		BILINEAR
	};
#pragma endregion Enum

#pragma region Description
	struct SwapChainDesc
	{
		uint32_t	width;
		uint32_t	height;
		SR_FORMAT	format;
		HWND		hOutputWindow;

		SwapChainDesc() :
			width(0),
			height(0),
			format(SR_FORMAT::UNKNOWN),
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

	struct SRBufferDesc
	{
		uint32_t		byteWidth;
		SR_BIND_FLAG	bindFlag;

		SRBufferDesc() :
			byteWidth(0),
			bindFlag(SR_BIND_FLAG::BIND_UNKNOWN)
		{}
	};

	struct SRTexture2DDesc
	{
		uint32_t		width;
		uint32_t		height;
		SR_FORMAT		format;

		SRTexture2DDesc() :
			width(0),
			height(0),
			format(SR_FORMAT::UNKNOWN)
		{}
	};

	struct SRSamplerDesc
	{
		SR_FILTER		filter;

		SRSamplerDesc() :
			filter(SR_FILTER::POINT)
		{}
	};

	struct SRRenderTargetViewDesc
	{
		SR_FORMAT			format;
		SR_RTV_DIMENSION	viewDimension;

		SRRenderTargetViewDesc() :
			format(SR_FORMAT::UNKNOWN),
			viewDimension(SR_RTV_DIMENSION::UNKNOWN)
		{}
	};

	struct SRDepthStencilViewDesc
	{
		SR_FORMAT format;
		SR_DSV_DIMENSION viewDimension;

		SRDepthStencilViewDesc() :
			format(SR_FORMAT::UNKNOWN),
			viewDimension(SR_DSV_DIMENSION::UNKNOWN)
		{}
	};

	struct SRShaderResourceViewDesc
	{
		SR_FORMAT format;
		SR_SRV_DIMENSION viewDimension;
	};
#pragma endregion Description

	struct SRSubResourceData
	{
		const void* pSysMem;
		uint32_t	sysMemPitch;

		SRSubResourceData() :
			pSysMem(nullptr),
			sysMemPitch(0)
		{}
	};

	struct SRViewport
	{
		float topLeftX;
		float topLeftY;
		float width;
		float height;
		float minDepth;
		float maxDepth;
	};

#pragma region Interface

	class ISRUnknown
	{
	public:
		virtual void AddRef() = 0;
		virtual void Release() = 0;
	};

	class ISRVertexShader : public ISRUnknown
	{
	public:
	};

	class ISRPixelShader : public ISRUnknown
	{
	public:
	};

	class ISFResource : public ISRUnknown
	{
	public:
		virtual void GetType(SR_RESOURCE_DIMENSION* pResDimension) = 0;
	};

	class ISRBuffer : public ISFResource
	{
	public:
		virtual void GetDesc(SRBufferDesc* pDesc) = 0;
	};

	class ISRTexture2D : public ISFResource
	{
	public:
		virtual void GetDesc(SRTexture2DDesc* pDesc) = 0;

		//以下两个接口为临时添加，后续需要删除
		virtual void*& GetData() = 0;
		virtual const void* GetData() const = 0;
	};

	class ISRSamplerState : public ISRUnknown
	{
		virtual void GetDesc(SRSamplerDesc* pDesc) = 0;
	};

	class ISRView : public ISRUnknown
	{
	public:
		virtual void GetResource(ISFResource** ppResource) = 0;
	};

	class ISRRenderTargetView : public ISRView
	{
	public:
		virtual void GetDesc(SRRenderTargetViewDesc* pDesc) = 0;
	};

	class ISRDepthStencilView : public ISRView
	{
	public:
		virtual void GetDesc(SRDepthStencilViewDesc* pDesc) = 0;
	};

	class ISRShaderResourceView : public ISRView
	{
	public:
		virtual void GetDesc(SRShaderResourceViewDesc* pDesc) = 0;
	};

#pragma region Device
	class ISRDevice : public ISRUnknown
	{
	public:
		virtual bool CreateTexture2D(const SRTexture2DDesc* pDesc, const SRSubResourceData* pInitData, ISRTexture2D** ppTexture) = 0;
		virtual bool CreateRenderTargetView(ISFResource* pResource, const SRRenderTargetViewDesc* pDesc, ISRRenderTargetView** ppRenderTargetView) = 0;
		virtual bool CreateDepthStencilView(ISFResource* pResource, const SRDepthStencilViewDesc* pDesc, ISRDepthStencilView** ppDepthStencilView) = 0;
		virtual bool CreateShaderResourceView(ISFResource* pResource, const SRShaderResourceViewDesc* pDesc, ISRShaderResourceView** ppShaderResourceView) = 0;
		virtual bool CreateBuffer(const SRBufferDesc* pDesc, const SRSubResourceData* pInitData, ISRBuffer** ppBuffer) = 0;
		virtual bool CreateVertexShader(ISRVertexShader** ppVertexShader) = 0;
		virtual bool CreatePixelShader(ISRPixelShader** ppPixelShader) = 0;
		virtual bool CreateSamplerState(const SRSamplerDesc* pDesc, ISRSamplerState** ppSamplerState) = 0;
	};

	class ISRDeviceContext : public ISRUnknown
	{
	public:
		virtual void IASetVertexBuffer(ISRBuffer* pVB) = 0;
		virtual void IASetIndexBuffer(ISRBuffer* pIB) = 0;
		virtual void IASetPrimitiveTopology(SR_PRIMITIVE_TOPOLOGY topology) = 0;

		virtual	void UpdateSubresource(ISFResource* pDstResource, const void* pSrcData, uint32_t srcRowPitch, uint32_t srcDepthPitch) = 0;

		virtual void VSSetShader(ISRVertexShader* pVS) = 0;
		virtual void VSSetConstantBuffer(uint32_t startSlot, ISRBuffer* const* ppConstantBuffer) = 0;
		virtual void PSSetConstantBuffer(uint32_t startSlot, ISRBuffer* const* ppConstantBuffer) = 0;
		virtual void PSSetShader(ISRPixelShader* pPS) = 0;
		virtual void PSSetShaderResource(ISRShaderResourceView* const* ppShaderResourceView) = 0;
		virtual void PSSetSampler(uint32_t startSlot, ISRSamplerState* const* ppSamplerState) = 0;

		virtual void RSSetViewport(const SRViewport* pViewport) = 0;

		virtual void OMSetRenderTarget(ISRRenderTargetView* pRenderTargetView, ISRDepthStencilView* pDepthStencilView) = 0;
		virtual void ClearRenderTargetView(ISRRenderTargetView* pRenderTargetView, const float* clearColor) = 0;
		virtual void ClearDepthStencilView(ISRDepthStencilView* pDepthStencilView, float fDepth) = 0;
		virtual void Draw() = 0;
		virtual void DrawIndex(uint32_t nIndexNum) = 0;
		virtual void DrawLineWithDDA(float fPos1X, float fPos1Y, float fPos2X, float fPos2Y, const float* lineColor) = 0;
	};
#pragma endregion Device

	class ISRSwapChain : public ISRUnknown
	{
	public:
		virtual bool GetBuffer(void** ppSurface) = 0;
		virtual bool ResizeBuffers(uint32_t bufferCnts, uint32_t width, uint32_t height, SR_FORMAT format) = 0;

		virtual void GetDesc(SwapChainDesc* pDesc) = 0;

		virtual void Present() = 0;
	};
#pragma endregion Interface

	bool CreateDeviceAndSwapChain(ISRDevice** pDevice, ISRDeviceContext** pDeviceContext, ISRSwapChain** ppSwapChain, const SwapChainDesc* pSwapChainDesc);
}