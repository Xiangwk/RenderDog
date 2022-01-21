////////////////////////////////////////
//RenderDog <·,·>
//FileName: D3D11Renderer.cpp
//Written by Xiang Weikang
////////////////////////////////////////

#include "Renderer.h"
#include "D3D11Renderer.h"
#include "Primitive.h"
#include "Scene.h"
#include "SceneView.h"
#include "D3D11InputLayout.h"
#include "Matrix.h"
#include "Camera.h"

namespace RenderDog
{
	ID3D11Device*			g_pD3D11Device = nullptr;
	ID3D11DeviceContext*	g_pD3D11ImmediateContext = nullptr;

	class D3D11MeshRenderer : public IPrimitiveRenderer
	{
	public:
		D3D11MeshRenderer();
		D3D11MeshRenderer(IConstantBuffer* pCB);
		virtual ~D3D11MeshRenderer();

		virtual IConstantBuffer*		GetVSConstantBuffer() override { return m_pVSConstantBuffer; }

		virtual void					Render(const PrimitiveRenderParam& renderParam) override;

	private:
		IConstantBuffer*				m_pVSConstantBuffer;
	};

	D3D11MeshRenderer::D3D11MeshRenderer() :
		m_pVSConstantBuffer(nullptr)
	{}

	D3D11MeshRenderer::~D3D11MeshRenderer()
	{}

	D3D11MeshRenderer::D3D11MeshRenderer(IConstantBuffer* pCB) :
		m_pVSConstantBuffer(pCB)
	{}

	void D3D11MeshRenderer::Render(const PrimitiveRenderParam& renderParam)
	{
		if (!g_pD3D11ImmediateContext)
		{
			return;
		}

		if (!renderParam.pVB || !renderParam.pIB)
		{
			return;
		}

		ID3D11Buffer* pVB = (ID3D11Buffer*)(renderParam.pVB->GetVertexBuffer());
		ID3D11Buffer* pIB = (ID3D11Buffer*)(renderParam.pIB->GetIndexBuffer());

		uint32_t indexNum = renderParam.pIB->GetIndexNum();

		uint32_t stride = renderParam.pVB->GetStride();
		uint32_t offset = renderParam.pVB->GetOffset();
		g_pD3D11ImmediateContext->IASetVertexBuffers(0, 1, &pVB, &stride, &offset);
		g_pD3D11ImmediateContext->IASetIndexBuffer(pIB, DXGI_FORMAT_R32_UINT, 0);

		renderParam.pVS->SetToContext();

		ID3D11Buffer* pGlobalCB = (ID3D11Buffer*)(renderParam.pGlobalCB->GetConstantBuffer());
		g_pD3D11ImmediateContext->VSSetConstantBuffers(0, 1, &pGlobalCB);

		ID3D11Buffer* pPerObjCB = (ID3D11Buffer*)(renderParam.pPerObjCB->GetConstantBuffer());
		g_pD3D11ImmediateContext->VSSetConstantBuffers(1, 1, &pPerObjCB);

		renderParam.pPS->SetToContext();

		g_pD3D11ImmediateContext->DrawIndexed(indexNum, 0, 0);
	}


	class D3D11Renderer : public IRenderer
	{
	private:
		struct GlobalConstantData
		{
			Matrix4x4	viewMatrix;
			Matrix4x4	projMatrix;
		};

	public:
		D3D11Renderer();
		virtual ~D3D11Renderer();

		virtual bool				Init(const RendererInitDesc& desc) override;
		virtual void				Release() override;

		virtual void				Update() override;
		virtual void				Render(IScene* pScene) override;

		virtual bool				OnResize(uint32_t width, uint32_t height);

	private:
		void						ClearBackRenderTarget(float* clearColor);

		void						AddPrimitivesToSceneView(IScene* pScene);

		void						RenderPrimitives();

	private:
		IDXGISwapChain*				m_pSwapChain;

		ID3D11Texture2D*			m_pDepthStencilTexture;
		ID3D11RenderTargetView*		m_pRenderTargetView;
		ID3D11DepthStencilView*		m_pDepthStencilView;
		D3D11_VIEWPORT				m_ScreenViewport;

		SceneView*					m_pSceneView;
		GlobalConstantData			m_GlobalConstantData;
		IConstantBuffer*			m_pGlobalConstantBuffer;
	};

	D3D11Renderer g_D3D11Renderer;
	IRenderer* g_pIRenderer = &g_D3D11Renderer;

