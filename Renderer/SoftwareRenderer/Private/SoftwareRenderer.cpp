///////////////////////////////////
//RenderDog <??,??>
//FileName: SoftwareRender3D.cpp
//Written by Xiang Weikang
///////////////////////////////////

#include "SoftwareRenderer.h"
#include "SoftwareRender3D.h"
#include "Renderer.h"
#include "SceneView.h"
#include "Buffer.h"
#include "Matrix.h"
#include "Camera.h"
#include "Light.h"
#include "Primitive.h"
#include "Texture.h"
#include "Scene.h"

namespace RenderDog
{
	ISRDevice*			g_pSRDevice = nullptr;
	ISRDeviceContext*	g_pSRImmediateContext = nullptr;

	//===========================================================
	//    Mesh Renderer
	//===========================================================
#pragma region MeshRenderer
	class SoftwareMeshRenderer : public IPrimitiveRenderer
	{
	public:
		SoftwareMeshRenderer();
		SoftwareMeshRenderer(IConstantBuffer* pGlobalCB);
		virtual ~SoftwareMeshRenderer();

	protected:
		IConstantBuffer*			m_pGlobalCB;
	};

	SoftwareMeshRenderer::SoftwareMeshRenderer() :
		m_pGlobalCB(nullptr)
	{}

	SoftwareMeshRenderer::~SoftwareMeshRenderer()
	{}

	SoftwareMeshRenderer::SoftwareMeshRenderer(IConstantBuffer* pGlobalCB) :
		m_pGlobalCB(pGlobalCB)
	{}
#pragma endregion MeshRenderer

#pragma region LineMeshRenderer
	class SoftwareLineMeshRenderer : public SoftwareMeshRenderer
	{
	public:
		SoftwareLineMeshRenderer();
		SoftwareLineMeshRenderer(IConstantBuffer* pGlobalCB);
		virtual ~SoftwareLineMeshRenderer();

		virtual void					Render(const PrimitiveRenderParam& renderParam) override;
	};

	SoftwareLineMeshRenderer::SoftwareLineMeshRenderer()
	{}

	SoftwareLineMeshRenderer::~SoftwareLineMeshRenderer()
	{}

	SoftwareLineMeshRenderer::SoftwareLineMeshRenderer(IConstantBuffer* pGlobalCB) :
		SoftwareMeshRenderer(pGlobalCB)
	{}

	void SoftwareLineMeshRenderer::Render(const PrimitiveRenderParam& renderParam)
	{
		if (!g_pSRImmediateContext)
		{
			return;
		}

		if (!renderParam.pVB || !renderParam.pIB)
		{
			return;
		}

		g_pSRImmediateContext->IASetPrimitiveTopology(RenderDog::SR_PRIMITIVE_TOPOLOGY::LINE_LIST);

		ISRBuffer* pVB = (ISRBuffer*)(renderParam.pVB->GetResource());
		ISRBuffer* pIB = (ISRBuffer*)(renderParam.pIB->GetResource());

		uint32_t indexNum = renderParam.pIB->GetIndexNum();

		uint32_t stride = renderParam.pVB->GetStride();
		uint32_t offset = renderParam.pVB->GetOffset();
		g_pSRImmediateContext->IASetVertexBuffer(pVB);
		g_pSRImmediateContext->IASetIndexBuffer(pIB);

		renderParam.pVS->SetToContext();

		ISRBuffer* pGlobalCB = (ISRBuffer*)(m_pGlobalCB->GetResource());
		g_pSRImmediateContext->VSSetConstantBuffer(0, &pGlobalCB);

		ISRBuffer* pPerObjCB = (ISRBuffer*)(renderParam.pPerObjCB->GetResource());
		g_pSRImmediateContext->VSSetConstantBuffer(1, &pPerObjCB);

		renderParam.pPS->SetToContext();

		g_pSRImmediateContext->DrawIndex(indexNum);
	}
#pragma endregion LineMeshRenderer
	
#pragma region MeshLightingRenderer
	struct MeshLightingGlobalData
	{
		IConstantBuffer* pGlobalCB;
		IConstantBuffer* pLightingCB;
	};

