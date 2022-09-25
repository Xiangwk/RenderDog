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
#include "Material.h"

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
		explicit SoftwareMeshRenderer(SceneView* pSceneView);
		virtual ~SoftwareMeshRenderer();

	protected:
		SceneView*		m_pSceneView;

		IShader*		m_pVertexShader;
		IShader*		m_pPixelShader;
	};

	SoftwareMeshRenderer::SoftwareMeshRenderer(SceneView* pSceneView) :
		m_pSceneView(pSceneView),
		m_pVertexShader(nullptr),
		m_pPixelShader(nullptr)
	{}

	SoftwareMeshRenderer::~SoftwareMeshRenderer()
	{
		if (m_pVertexShader)
		{
			m_pVertexShader->Release();
			m_pVertexShader = nullptr;
		}

		if (m_pPixelShader)
		{
			m_pPixelShader->Release();
			m_pPixelShader = nullptr;
		}
	}
#pragma endregion MeshRenderer

#pragma region LineMeshRenderer
	class SoftwareLineMeshRenderer : public SoftwareMeshRenderer
	{
	public:
		explicit SoftwareLineMeshRenderer(SceneView* pSceneView);
		virtual ~SoftwareLineMeshRenderer();

		virtual void	Render(const PrimitiveRenderParam& renderParam) override;
	};

	SoftwareLineMeshRenderer::SoftwareLineMeshRenderer(SceneView* pSceneView) :
		SoftwareMeshRenderer(pSceneView)
	{}

	SoftwareLineMeshRenderer::~SoftwareLineMeshRenderer()
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

		renderParam.pVS->Apply();

		m_pPixelShader->Apply();

		g_pSRImmediateContext->DrawIndex(indexNum);
	}
#pragma endregion LineMeshRenderer
	
