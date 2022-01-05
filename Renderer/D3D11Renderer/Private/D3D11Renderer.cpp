////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: D3D11Renderer.cpp
//Written by Xiang Weikang
////////////////////////////////////////

#include "D3D11Renderer.h"

namespace RenderDog
{
	class D3D11Renderer : public ID3D11Renderer
	{
	public:
		D3D11Renderer();
		virtual ~D3D11Renderer();

		virtual bool Init(const RendererInitDesc& desc);
		virtual void Release();

		virtual void Render();

		virtual bool OnResize(uint32_t width, uint32_t height);

	private:
		ID3D11Device*				m_pD3DDevice;
		ID3D11DeviceContext*		m_pD3DImmediateContext;
		IDXGISwapChain*				m_pSwapChain;

		ID3D11Texture2D*			m_pDepthStencilTexture;
		ID3D11RenderTargetView*		m_pRenderTargetView;
		ID3D11DepthStencilView*		m_pDepthStencilView;
		D3D11_VIEWPORT				m_ScreenViewport;
	};

	D3D11Renderer g_D3D11Renderer;
	IRenderer* g_pIRenderer = &g_D3D11Renderer;

	D3D11Renderer::D3D11Renderer() :
		m_pD3DDevice(nullptr),
		m_pD3DImmediateContext(nullptr),
		m_pSwapChain(nullptr),
		m_pDepthStencilTexture(nullptr),
		m_pRenderTargetView(nullptr),
		m_pDepthStencilView(nullptr),
		m_ScreenViewport()
	{}

	D3D11Renderer::~D3D11Renderer()
	{}

	bool D3D11Renderer::Init(const RendererInitDesc& desc)
	{
		UINT createDeviceFlag = 0;
#if defined(DEBUG) | defined(_DEBUG)
		createDeviceFlag |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		D3D_FEATURE_LEVEL featureLevel;
		HRESULT hr = D3D11CreateDevice(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			0,
			createDeviceFlag,
			nullptr,
			0,
			D3D11_SDK_VERSION,
			&m_pD3DDevice,
			&featureLevel,
			&m_pD3DImmediateContext);
		if (FAILED(hr))
		{
			MessageBox(nullptr, L"D3D11CreateDevice Failed!", L"ERROR", MB_OK);
			return false;
		}

		if (featureLevel != D3D_FEATURE_LEVEL_11_0)
		{
			MessageBox(nullptr, L"Direct3D Feature Level 11 unsupported.", 0, 0);
			return false;
		}

		DXGI_SWAP_CHAIN_DESC swapchainDesc;
		swapchainDesc.BufferDesc.Width = desc.backBufferWidth;
		swapchainDesc.BufferDesc.Height = desc.backBufferHeight;
		swapchainDesc.BufferDesc.RefreshRate.Numerator = 60;
		swapchainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapchainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapchainDesc.SampleDesc.Count = 1;
		swapchainDesc.SampleDesc.Quality = 0;
		swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapchainDesc.BufferCount = 1;
		swapchainDesc.OutputWindow = desc.hWnd;
		swapchainDesc.Windowed = true;
		swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapchainDesc.Flags = 0;

		IDXGIDevice* pDXGIDevice = nullptr;
		if (FAILED(m_pD3DDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDXGIDevice)))
		{
			return false;
		}
		IDXGIAdapter* pDXGIAdapter = nullptr;
		if (FAILED(pDXGIDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&pDXGIAdapter)))
		{
			return false;
		}
		IDXGIFactory* pDXGIFactory = nullptr;
		if (FAILED(pDXGIAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&pDXGIFactory)))
		{
			return false;
		}

		if (FAILED(pDXGIFactory->CreateSwapChain(m_pD3DDevice, &swapchainDesc, &m_pSwapChain)))
		{
			MessageBox(nullptr, L"Direct3D create swapchain failed.", 0, 0);
			return false;
		}
		pDXGIDevice->Release();
		pDXGIAdapter->Release();
		pDXGIFactory->Release();

		if (!OnResize(desc.backBufferWidth, desc.backBufferHeight))
		{
			MessageBox(nullptr, L"Direct3D OnResize failed.", 0, 0);
			return false;
		}

		return true;
	}

	void D3D11Renderer::Release()
	{
		if (m_pRenderTargetView)
		{
			m_pRenderTargetView->Release();
			m_pRenderTargetView = nullptr;
		}

		if (m_pDepthStencilView)
		{
			m_pDepthStencilView->Release();
			m_pDepthStencilView = nullptr;
		}

		if (m_pDepthStencilTexture)
		{
			m_pDepthStencilTexture->Release();
			m_pDepthStencilTexture = nullptr;
		}

		if (m_pSwapChain)
		{
			m_pSwapChain->Release();
			m_pSwapChain = nullptr;
		}

		if (m_pD3DImmediateContext)
		{
			m_pD3DImmediateContext->ClearState();
			m_pD3DImmediateContext->Release();
			m_pD3DImmediateContext = nullptr;
		}

#if defined(DEBUG) || defined(_DEBUG)
		ID3D11Debug* pD3dDebug = nullptr;
		HRESULT hr = m_pD3DDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&pD3dDebug));
		if (SUCCEEDED(hr))
		{
			hr = pD3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
		}
		pD3dDebug->Release();
#endif

		if (m_pD3DDevice)
		{
			m_pD3DDevice->Release();
			m_pD3DDevice = nullptr;
		}
	}

	void D3D11Renderer::Render()
	{
		float clearColor[4] = { 1.0f, 0.3f, 0.3f, 1.0f };
		m_pD3DImmediateContext->ClearRenderTargetView(m_pRenderTargetView, clearColor);
		m_pD3DImmediateContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

		m_pD3DImmediateContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

		m_pSwapChain->Present(0, 0);
	}

	bool D3D11Renderer::OnResize(uint32_t width, uint32_t height)
	{
		if (!m_pD3DDevice)
		{
			return false;
		}

		if (m_pRenderTargetView)
		{
			m_pRenderTargetView->Release();
		}

		if (m_pDepthStencilView)
		{
			m_pDepthStencilView->Release();
		}

		if (m_pDepthStencilTexture)
		{
			m_pDepthStencilTexture->Release();
		}

		if (FAILED(m_pSwapChain->ResizeBuffers(1, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0)))
		{
			return false;
		}
		ID3D11Texture2D* pBackBuffer = nullptr;
		if (FAILED(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer)))
		{
			return false;
		}
		if (FAILED(m_pD3DDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_pRenderTargetView)))
		{
			return false;
		}
		pBackBuffer->Release();

		D3D11_TEXTURE2D_DESC depthStencilDesc;
		depthStencilDesc.Width = width;
		depthStencilDesc.Height = height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;
		if (FAILED(m_pD3DDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilTexture)))
		{
			return false;
		}
		if (FAILED(m_pD3DDevice->CreateDepthStencilView(m_pDepthStencilTexture, nullptr, &m_pDepthStencilView)))
		{
			return false;
		}

		m_ScreenViewport.TopLeftX = 0;
		m_ScreenViewport.TopLeftY = 0;
		m_ScreenViewport.Width = static_cast<float>(width);
		m_ScreenViewport.Height = static_cast<float>(height);
		m_ScreenViewport.MinDepth = 0;
		m_ScreenViewport.MaxDepth = 1;
		m_pD3DImmediateContext->RSSetViewports(1, &m_ScreenViewport);

		return true;
	}
}