	class SoftwareMeshLightingRenderer : public SoftwareMeshRenderer
	{
	public:
		SoftwareMeshLightingRenderer();
		SoftwareMeshLightingRenderer(const MeshLightingGlobalData& globalData);
		virtual ~SoftwareMeshLightingRenderer();

		virtual void				Render(const PrimitiveRenderParam& renderParam) override;

	protected:
		IConstantBuffer*			m_LightingCB;
	};

	SoftwareMeshLightingRenderer::SoftwareMeshLightingRenderer() :
		SoftwareMeshRenderer(),
		m_LightingCB(nullptr)
	{}

	SoftwareMeshLightingRenderer::SoftwareMeshLightingRenderer(const MeshLightingGlobalData& globalData) :
		SoftwareMeshRenderer(globalData.pGlobalCB),
		m_LightingCB(globalData.pLightingCB)
	{}

	SoftwareMeshLightingRenderer::~SoftwareMeshLightingRenderer()
	{}

	void SoftwareMeshLightingRenderer::Render(const PrimitiveRenderParam& renderParam)
	{
		if (!g_pSRImmediateContext)
		{
			return;
		}

		if (!renderParam.pVB || !renderParam.pIB)
		{
			return;
		}

		g_pSRImmediateContext->IASetPrimitiveTopology(RenderDog::SR_PRIMITIVE_TOPOLOGY::TRIANGLE_LIST);

		ISRBuffer* pVB = (ISRBuffer*)(renderParam.pVB->GetResource());
		ISRBuffer* pIB = (ISRBuffer*)(renderParam.pIB->GetResource());

		uint32_t indexNum = renderParam.pIB->GetIndexNum();

		uint32_t stride = renderParam.pVB->GetStride();
		uint32_t offset = renderParam.pVB->GetOffset();
		g_pSRImmediateContext->IASetVertexBuffer(pVB);
		g_pSRImmediateContext->IASetIndexBuffer(pIB);

		renderParam.pVS->SetToContext();

		ISRBuffer* pGlobalCB = (ISRBuffer*)(m_pGlobalCB->GetResource());
		g_pSRImmediateContext->VSSetConstantBuffer(0, &pGlobalCB);

		ISRBuffer* pPerObjCB = (ISRBuffer*)(renderParam.pPerObjCB->GetResource());
		g_pSRImmediateContext->VSSetConstantBuffer(1, &pPerObjCB);

		renderParam.pPS->SetToContext();

		ISRBuffer* pLightingCB = (ISRBuffer*)(m_LightingCB->GetResource());
		g_pSRImmediateContext->PSSetConstantBuffer(0, &pLightingCB);

		ISRShaderResourceView* pSRV = (ISRShaderResourceView*)(renderParam.pNormalTexture->GetShaderResourceView());
		g_pSRImmediateContext->PSSetShaderResource(&pSRV);
		renderParam.pNormalTextureSampler->SetToPixelShader(0);

		g_pSRImmediateContext->DrawIndex(indexNum);
	}

	//===========================================================
	//    Software Renderer
	//===========================================================


	class SoftwareRenderer : public IRenderer
	{
	private:
		struct GlobalConstantData
		{
			Matrix4x4	viewMatrix;
			Matrix4x4	projMatrix;
		};

		struct DirectionalLightData
		{
			Vector4		color;
			Vector3		direction;
			float		luminance;
		};

	public:
		SoftwareRenderer();
		~SoftwareRenderer();

		virtual bool				Init(const RendererInitDesc& desc) override;
		virtual void				Release() override;

		virtual void				Update(IScene* pScene) override;
		virtual void				Render(IScene* pScene) override;

		virtual bool				OnResize(uint32_t width, uint32_t height);

	private:
		void						ClearBackRenderTarget(float* clearColor);

		void						AddPrisAndLightsToSceneView(IScene* pScene);

		void						RenderPrimitives();

	private:
		ISRSwapChain*				m_pSwapChain;

		ISRTexture2D*				m_pDepthStencilTexture;
		ISRRenderTargetView*		m_pRenderTargetView;
		ISRDepthStencilView*		m_pDepthStencilView;
		SRViewport					m_ScreenViewport;

		SceneView*					m_pSceneView;

		IConstantBuffer*			m_pGlobalConstantBuffer;
		IConstantBuffer*			m_pLightingConstantBuffer;
	};