#pragma region MeshLightingRenderer
	struct MeshLightingGlobalData
	{
		SceneView*	pSceneView;

		MeshLightingGlobalData() :
			pSceneView(nullptr)
		{}
	};

	class SoftwareMeshLightingRenderer : public SoftwareMeshRenderer
	{
	public:
		explicit SoftwareMeshLightingRenderer(const MeshLightingGlobalData& globalData);
		virtual ~SoftwareMeshLightingRenderer();

		virtual void				Render(const PrimitiveRenderParam& renderParam) override;

	protected:
		void						ApplyMaterialParam(IMaterialInstance* pMtlIns);
	};

	SoftwareMeshLightingRenderer::SoftwareMeshLightingRenderer(const MeshLightingGlobalData& globalData) :
		SoftwareMeshRenderer(globalData.pSceneView)
	{
		ShaderCompileDesc psDesc(g_DirectionalLightingPixelShaderFilePath, nullptr, "Main", "ps_5_0", 0);
		m_pPixelShader = g_pIShaderManager->GetDirectionLightingPixelShader(psDesc);
	}

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

		renderParam.pVS->Apply(&renderParam.PerObjParam);

		ApplyMaterialParam(renderParam.pMtlIns);

		m_pPixelShader->Apply();

		g_pSRImmediateContext->DrawIndex(indexNum);
	}

	void SoftwareMeshLightingRenderer::ApplyMaterialParam(IMaterialInstance* pMtlIns)
	{
		if (!pMtlIns)
		{
			return;
		}

		uint32_t mtlParamNum = pMtlIns->GetMaterialParamNum();
		for (uint32_t i = 0; i < mtlParamNum; ++i)
		{
			MaterialParam& param = pMtlIns->GetMaterialParamByIndex(i);
			MATERIAL_PARAM_TYPE paramType = param.GetType();
			const std::string& paramName = param.GetName();
			switch (paramType)
			{
			case MATERIAL_PARAM_TYPE::UNKNOWN:
				break;
			case MATERIAL_PARAM_TYPE::SCALAR:
			{
				break;
			}
			case MATERIAL_PARAM_TYPE::VECTOR4:
			{
				break;
			}
			case MATERIAL_PARAM_TYPE::TEXTURE2D:
			{
				ShaderParam* pTextureParam = m_pPixelShader->GetShaderParamPtrByName(paramName);
				pTextureParam->SetTexture(param.GetTexture2D());

				break;
			}
			case MATERIAL_PARAM_TYPE::SAMPLER:
			{
				ShaderParam* pSamplerParam = m_pPixelShader->GetShaderParamPtrByName(paramName);
				pSamplerParam->SetSampler(param.GetSamplerState());

				break;
			}
			default:
				break;
			}
		}

		/*ShaderParam* pNormalTextureParam = m_pPixelShader->GetShaderParamPtrByName("NormalTexture");
		pNormalTextureParam->SetTexture(renderParam.pNormalTexture);

		ShaderParam* pNormalTextureSamplerParam = m_pPixelShader->GetShaderParamPtrByName("NormalTextureSampler");
		pNormalTextureSamplerParam->SetSampler(renderParam.pNormalTextureSampler);*/
	}

	//===========================================================
	//    Software Renderer
	//===========================================================


	class SoftwareRenderer : public IRenderer
	{
	private:
		struct ViewParamData
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
		bool						CreateInternalShaders();
		void						ReleaseInternalShaders();

		void						ClearBackRenderTarget(float* clearColor);

		void						AddPrisAndLightsToSceneView(IScene* pScene);

		void						RenderPrimitives();

	private:
		ISRSwapChain*				m_pSwapChain;

		ISRTexture2D*				m_pDepthStencilTexture;
		ISRRenderTargetView*		m_pRenderTargetView;
		ISRDepthStencilView*		m_pDepthStencilView;

		SceneView*					m_pSceneView;

		IConstantBuffer*			m_pViewParamConstantBuffer;
		IConstantBuffer*			m_pLightingConstantBuffer;

		IShader*					m_pModelVertexShader;
		IShader*					m_pLightingPixelShader;
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
		m_pSceneView(nullptr),
		m_pViewParamConstantBuffer(nullptr),
		m_pLightingConstantBuffer(nullptr),
		m_pModelVertexShader(nullptr),
		m_pLightingPixelShader(nullptr)
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
		cbDesc.name = "ComVar_ConstantBuffer_ViewParam";
		cbDesc.byteWidth = sizeof(ViewParamData);
		cbDesc.pInitData = nullptr;
		cbDesc.isDynamic = true;
		m_pViewParamConstantBuffer = (IConstantBuffer*)g_pIBufferManager->GetConstantBuffer(cbDesc);

		cbDesc.name = "ComVar_ConstantBuffer_LightingParam";
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

		if (m_pViewParamConstantBuffer)
		{
			m_pViewParamConstantBuffer->Release();
			m_pViewParamConstantBuffer = nullptr;
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
		ViewParamData ViewParamCBData = {};
		ViewParamCBData.viewMatrix = pCamera->GetViewMatrix();
		ViewParamCBData.projMatrix = pCamera->GetPerspProjectionMatrix();

		m_pViewParamConstantBuffer->Update(&ViewParamCBData, sizeof(ViewParamCBData));

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
	bool SoftwareRenderer::CreateInternalShaders()
	{
		//VertexShader
		ShaderCompileDesc vsDesc(g_SimpleModelVertexShadreFilePath, nullptr, "Main", "vs_5_0", 0);
		m_pModelVertexShader = g_pIShaderManager->GetModelVertexShader(VERTEX_TYPE::STANDARD, vsDesc);

		//PixeShader
		ShaderCompileDesc psDesc = ShaderCompileDesc(g_DirectionalLightingPixelShaderFilePath, nullptr, "Main", "ps_5_0", 0);
		m_pLightingPixelShader = g_pIShaderManager->GetDirectionLightingPixelShader(psDesc);

		return true;
	}

	void SoftwareRenderer::ReleaseInternalShaders()
	{
		if (m_pModelVertexShader)
		{
			m_pModelVertexShader->Release();
			m_pModelVertexShader = nullptr;
		}

		if (m_pLightingPixelShader)
		{
			m_pLightingPixelShader->Release();
			m_pLightingPixelShader = nullptr;
		}
	}

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
		SoftwareLineMeshRenderer lineMeshRenderer(m_pSceneView);

		uint32_t simplePriNum = m_pSceneView->GetSimplePrisNum();
		for (uint32_t i = 0; i < simplePriNum; ++i)
		{
			IPrimitive* pPri = m_pSceneView->GetSimplePri(i);
			pPri->Render(&lineMeshRenderer);
		}

		MeshLightingGlobalData meshLightingData;
		meshLightingData.pSceneView = m_pSceneView;
		SoftwareMeshLightingRenderer meshRender(meshLightingData);

		uint32_t opaquePriNum = m_pSceneView->GetOpaquePrisNum();
		for (uint32_t i = 0; i < opaquePriNum; ++i)
		{
			IPrimitive* pPri = m_pSceneView->GetOpaquePri(i);
			pPri->Render(&meshRender);
		}
	}
}