	//------------------------------------------------------------------------
	//   Public Function
	//------------------------------------------------------------------------
	D3D11Renderer::D3D11Renderer() :
		m_pSwapChain(nullptr),
		m_pDepthStencilTexture(nullptr),
		m_pRenderTargetView(nullptr),
		m_pDepthStencilView(nullptr),
		m_ScreenViewport(),
		m_pSceneView(nullptr),
		m_GlobalConstantData(),
		m_pGlobalConstantBuffer(nullptr)
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
			&g_pD3D11Device,
			&featureLevel,
			&g_pD3D11ImmediateContext);
		if (FAILED(hr))
		{
			MessageBox(nullptr, "D3D11CreateDevice Failed!", "ERROR", MB_OK);
			return false;
		}

		if (featureLevel != D3D_FEATURE_LEVEL_11_0)
		{
			MessageBox(nullptr, "Direct3D Feature Level 11 unsupported.", 0, 0);
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
		if (FAILED(g_pD3D11Device->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDXGIDevice)))
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

		if (FAILED(pDXGIFactory->CreateSwapChain(g_pD3D11Device, &swapchainDesc, &m_pSwapChain)))
		{
			MessageBox(nullptr, "Direct3D create swapchain failed.", 0, 0);
			return false;
		}
		pDXGIDevice->Release();
		pDXGIAdapter->Release();
		pDXGIFactory->Release();

		if (!OnResize(desc.backBufferWidth, desc.backBufferHeight))
		{
			MessageBox(nullptr, "Direct3D OnResize failed.", 0, 0);
			return false;
		}

		m_pSceneView = new SceneView(desc.backBufferWidth, desc.backBufferHeight);
		m_pGlobalConstantBuffer = g_pIBufferManager->CreateConstantBuffer();
		BufferDesc cbDesc = {};
		cbDesc.byteWidth = sizeof(GlobalConstantData);
		cbDesc.pInitData = nullptr;
		cbDesc.isDynamic = true;
		if (!m_pGlobalConstantBuffer->Init(cbDesc))
		{
			return false;
		}

		return true;
	}

	void D3D11Renderer::Release()
	{
		if (m_pSceneView)
		{
			delete m_pSceneView;
			m_pSceneView = nullptr;
		}

		if (m_pGlobalConstantBuffer)
		{
			m_pGlobalConstantBuffer->Release();
			delete m_pGlobalConstantBuffer;
			m_pGlobalConstantBuffer = nullptr;
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

		if (m_pSwapChain)
		{
			m_pSwapChain->Release();
		}

		if (g_pD3D11ImmediateContext)
		{
			g_pD3D11ImmediateContext->ClearState();
			g_pD3D11ImmediateContext->Release();
		}

#if defined(DEBUG) || defined(_DEBUG)
		/*ID3D11Debug* pD3dDebug = nullptr;
		HRESULT hr = g_pD3D11Device->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&pD3dDebug));
		if (SUCCEEDED(hr))
		{
			hr = pD3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
		}
		pD3dDebug->Release();*/
#endif

		if (g_pD3D11Device)
		{
			g_pD3D11Device->Release();
		}
	}

	void D3D11Renderer::Update()
	{
		FPSCamera* pCamera = m_pSceneView->GetCamera();
		m_GlobalConstantData.viewMatrix = Transpose(pCamera->GetViewMatrix());
		m_GlobalConstantData.projMatrix = Transpose(pCamera->GetPerspProjectionMatrix());

		m_pGlobalConstantBuffer->Update(&m_GlobalConstantData, sizeof(m_GlobalConstantData));
	}


	void D3D11Renderer::Render(IScene* pScene)
	{
		m_pSceneView->ClearPrimitives();

		AddPrimitivesToSceneView(pScene);

		g_pD3D11ImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		float clearColor[4] = { 0.3f, 0.3f, 0.3f, 1.0f };
		ClearBackRenderTarget(clearColor);

		RenderPrimitives();

		m_pSwapChain->Present(0, 0);
	}

	bool D3D11Renderer::OnResize(uint32_t width, uint32_t height)
	{
		if (!g_pD3D11Device)
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
		if (FAILED(g_pD3D11Device->CreateRenderTargetView(pBackBuffer, nullptr, &m_pRenderTargetView)))
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
		if (FAILED(g_pD3D11Device->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilTexture)))
		{
			return false;
		}
		if (FAILED(g_pD3D11Device->CreateDepthStencilView(m_pDepthStencilTexture, nullptr, &m_pDepthStencilView)))
		{
			return false;
		}

		m_ScreenViewport.TopLeftX = 0;
		m_ScreenViewport.TopLeftY = 0;
		m_ScreenViewport.Width = static_cast<float>(width);
		m_ScreenViewport.Height = static_cast<float>(height);
		m_ScreenViewport.MinDepth = 0;
		m_ScreenViewport.MaxDepth = 1;
		g_pD3D11ImmediateContext->RSSetViewports(1, &m_ScreenViewport);

		return true;
	}

	//------------------------------------------------------------------------
	//   Private Function
	//------------------------------------------------------------------------
	void D3D11Renderer::ClearBackRenderTarget(float* clearColor)
	{
		g_pD3D11ImmediateContext->ClearRenderTargetView(m_pRenderTargetView, clearColor);
		g_pD3D11ImmediateContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

		g_pD3D11ImmediateContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);
	}

	void D3D11Renderer::AddPrimitivesToSceneView(IScene* pScene)
	{
		uint32_t priNum = pScene->GetPrimitivesNum();
		for (uint32_t i = 0; i < priNum; ++i)
		{
			IPrimitive* pPri = pScene->GetPrimitive(i);

			//FIXME!!! 这里后续应该添加视锥裁剪
			m_pSceneView->AddPrimitive(pPri);
		}
	}

	void D3D11Renderer::RenderPrimitives()
	{
		D3D11MeshRenderer meshRender(m_pGlobalConstantBuffer);

		uint32_t priNum = m_pSceneView->GetPrimitiveNum();
		for (uint32_t i = 0; i < priNum; ++i)
		{
			IPrimitive* pPri = m_pSceneView->GetPrimitive(i);
			pPri->Render(&meshRender);
		}
	}

}// namespace RenderDog