#pragma once

#include <windows.h>
#include <cstdint>

namespace RenderDog
{
	class IDevice;
	class VertexShader;
	class PixelShader;
	class DirectionalLight;

	struct Vertex;
	struct Viewport;
	struct ShaderResourceTexture;

#pragma region Enum
	enum class RD_PRIMITIVE_TOPOLOGY
	{
		LINE_LIST,
		TRIANGLE_LIST
	};

	enum class RD_RESOURCE_DIMENSION
	{
		UNKNOWN = 0,
		BUFFER = 1,
		TEXTURE2D = 2,
	};

	enum class RD_RTV_DIMENSION
	{
		UNKNOWN = 0,
		BUFFER = 1,
		TEXTURE2D = 2,
	};

	enum class RD_DSV_DIMENSION
	{
		UNKNOWN = 0,
		TEXTURE2D = 1,
	};

	enum class RD_SRV_DIMENSION
	{
		UNKNOWN = 0,
		BUFFER = 1,
		TEXTURE2D = 2,
	};

	enum class RD_FORMAT
	{
		UNKNOWN = 0,
		R8G8B8A8_UNORM = 1,
		R32G32B32A32_FLOAT = 2,
		R32_FLOAT = 3
	};

	enum class RD_BIND_FLAG
	{
		BIND_UNKNOWN = 0,
		BIND_VERTEX_BUFFER = 1,
		BIND_INDEX_BUFFER = 2,
		BIND_CONSTANT_BUFFER = 3,
		BIND_SHADER_RESOURCE = 4,
		BIND_RENDER_TARGET = 5,
		BIND_DEPTH_STENCIL = 6
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

	struct BufferDesc
	{
		uint32_t		byteWidth;
		RD_BIND_FLAG	bindFlag;

		BufferDesc() :
			byteWidth(0),
			bindFlag(RD_BIND_FLAG::BIND_UNKNOWN)
		{}
	};

	struct Texture2DDesc
	{
		uint32_t		width;
		uint32_t		height;
		RD_FORMAT		format;

		Texture2DDesc() :
			width(0),
			height(0),
			format(RD_FORMAT::UNKNOWN)
		{}
	};

	struct RenderTargetViewDesc
	{
		RD_FORMAT		format;
		RD_RTV_DIMENSION	viewDimension;

		RenderTargetViewDesc() :
			format(RD_FORMAT::UNKNOWN),
			viewDimension(RD_RTV_DIMENSION::UNKNOWN)
		{}
	};

	struct DepthStencilViewDesc
	{
		RD_FORMAT format;
		RD_DSV_DIMENSION viewDimension;

		DepthStencilViewDesc() :
			format(RD_FORMAT::UNKNOWN),
			viewDimension(RD_DSV_DIMENSION::UNKNOWN)
		{}
	};

	struct ShaderResourceViewDesc
	{
		RD_FORMAT format;
		RD_SRV_DIMENSION viewDimension;
	};
#pragma endregion Description

	struct SubResourceData
	{
		const void* pSysMem;
		uint32_t	sysMemPitch;

		SubResourceData() :
			pSysMem(nullptr),
			sysMemPitch(0)
		{}
	};

	struct Viewport
	{
		float topLeftX;
		float topLeftY;
		float width;
		float height;
		float minDepth;
		float maxDepth;
	};

#pragma region Interface

	class IUnknown
	{
	public:
		virtual void AddRef() = 0;
		virtual void Release() = 0;
	};

	class IVertexShader : public IUnknown
	{
	public:
	};

	class IPixelShader : public IUnknown
	{
	public:
	};

	class IResource : public IUnknown
	{
	public:
		virtual void GetType(RD_RESOURCE_DIMENSION* pResDimension) = 0;
	};

	class IBuffer : public IResource
	{
	public:
		virtual void GetDesc(BufferDesc* pDesc) = 0;
	};

	class ITexture2D : public IResource
	{
	public:
		virtual void GetDesc(Texture2DDesc* pDesc) = 0;

		//以下两个接口为临时添加，后续需要删除
		virtual void*& GetData() = 0;
		virtual const void* GetData() const = 0;
	};

	class IView : public IUnknown
	{
	public:
		virtual void GetResource(IResource** ppResource) = 0;
	};

	class IRenderTargetView : public IView
	{
	public:
		virtual void GetDesc(RenderTargetViewDesc* pDesc) = 0;
	};

	class IDepthStencilView : public IView
	{
	public:
		virtual void GetDesc(DepthStencilViewDesc* pDesc) = 0;
	};

	class IShaderResourceView : public IView
	{
	public:
		virtual void GetDesc(ShaderResourceViewDesc* pDesc) = 0;
	};

#pragma region Device
	class IDevice : public IUnknown
	{
	public:
		virtual bool CreateTexture2D(const Texture2DDesc* pDesc, const SubResourceData* pInitData, ITexture2D** ppTexture) = 0;
		virtual bool CreateRenderTargetView(IResource* pResource, const RenderTargetViewDesc* pDesc, IRenderTargetView** ppRenderTargetView) = 0;
		virtual bool CreateDepthStencilView(IResource* pResource, const DepthStencilViewDesc* pDesc, IDepthStencilView** ppDepthStencilView) = 0;
		virtual bool CreateShaderResourceView(IResource* pResource, const ShaderResourceViewDesc* pDesc, IShaderResourceView** ppShaderResourceView) = 0;
		virtual bool CreateBuffer(const BufferDesc* pDesc, const SubResourceData* pInitData, IBuffer** ppBuffer) = 0;
		virtual bool CreateVertexShader(IVertexShader** ppVertexShader) = 0;
		virtual bool CreatePixelShader(IPixelShader** ppPixelShader) = 0;
	};

	class IDeviceContext : public IUnknown
	{
	public:
		virtual void IASetVertexBuffer(IBuffer* pVB) = 0;
		virtual void IASetIndexBuffer(IBuffer* pIB) = 0;
		virtual void IASetPrimitiveTopology(RD_PRIMITIVE_TOPOLOGY topology) = 0;

		virtual	void UpdateSubresource(IResource* pDstResource, const void* pSrcData, uint32_t srcRowPitch, uint32_t srcDepthPitch) = 0;

		virtual void VSSetShader(IVertexShader* pVS) = 0;
		virtual void VSSetConstantBuffer(IBuffer* const* ppConstantBuffer) = 0;
		virtual void PSSetShader(IPixelShader* pPS) = 0;
		virtual void PSSetShaderResource(IShaderResourceView* const* ppShaderResourceView) = 0;
		virtual void PSSetMainLight(DirectionalLight* pLight) = 0;

		virtual void RSSetViewport(const Viewport* pViewport) = 0;

		virtual void OMSetRenderTarget(IRenderTargetView* pRenderTargetView, IDepthStencilView* pDepthStencilView) = 0;
		virtual void ClearRenderTargetView(IRenderTargetView* pRenderTargetView, const float* clearColor) = 0;
		virtual void ClearDepthStencilView(IDepthStencilView* pDepthStencilView, float fDepth) = 0;
		virtual void Draw() = 0;
		virtual void DrawIndex(uint32_t nIndexNum) = 0;

#if DEBUG_RASTERIZATION
		virtual bool CheckDrawPixelTwice() = 0;
#endif
		virtual void DrawLineWithDDA(float fPos1X, float fPos1Y, float fPos2X, float fPos2Y, const float* lineColor) = 0;
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