	SoftwareRenderer	g_SoftwareRenderer;
	IRenderer*			g_pIRenderer = &g_SoftwareRenderer;

	//------------------------------------------------------------------------
	//   Public Function
	//------------------------------------------------------------------------
	SoftwareRenderer::SoftwareRenderer() :
		m_pSwapChain(nullptr),
		m_pDepthStencilTexture(nullptr),
		m_pRenderTargetView(nullptr),
		m_pDepthStencilView(nullptr),
		m_ScreenViewport(),
		m_pSceneView(nullptr),
		m_pGlobalConstantBuffer(nullptr),
		m_pLightingConstantBuffer(nullptr)
	{}

	SoftwareRenderer::~SoftwareRenderer()
	{}

	bool SoftwareRenderer::Init(const RendererInitDesc& desc)
	{
		RenderDog::SwapChainDesc swapChainDesc;
		ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
		swapChainDesc.width = desc.backBufferWidth;
		swapChainDesc.height = desc.backBufferHeight;
		swapChainDesc.format = RenderDog::SR_FORMAT::R8G8B8A8_UNORM;
		swapChainDesc.hOutputWindow = desc.hWnd;

		if (!RenderDog::CreateDeviceAndSwapChain(&g_pSRDevice, &g_pSRImmediateContext, &m_pSwapChain, &swapChainDesc))
		{
			MessageBox(nullptr, "SorfwareRender CreateDevice Failed!", "ERROR", MB_OK);
			return false;
		}

		if (!OnResize(desc.backBufferWidth, desc.backBufferHeight))
		{
			MessageBox(nullptr, "SoftwareRenderer OnResize failed.", 0, 0);
			return false;
		}

		m_pSceneView = new SceneView(desc.pMainCamera);

		BufferDesc cbDesc = {};
		cbDesc.name = "GlobalConstantBuffer";
		cbDesc.byteWidth = sizeof(GlobalConstantData);
		cbDesc.pInitData = nullptr;
		cbDesc.isDynamic = true;
		m_pGlobalConstantBuffer = (IConstantBuffer*)g_pIBufferManager->GetConstantBuffer(cbDesc);

		cbDesc.name = "MainLightConstantBuffer";
		cbDesc.byteWidth = sizeof(DirectionalLightData);
		m_pLightingConstantBuffer = (IConstantBuffer*)g_pIBufferManager->GetConstantBuffer(cbDesc);

		return true;
	}

