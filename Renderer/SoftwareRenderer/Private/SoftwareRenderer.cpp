///////////////////////////////////
//RenderDog <·,·>
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

	class SoftwareMeshRenderer : public IPrimitiveRenderer
	{
	public:
		SoftwareMeshRenderer();
		SoftwareMeshRenderer(IConstantBuffer* pVertexShaderCB);
		virtual ~SoftwareMeshRenderer();

		virtual IConstantBuffer*	GetVSConstantBuffer() override { return m_pVertexShaderCB; }
		virtual IConstantBuffer*	GetLightingConstantbuffer() override { return nullptr; }

		virtual void				Render(const PrimitiveRenderParam& renderParam, ITexture2D* pDiffuseTexture, ISamplerState* pSampler) override;

	protected:
		IConstantBuffer*			m_pVertexShaderCB;
	};

	SoftwareMeshRenderer::SoftwareMeshRenderer() :
		m_pVertexShaderCB(nullptr)
	{}

	SoftwareMeshRenderer::~SoftwareMeshRenderer()
	{}

	SoftwareMeshRenderer::SoftwareMeshRenderer(IConstantBuffer* pVertexShaderCB) :
		m_pVertexShaderCB(pVertexShaderCB)
	{}

	void SoftwareMeshRenderer::Render(const PrimitiveRenderParam& renderParam, ITexture2D* pDiffuseTexture, ISamplerState* pSampler)
	{
		if (!g_pSRImmediateContext)
		{
			return;
		}

		if (!renderParam.pVB || !renderParam.pIB)
		{
			return;
		}

		ISRBuffer* pVB = (ISRBuffer*)(renderParam.pVB->GetVertexBuffer());
		ISRBuffer* pIB = (ISRBuffer*)(renderParam.pIB->GetIndexBuffer());

		uint32_t indexNum = renderParam.pIB->GetIndexNum();

		uint32_t stride = renderParam.pVB->GetStride();
		uint32_t offset = renderParam.pVB->GetOffset();
		g_pSRImmediateContext->IASetVertexBuffer(pVB);
		g_pSRImmediateContext->IASetIndexBuffer(pIB);

		renderParam.pVS->SetToContext();

		ISRBuffer* pGlobalCB = (ISRBuffer*)(renderParam.pGlobalCB->GetConstantBuffer());
		g_pSRImmediateContext->VSSetConstantBuffer(0, &pGlobalCB);

		ISRBuffer* pPerObjCB = (ISRBuffer*)(renderParam.pPerObjCB->GetConstantBuffer());
		g_pSRImmediateContext->VSSetConstantBuffer(1, &pPerObjCB);

		renderParam.pPS->SetToContext();

		ISRShaderResourceView* pSRV = (ISRShaderResourceView*)(pDiffuseTexture->GetShaderResourceView());
		g_pSRImmediateContext->PSSetShaderResource(&pSRV);
		pSampler->SetToPixelShader(0);

		g_pSRImmediateContext->DrawIndex(indexNum);
	}

	class SoftwareMeshLightingRenderer : public SoftwareMeshRenderer
	{
	public:
		SoftwareMeshLightingRenderer();
		SoftwareMeshLightingRenderer(IConstantBuffer* pVertexShaderCB, IConstantBuffer* pLightingCB);
		virtual ~SoftwareMeshLightingRenderer();

		virtual IConstantBuffer*	GetLightingConstantbuffer() override { return m_LightingCB; }

		virtual void				Render(const PrimitiveRenderParam& renderParam, ITexture2D* pDiffuseTexture, ISamplerState* pSampler) override;

	protected:
		IConstantBuffer*			m_LightingCB;
	};

	SoftwareMeshLightingRenderer::SoftwareMeshLightingRenderer() :
		SoftwareMeshRenderer(),
		m_LightingCB(nullptr)
	{}

	SoftwareMeshLightingRenderer::SoftwareMeshLightingRenderer(IConstantBuffer* pVertexShaderCB, IConstantBuffer* pLightingCB) :
		SoftwareMeshRenderer(pVertexShaderCB),
		m_LightingCB(pLightingCB)
	{}

	SoftwareMeshLightingRenderer::~SoftwareMeshLightingRenderer()
	{}

	void SoftwareMeshLightingRenderer::Render(const PrimitiveRenderParam& renderParam, ITexture2D* pDiffuseTexture, ISamplerState* pSampler)
	{
		if (!g_pSRImmediateContext)
		{
			return;
		}

		if (!renderParam.pVB || !renderParam.pIB)
		{
			return;
		}

		ISRBuffer* pVB = (ISRBuffer*)(renderParam.pVB->GetVertexBuffer());
		ISRBuffer* pIB = (ISRBuffer*)(renderParam.pIB->GetIndexBuffer());

		uint32_t indexNum = renderParam.pIB->GetIndexNum();

		uint32_t stride = renderParam.pVB->GetStride();
		uint32_t offset = renderParam.pVB->GetOffset();
		g_pSRImmediateContext->IASetVertexBuffer(pVB);
		g_pSRImmediateContext->IASetIndexBuffer(pIB);

		renderParam.pVS->SetToContext();

		ISRBuffer* pGlobalCB = (ISRBuffer*)(renderParam.pGlobalCB->GetConstantBuffer());
		g_pSRImmediateContext->VSSetConstantBuffer(0, &pGlobalCB);

		ISRBuffer* pPerObjCB = (ISRBuffer*)(renderParam.pPerObjCB->GetConstantBuffer());
		g_pSRImmediateContext->VSSetConstantBuffer(1, &pPerObjCB);

		renderParam.pPS->SetToContext();

		ISRBuffer* pLightingCB = (ISRBuffer*)(m_LightingCB->GetConstantBuffer());
		g_pSRImmediateContext->PSSetConstantBuffer(0, &pLightingCB);

		ISRShaderResourceView* pSRV = (ISRShaderResourceView*)(pDiffuseTexture->GetShaderResourceView());
		g_pSRImmediateContext->PSSetShaderResource(&pSRV);
		pSampler->SetToPixelShader(0);

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

		virtual void				Update() override;
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
		cbDesc.bufferBind = BUFFER_BIND::CONSTANT;
		cbDesc.byteWidth = sizeof(GlobalConstantData);
		cbDesc.pInitData = nullptr;
		cbDesc.isDynamic = true;
		m_pGlobalConstantBuffer = (IConstantBuffer*)g_pIBufferManager->CreateBuffer(cbDesc);

		cbDesc.byteWidth = sizeof(DirectionalLightData);
		m_pLightingConstantBuffer = (IConstantBuffer*)g_pIBufferManager->CreateBuffer(cbDesc);

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

	void SoftwareRenderer::Update()
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
	}

	void SoftwareRenderer::Render(IScene* pScene)
	{
		m_pSceneView->ClearPrimitives();

		AddPrisAndLightsToSceneView(pScene);

		g_pSRImmediateContext->IASetPrimitiveTopology(RenderDog::SR_PRIMITIVE_TOPOLOGY::TRIANGLE_LIST);

		float clearColor[4] = { 0.3f, 0.3f, 0.3f, 1.0f };
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

			//FIXME!!! 这里后续应该添加视锥裁剪
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
		//D3D11MeshRenderer meshRender(m_pGlobalConstantBuffer);
		SoftwareMeshLightingRenderer meshRender(m_pGlobalConstantBuffer, m_pLightingConstantBuffer);

		uint32_t priNum = m_pSceneView->GetPrimitiveNum();
		for (uint32_t i = 0; i < priNum; ++i)
		{
			IPrimitive* pPri = m_pSceneView->GetPrimitive(i);
			pPri->Render(&meshRender);
		}
	}
}