	void SoftwareRenderer::Release()
	{
		if (m_pSceneView)
		{
			delete m_pSceneView;
			m_pSceneView = nullptr;
		}

		if (m_pGlobalConstantBuffer)
		{
			m_pGlobalConstantBuffer->Release();
			m_pGlobalConstantBuffer = nullptr;
		}

		if (m_pLightingConstantBuffer)
		{
			m_pLightingConstantBuffer->Release();
			m_pLightingConstantBuffer = nullptr;
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

		if (g_pSRImmediateContext)
		{
			g_pSRImmediateContext->Release();
		}

		if (g_pSRDevice)
		{
			g_pSRDevice->Release();
		}
	}

	void SoftwareRenderer::Update(IScene* pScene)
	{
		FPSCamera* pCamera = m_pSceneView->GetCamera();
		GlobalConstantData globalCBData = {};
		globalCBData.viewMatrix = pCamera->GetViewMatrix();
		globalCBData.projMatrix = pCamera->GetPerspProjectionMatrix();

		m_pGlobalConstantBuffer->Update(&globalCBData, sizeof(globalCBData));

		if (m_pSceneView->GetLightNum() > 0)
		{
			ILight* pMainLight = m_pSceneView->GetLight(0);
			DirectionalLightData dirLightData = {};
			dirLightData.color = Vector4(pMainLight->GetColor(), 1.0f);
			dirLightData.direction = pMainLight->GetDirection();
			dirLightData.luminance = pMainLight->GetLuminance();

			m_pLightingConstantBuffer->Update(&dirLightData, sizeof(dirLightData));
		}

		m_pSceneView->ClearPrimitives();
		AddPrisAndLightsToSceneView(pScene);
	}

	void SoftwareRenderer::Render(IScene* pScene)
	{
		float clearColor[4] = { 0.85f, 0.92f, 0.99f, 1.0f };
		ClearBackRenderTarget(clearColor);

		RenderPrimitives();

		m_pSwapChain->Present();
	}

	bool SoftwareRenderer::OnResize(uint32_t width, uint32_t height)
	{
		if (!g_pSRDevice)
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

		if (!m_pSwapChain->ResizeBuffers(1, width, height, RenderDog::SR_FORMAT::R8G8B8A8_UNORM))
		{
			return false;
		}

		RenderDog::ISRTexture2D* pBackBuffer = nullptr;
		if (!m_pSwapChain->GetBuffer((void**)&pBackBuffer))
		{
			return false;
		}

		if (!g_pSRDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_pRenderTargetView))
		{
			return false;
		}
		pBackBuffer->Release();

		RenderDog::SRTexture2DDesc depthDesc;
		depthDesc.width = width;
		depthDesc.height = height;
		depthDesc.format = RenderDog::SR_FORMAT::R32_FLOAT;
		if (!g_pSRDevice->CreateTexture2D(&depthDesc, nullptr, &m_pDepthStencilTexture))
		{
			return false;
		}

		RenderDog::SRDepthStencilViewDesc dsDesc;
		dsDesc.format = depthDesc.format;
		dsDesc.viewDimension = RenderDog::SR_DSV_DIMENSION::TEXTURE2D;
		if (!g_pSRDevice->CreateDepthStencilView(m_pDepthStencilTexture, &dsDesc, &m_pDepthStencilView))
		{
			return false;
		}

		RenderDog::SRViewport vp;
		vp.width = (float)width;
		vp.height = (float)height;
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;
		vp.topLeftX = 0;
		vp.topLeftY = 0;
		g_pSRImmediateContext->RSSetViewport(&vp);

		if (m_pSceneView && m_pSceneView->GetCamera())
		{
			m_pSceneView->GetCamera()->OnWindowResize(width, height);
		}

		return true;
	}

	//------------------------------------------------------------------------
	//   Private Function
	//------------------------------------------------------------------------
	void SoftwareRenderer::ClearBackRenderTarget(float* clearColor)
	{
		g_pSRImmediateContext->ClearRenderTargetView(m_pRenderTargetView, clearColor);
		g_pSRImmediateContext->ClearDepthStencilView(m_pDepthStencilView, 1.0f);

		g_pSRImmediateContext->OMSetRenderTarget(m_pRenderTargetView, m_pDepthStencilView);
	}

	void SoftwareRenderer::AddPrisAndLightsToSceneView(IScene* pScene)
	{
		uint32_t priNum = pScene->GetPrimitivesNum();
		for (uint32_t i = 0; i < priNum; ++i)
		{
			IPrimitive* pPri = pScene->GetPrimitive(i);

			//FIXME!!! ????????Ӧ????????׶?ü?
			m_pSceneView->AddPrimitive(pPri);
		}

		uint32_t lightsNum = pScene->GetLightsNum();
		for (uint32_t i = 0; i < lightsNum; ++i)
		{
			ILight* pLight = pScene->GetLight(i);

			if (pLight->GetType() == LIGHT_TYPE::DIRECTIONAL)
			{
				m_pSceneView->AddLight(pLight);
			}
		}
	}

	void SoftwareRenderer::RenderPrimitives()
	{
		SoftwareLineMeshRenderer lineMeshRenderer(m_pGlobalConstantBuffer);

		uint32_t simplePriNum = m_pSceneView->GetSimplePrisNum();
		for (uint32_t i = 0; i < simplePriNum; ++i)
		{
			IPrimitive* pPri = m_pSceneView->GetSimplePri(i);
			pPri->Render(&lineMeshRenderer);
		}

		MeshLightingGlobalData meshLightingData;
		meshLightingData.pGlobalCB = m_pGlobalConstantBuffer;
		meshLightingData.pLightingCB = m_pLightingConstantBuffer;
		SoftwareMeshLightingRenderer meshRender(meshLightingData);

		uint32_t opaquePriNum = m_pSceneView->GetOpaquePrisNum();
		for (uint32_t i = 0; i < opaquePriNum; ++i)
		{
			IPrimitive* pPri = m_pSceneView->GetOpaquePri(i);
			pPri->Render(&meshRender);
		}
	}
}