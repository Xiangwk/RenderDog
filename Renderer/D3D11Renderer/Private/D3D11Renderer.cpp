////////////////////////////////////////
//RenderDog <·,·>
//FileName: D3D11Renderer.cpp
//Written by Xiang Weikang
////////////////////////////////////////

#include "Renderer.h"
#include "D3D11Renderer.h"
#include "Primitive.h"
#include "Light.h"
#include "Sky.h"
#include "Scene.h"
#include "SceneView.h"
#include "D3D11InputLayout.h"
#include "Matrix.h"
#include "Camera.h"
#include "Texture.h"
#include "Bounding.h"
#include "Transform.h"
#include "GlobalValue.h"

namespace RenderDog
{
	ID3D11Device*			g_pD3D11Device = nullptr;
	ID3D11DeviceContext*	g_pD3D11ImmediateContext = nullptr;

	const std::string g_StaticModelVertexShaderFilePath = "Shaders/StaticModelVertexShader.hlsl";
	const std::string g_DirectionalLightingPixelShaderFilePath = "Shaders/PhongLightingPixelShader.hlsl";
	const std::string g_SkyVertexShaderFilePath = "Shaders/SkyVertexShader.hlsl";
	const std::string g_SkyPixelShaderFilePath = "Shaders/SkyPixelShader.hlsl";

	//===========================================================
	//    Mesh Renderer
	//===========================================================
#pragma region MeshRenderer
	class D3D11MeshRenderer : public IPrimitiveRenderer
	{
	public:
		D3D11MeshRenderer();
		D3D11MeshRenderer(SceneView* pSceneView);
		virtual ~D3D11MeshRenderer();

	protected:
		IShader*				m_pVertexShader;
		IShader*				m_pPixelShader;
		SceneView*				m_pSceneView;
	};

	D3D11MeshRenderer::D3D11MeshRenderer() :
		m_pVertexShader(nullptr),
		m_pPixelShader(nullptr),
		m_pSceneView(nullptr)
	{}

	D3D11MeshRenderer::~D3D11MeshRenderer()
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

	D3D11MeshRenderer::D3D11MeshRenderer(SceneView* pSceneView) :
		m_pVertexShader(nullptr),
		m_pPixelShader(nullptr),
		m_pSceneView(pSceneView)
	{}
#pragma endregion MeshRenderer

#pragma region LineMeshRenderer
	//LineMeshRenderer: Use to draw line
	class D3D11LineMeshRenderer : public D3D11MeshRenderer
	{
	public:
		D3D11LineMeshRenderer();
		D3D11LineMeshRenderer(SceneView* pSceneView);
		virtual ~D3D11LineMeshRenderer();

		virtual void					Render(const PrimitiveRenderParam& renderParam) override;
	};

	D3D11LineMeshRenderer::D3D11LineMeshRenderer()
	{}

	D3D11LineMeshRenderer::~D3D11LineMeshRenderer()
	{}

	D3D11LineMeshRenderer::D3D11LineMeshRenderer(SceneView* pSceneView) :
		D3D11MeshRenderer(pSceneView)
	{}

	void D3D11LineMeshRenderer::Render(const PrimitiveRenderParam& renderParam)
	{
		if (!g_pD3D11ImmediateContext)
		{
			return;
		}

		if (!renderParam.pVB || !renderParam.pIB)
		{
			return;
		}

		g_pD3D11ImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

		ID3D11Buffer* pVB = (ID3D11Buffer*)(renderParam.pVB->GetResource());
		ID3D11Buffer* pIB = (ID3D11Buffer*)(renderParam.pIB->GetResource());

		uint32_t indexNum = renderParam.pIB->GetIndexNum();

		uint32_t stride = renderParam.pVB->GetStride();
		uint32_t offset = renderParam.pVB->GetOffset();
		g_pD3D11ImmediateContext->IASetVertexBuffers(0, 1, &pVB, &stride, &offset);
		g_pD3D11ImmediateContext->IASetIndexBuffer(pIB, DXGI_FORMAT_R32_UINT, 0);
		
		ShaderParam* pWorldToViewMatrix = renderParam.pVS->GetShaderParamPtrByName("ComVar_Matrix_WorldToView");
		ShaderParam* pViewToClipMatrix = renderParam.pVS->GetShaderParamPtrByName("ComVar_Matrix_ViewToClip");
		ShaderParam* pWorldEyePosition = renderParam.pVS->GetShaderParamPtrByName("ComVar_Vector_WorldEyePosition");

		FPSCamera* pCamera = m_pSceneView->GetCamera();
		pWorldToViewMatrix->SetMatrix4x4(pCamera->GetViewMatrix());
		pViewToClipMatrix->SetMatrix4x4(pCamera->GetPerspProjectionMatrix());
		pWorldEyePosition->SetVector4(Vector4(pCamera->GetPosition(), 1.0f));

		renderParam.pVS->Apply();

		ID3D11Buffer* pPerObjCB = (ID3D11Buffer*)(renderParam.pPerObjCB->GetResource());
		g_pD3D11ImmediateContext->VSSetConstantBuffers(1, 1, &pPerObjCB);

		renderParam.pPS->Apply();

		g_pD3D11ImmediateContext->DrawIndexed(indexNum, 0, 0);
	}
#pragma endregion LineMeshRenderer

#pragma region SkyMeshRenderer
	class D3D11SkyRenderer : public D3D11MeshRenderer
	{
	public:
		D3D11SkyRenderer(SceneView* pSceneView);
		virtual ~D3D11SkyRenderer();

		virtual void					Render(const PrimitiveRenderParam& renderParam) override;
	};

	D3D11SkyRenderer::D3D11SkyRenderer(SceneView* pSceneView) :
		D3D11MeshRenderer(pSceneView)
	{
		ShaderCompileDesc vsDesc(g_SkyVertexShaderFilePath, nullptr, "Main", "vs_5_0", 0);
		m_pVertexShader = g_pIShaderManager->GetStaticModelVertexShader(VERTEX_TYPE::STANDARD, vsDesc);

		ShaderCompileDesc psDesc(g_SkyPixelShaderFilePath, nullptr, "Main", "ps_5_0", 0);
		m_pPixelShader = g_pIShaderManager->GetSkyPixelShader(psDesc);
	}

	D3D11SkyRenderer::~D3D11SkyRenderer()
	{}

	void D3D11SkyRenderer::Render(const PrimitiveRenderParam& renderParam)
	{
		if (!g_pD3D11ImmediateContext)
		{
			return;
		}

		if (!renderParam.pVB || !renderParam.pIB)
		{
			return;
		}

		g_pD3D11ImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		ID3D11Buffer* pVB = (ID3D11Buffer*)(renderParam.pVB->GetResource());
		ID3D11Buffer* pIB = (ID3D11Buffer*)(renderParam.pIB->GetResource());

		uint32_t indexNum = renderParam.pIB->GetIndexNum();

		uint32_t stride = renderParam.pVB->GetStride();
		uint32_t offset = renderParam.pVB->GetOffset();
		g_pD3D11ImmediateContext->IASetVertexBuffers(0, 1, &pVB, &stride, &offset);
		g_pD3D11ImmediateContext->IASetIndexBuffer(pIB, DXGI_FORMAT_R32_UINT, 0);

		ShaderParam* pWorldToViewMatrix = m_pVertexShader->GetShaderParamPtrByName("ComVar_Matrix_WorldToView");
		ShaderParam* pViewToClipMatrix = m_pVertexShader->GetShaderParamPtrByName("ComVar_Matrix_ViewToClip");
		ShaderParam* pWorldEyePosition = m_pVertexShader->GetShaderParamPtrByName("ComVar_Vector_WorldEyePosition");

		FPSCamera* pCamera = m_pSceneView->GetCamera();
		pWorldToViewMatrix->SetMatrix4x4(pCamera->GetViewMatrix());
		pViewToClipMatrix->SetMatrix4x4(pCamera->GetPerspProjectionMatrix());
		pWorldEyePosition->SetVector4(Vector4(pCamera->GetPosition(), 1.0f));
		m_pVertexShader->Apply();

		ID3D11Buffer* pPerObjCB = (ID3D11Buffer*)(renderParam.pPerObjCB->GetResource());
		g_pD3D11ImmediateContext->VSSetConstantBuffers(1, 1, &pPerObjCB);

		ShaderParam* pSkyCubeTextureParam = m_pPixelShader->GetShaderParamPtrByName("ComVar_Texture_SkyCubeTexture");
		pSkyCubeTextureParam->SetTexture(renderParam.pDiffuseTexture);

		ShaderParam* pSkyCubeTextureSamplerParam = m_pPixelShader->GetShaderParamPtrByName("ComVar_Texture_SkyCubeTextureSampler");
		pSkyCubeTextureSamplerParam->SetSampler(renderParam.pDiffuseTextureSampler);
		m_pPixelShader->Apply();

		g_pD3D11ImmediateContext->DrawIndexed(indexNum, 0, 0);
	}

#pragma endregion SkyMeshRenderer

#pragma region MeshLightingRenderer
	struct MeshLightingGlobalData
	{
		SceneView*					pSceneView;
		IConstantBuffer*			pLightingCB;
		IConstantBuffer*			pShadowDepthCB;
		IConstantBuffer*			pShadowTestCB;
		ITexture2D*					pShadowDepthTexture;
		ISamplerState*				pShadowDepthTextureSampler;
		ITexture2D*					pEnvReflectionTexture;
		ISamplerState*				pEnvReflectionTextureSampler;

		MeshLightingGlobalData() :
			pSceneView(nullptr),
			pLightingCB(nullptr),
			pShadowDepthCB(nullptr),
			pShadowTestCB(nullptr),
			pShadowDepthTexture(nullptr),
			pShadowDepthTextureSampler(nullptr),
			pEnvReflectionTexture(nullptr),
			pEnvReflectionTextureSampler(nullptr)
		{}
	};

	class D3D11MeshLightingRenderer : public D3D11MeshRenderer
	{
	public:
		explicit D3D11MeshLightingRenderer(const MeshLightingGlobalData& globalData);
		virtual ~D3D11MeshLightingRenderer();

		virtual void					Render(const PrimitiveRenderParam& renderParam) override;

	protected:
		IConstantBuffer*				m_pLightingCB;
		IConstantBuffer*				m_pShadowDepthCB;
		IConstantBuffer*				m_pShadowTestCB;
		ITexture2D*						m_pShadowDepthTexture;
		ISamplerState*					m_pShadowDepthTextureSampler;
		ITexture2D*						m_pEnvReflectionTexture;
		ISamplerState*					m_pEnvReflectionTextureSampler;
	};

	D3D11MeshLightingRenderer::D3D11MeshLightingRenderer(const MeshLightingGlobalData& globalData):
		D3D11MeshRenderer(globalData.pSceneView),
		m_pLightingCB(globalData.pLightingCB),
		m_pShadowDepthCB(globalData.pShadowDepthCB),
		m_pShadowTestCB(globalData.pShadowTestCB),
		m_pShadowDepthTexture(globalData.pShadowDepthTexture),
		m_pShadowDepthTextureSampler(globalData.pShadowDepthTextureSampler),
		m_pEnvReflectionTexture(globalData.pEnvReflectionTexture),
		m_pEnvReflectionTextureSampler(globalData.pEnvReflectionTextureSampler)
	{
		ShaderCompileDesc vsDesc(g_StaticModelVertexShaderFilePath, nullptr, "Main", "vs_5_0", 0);
		m_pVertexShader = g_pIShaderManager->GetStaticModelVertexShader(VERTEX_TYPE::STANDARD, vsDesc);

		ShaderCompileDesc psDesc(g_DirectionalLightingPixelShaderFilePath, nullptr, "Main", "ps_5_0", 0);
		m_pPixelShader = g_pIShaderManager->GetDirectionLightingPixelShader(psDesc);
	}

	D3D11MeshLightingRenderer::~D3D11MeshLightingRenderer()
	{}

	void D3D11MeshLightingRenderer::Render(const PrimitiveRenderParam& renderParam)
	{
		if (!g_pD3D11ImmediateContext)
		{
			return;
		}
		
		if (!renderParam.pVB || !renderParam.pIB)
		{
			return;
		}

		g_pD3D11ImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		ID3D11Buffer* pVB = (ID3D11Buffer*)(renderParam.pVB->GetResource());
		ID3D11Buffer* pIB = (ID3D11Buffer*)(renderParam.pIB->GetResource());
		
		uint32_t indexNum = renderParam.pIB->GetIndexNum();

		uint32_t stride = renderParam.pVB->GetStride();
		uint32_t offset = renderParam.pVB->GetOffset();
		g_pD3D11ImmediateContext->IASetVertexBuffers(0, 1, &pVB, &stride, &offset);
		g_pD3D11ImmediateContext->IASetIndexBuffer(pIB, DXGI_FORMAT_R32_UINT, 0);

		ShaderParam* pWorldToViewMatrix = m_pVertexShader->GetShaderParamPtrByName("ComVar_Matrix_WorldToView");
		ShaderParam* pViewToClipMatrix = m_pVertexShader->GetShaderParamPtrByName("ComVar_Matrix_ViewToClip");
		ShaderParam* pWorldEyePosition = m_pVertexShader->GetShaderParamPtrByName("ComVar_Vector_WorldEyePosition");

		FPSCamera* pCamera = m_pSceneView->GetCamera();
		pWorldToViewMatrix->SetMatrix4x4(pCamera->GetViewMatrix());
		pViewToClipMatrix->SetMatrix4x4(pCamera->GetPerspProjectionMatrix());
		pWorldEyePosition->SetVector4(Vector4(pCamera->GetPosition(), 1.0f));

		m_pVertexShader->Apply();
		
		ID3D11Buffer* pPerObjCB = (ID3D11Buffer*)(renderParam.pPerObjCB->GetResource());
		g_pD3D11ImmediateContext->VSSetConstantBuffers(1, 1, &pPerObjCB);

		ID3D11Buffer* pShadowDepthCB = (ID3D11Buffer*)(m_pShadowDepthCB->GetResource());
		g_pD3D11ImmediateContext->VSSetConstantBuffers(2, 1, &pShadowDepthCB);
		
		ShaderParam* pSkyCubeTextureParam = m_pPixelShader->GetShaderParamPtrByName("ComVar_Texture_SkyCubeTexture");
		pSkyCubeTextureParam->SetTexture(m_pEnvReflectionTexture);

		ShaderParam* pSkyCubeTextureSamplerParam = m_pPixelShader->GetShaderParamPtrByName("ComVar_Texture_SkyCubeTextureSampler");
		pSkyCubeTextureSamplerParam->SetSampler(m_pEnvReflectionTextureSampler);

		ShaderParam* pDiffuseTextureParam = m_pPixelShader->GetShaderParamPtrByName("DiffuseTexture");
		pDiffuseTextureParam->SetTexture(renderParam.pDiffuseTexture);

		ShaderParam* pDiffuseTextureSamplerParam = m_pPixelShader->GetShaderParamPtrByName("DiffuseTextureSampler");
		pDiffuseTextureSamplerParam->SetSampler(renderParam.pDiffuseTextureSampler);

		ShaderParam* pNormalTextureParam = m_pPixelShader->GetShaderParamPtrByName("NormalTexture");
		pNormalTextureParam->SetTexture(renderParam.pNormalTexture);

		ShaderParam* pNormalTextureSamplerParam = m_pPixelShader->GetShaderParamPtrByName("NormalTextureSampler");
		pNormalTextureSamplerParam->SetSampler(renderParam.pNormalTextureSampler);

		ShaderParam* pShadowDepthTextureParam = m_pPixelShader->GetShaderParamPtrByName("ComVar_Texture_ShadowDepthTexture");
		pShadowDepthTextureParam->SetTexture(m_pShadowDepthTexture);

		ShaderParam* pShadowDepthTextureSamplerParam = m_pPixelShader->GetShaderParamPtrByName("ComVar_Texture_ShadowDepthTextureSampler");
		pShadowDepthTextureSamplerParam->SetSampler(m_pShadowDepthTextureSampler);

		m_pPixelShader->Apply();

		ID3D11Buffer* pLightingCB = (ID3D11Buffer*)(m_pLightingCB->GetResource());
		g_pD3D11ImmediateContext->PSSetConstantBuffers(0, 1, &pLightingCB);

		ID3D11Buffer* pShadowTestCB = (ID3D11Buffer*)(m_pShadowTestCB->GetResource());
		g_pD3D11ImmediateContext->PSSetConstantBuffers(1, 1, &pShadowTestCB);

		

		g_pD3D11ImmediateContext->DrawIndexed(indexNum, 0, 0);

		//Unbind ShadowDepthSRV
		ID3D11ShaderResourceView* nullRes[] = { nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr };
		g_pD3D11ImmediateContext->PSSetShaderResources(1, 8, nullRes);
	}
#pragma endregion MeshLightingRenderer

#pragma region MeshShadowRenderer
	class D3D11MeshShadowRenderer : public D3D11MeshRenderer
	{
	public:
		D3D11MeshShadowRenderer();
		D3D11MeshShadowRenderer(SceneView* pSceneView, IShader* pVS, IShader* pPS);
		virtual ~D3D11MeshShadowRenderer();

		virtual void					Render(const PrimitiveRenderParam& renderParam) override;

	private:
		IShader*						m_pVS;
		IShader*						m_pPS;
	};

	D3D11MeshShadowRenderer::D3D11MeshShadowRenderer():
		m_pVS(nullptr),
		m_pPS(nullptr)
	{}

	D3D11MeshShadowRenderer::D3D11MeshShadowRenderer(SceneView* pSceneView, IShader* pVS, IShader* pPS):
		D3D11MeshRenderer(pSceneView),
		m_pVS(pVS),
		m_pPS(pPS)
	{}

	D3D11MeshShadowRenderer::~D3D11MeshShadowRenderer()
	{}

	void D3D11MeshShadowRenderer::Render(const PrimitiveRenderParam& renderParam)
	{
		if (!g_pD3D11ImmediateContext)
		{
			return;
		}

		if (!renderParam.pVB || !renderParam.pIB)
		{
			return;
		}

		g_pD3D11ImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		ID3D11Buffer* pVB = (ID3D11Buffer*)(renderParam.pVB->GetResource());
		ID3D11Buffer* pIB = (ID3D11Buffer*)(renderParam.pIB->GetResource());
		
		uint32_t indexNum = renderParam.pIB->GetIndexNum();

		uint32_t stride = renderParam.pVB->GetStride();
		uint32_t offset = renderParam.pVB->GetOffset();
		g_pD3D11ImmediateContext->IASetVertexBuffers(0, 1, &pVB, &stride, &offset);
		g_pD3D11ImmediateContext->IASetIndexBuffer(pIB, DXGI_FORMAT_R32_UINT, 0);

		ShaderParam* pWorldToViewMatrix = m_pVS->GetShaderParamPtrByName("ComVar_Matrix_WorldToView");
		ShaderParam* pViewToClipMatrix = m_pVS->GetShaderParamPtrByName("ComVar_Matrix_ViewToClip");
		ShaderParam* pWorldEyePosition = m_pVS->GetShaderParamPtrByName("ComVar_Vector_WorldEyePosition");

		FPSCamera* pCamera = m_pSceneView->GetCamera();
		pWorldToViewMatrix->SetMatrix4x4(pCamera->GetViewMatrix());
		pViewToClipMatrix->SetMatrix4x4(pCamera->GetPerspProjectionMatrix());
		pWorldEyePosition->SetVector4(Vector4(pCamera->GetPosition(), 1.0f));
		m_pVS->Apply();

		ID3D11Buffer* pPerObjCB = (ID3D11Buffer*)(renderParam.pPerObjCB->GetResource());
		g_pD3D11ImmediateContext->VSSetConstantBuffers(1, 1, &pPerObjCB);

		m_pPS->Apply();

		g_pD3D11ImmediateContext->DrawIndexed(indexNum, 0, 0);
	}
#pragma endregion MeshShadowRenderer

	//===========================================================
	//    D3D11 Renderer
	//===========================================================

#pragma region PipelineRenderer
	class D3D11Renderer : public IRenderer
	{
	protected:
		struct GlobalConstantData
		{
			Matrix4x4	viewMatrix;
			Matrix4x4	projMatrix;
			Vector4		mainCameraWorldPos;
		};

		struct DirectionalLightData
		{
			Vector4		color;
			Vector3		direction;
			float		luminance;
		};

		struct ShadowDepthConstantData
		{
			Matrix4x4	viewMatrix;
			Matrix4x4	orthoMatrix;
		};

		struct ShadowTestConstantData
		{
			Vector4		param0;			//x: shadowDepthOffset, y: shadowDistance
		};

	public:
		D3D11Renderer();
		virtual ~D3D11Renderer();

		virtual bool				Init(const RendererInitDesc& desc) override;
		virtual void				Release() override;

		virtual void				Update(IScene* pScene) override;
		virtual void				Render(IScene* pScene) override;

		virtual bool				OnResize(uint32_t width, uint32_t height);

	private:
		bool						CreateInternalShaders();
		void						ReleaseInternalShaders();

		bool						CreateShadowResources(uint32_t width, uint32_t height);
		void						ReleaseShadowResources();

		//Shadow
		void						AddPrisToShadowView(IScene* pScene);
		void						ShadowDepthPass();

		void						ClearBackRenderTarget(float* clearColor);
		void						AddPrisAndLightsToSceneView(IScene* pScene);
		void						RenderPrimitives(IScene* pScene);

		void						RenderSky(IScene* pScene);

	private:
		IDXGISwapChain*				m_pSwapChain;

		ID3D11Texture2D*			m_pDepthStencilTexture;
		ID3D11RenderTargetView*		m_pRenderTargetView;
		ID3D11DepthStencilView*		m_pDepthStencilView;
		D3D11_VIEWPORT				m_ScreenViewport;

		SceneView*					m_pSceneView;

		IConstantBuffer*			m_pGlobalConstantBuffer;
		IConstantBuffer*			m_pLightingConstantBuffer;
		IConstantBuffer*			m_pShadowDepthConstantBuffer;
		IConstantBuffer*			m_pShadowTestConstantBuffer;

		//Shadow
		SceneView*					m_pShadowSceneView;

		D3D11_VIEWPORT				m_ShadowViewport;

		ITexture2D*					m_pShadowDepthTexture;
		ISamplerState*				m_pShadowDepthTextureSampler;

		IShader*					m_pStaticModelVertexShader;
		IShader*					m_pDirectionalLightingPixelShader;
		IShader*					m_pSkyVertexShader;
		IShader*					m_pSkyPixelShader;
		IShader*					m_pShadowDepthStaticModelVS;
		IShader*					m_pShadowDepthSkinModelVS;
		IShader*					m_pShadowDepthPS;

		ID3D11RasterizerState*		m_pSkyRasterizerState;
		ID3D11DepthStencilState*	m_pSkyDepthStencilState;
	};

	D3D11Renderer	g_D3D11Renderer;
	IRenderer*		g_pIRenderer = &g_D3D11Renderer;

	//------------------------------------------------------------------------
	//   Public Function
	//------------------------------------------------------------------------
	D3D11Renderer::D3D11Renderer() :
		m_pSwapChain(nullptr),
		m_pDepthStencilTexture(nullptr),
		m_pRenderTargetView(nullptr),
		m_pDepthStencilView(nullptr),
		m_ScreenViewport(),
		m_ShadowViewport(),
		m_pShadowSceneView(nullptr),
		m_pSceneView(nullptr),
		m_pGlobalConstantBuffer(nullptr),
		m_pLightingConstantBuffer(nullptr),
		m_pShadowDepthConstantBuffer(nullptr),
		m_pShadowTestConstantBuffer(nullptr),
		m_pShadowDepthTexture(nullptr),
		m_pShadowDepthTextureSampler(nullptr),
		m_pShadowDepthStaticModelVS(nullptr),
		m_pShadowDepthSkinModelVS(nullptr),
		m_pShadowDepthPS(nullptr),
		m_pSkyRasterizerState(nullptr),
		m_pSkyDepthStencilState(nullptr)
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
		HRESULT hr = D3D11CreateDevice(nullptr,
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
		swapchainDesc.BufferDesc.Width						= desc.backBufferWidth;
		swapchainDesc.BufferDesc.Height						= desc.backBufferHeight;
		swapchainDesc.BufferDesc.RefreshRate.Numerator		= 60;
		swapchainDesc.BufferDesc.RefreshRate.Denominator	= 1;
		swapchainDesc.BufferDesc.Format						= DXGI_FORMAT_R8G8B8A8_UNORM;
		swapchainDesc.SampleDesc.Count						= 1;
		swapchainDesc.SampleDesc.Quality					= 0;
		swapchainDesc.BufferUsage							= DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapchainDesc.BufferCount							= 1;
		swapchainDesc.OutputWindow							= desc.hWnd;
		swapchainDesc.Windowed								= true;
		swapchainDesc.SwapEffect							= DXGI_SWAP_EFFECT_DISCARD;
		swapchainDesc.Flags									= 0;

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

		m_pShadowSceneView = new SceneView();
		m_pSceneView = new SceneView(desc.pMainCamera);

		BufferDesc cbDesc = {};
		cbDesc.name = "ComVar_ConstantBuffer_Global";
		cbDesc.byteWidth = sizeof(GlobalConstantData);
		cbDesc.pInitData = nullptr;
		cbDesc.isDynamic = true;
		m_pGlobalConstantBuffer = (IConstantBuffer*)g_pIBufferManager->GetConstantBuffer(cbDesc);

		cbDesc.name = "MainLightingConstantBuffer";
		cbDesc.byteWidth = sizeof(DirectionalLightData);
		cbDesc.pInitData = nullptr;
		cbDesc.isDynamic = true;
		m_pLightingConstantBuffer = (IConstantBuffer*)g_pIBufferManager->GetConstantBuffer(cbDesc);

		CreateInternalShaders();

		int ShadowMapSize = g_ShadowMapRTSize;
		CreateShadowResources(ShadowMapSize, ShadowMapSize);

		D3D11_RASTERIZER_DESC skyRasterDesc = 
		{
			D3D11_FILL_SOLID,//D3D11_FILL_MODE FillMode;
			D3D11_CULL_NONE,//D3D11_CULL_MODE CullMode;
			false,//BOOL FrontCounterClockwise;
			0,//INT DepthBias;
			0.0,//FLOAT DepthBiasClamp;
			0.0,//FLOAT SlopeScaledDepthBias;
			true,//BOOL DepthClipEnable;
			false,//BOOL ScissorEnable;
			true,//BOOL MultisampleEnable;
			false//BOOL AntialiasedLineEnable;   
		};
		if (FAILED(g_pD3D11Device->CreateRasterizerState(&skyRasterDesc, &m_pSkyRasterizerState)))
		{
			MessageBox(nullptr, "Direct3D Create Sky Rasterizer State failed.", 0, 0);
			return false;
		}

		D3D11_DEPTH_STENCIL_DESC skyDepthDesc = {};
		skyDepthDesc.DepthEnable = true;
		skyDepthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		skyDepthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		skyDepthDesc.StencilEnable = false;
		if (FAILED(g_pD3D11Device->CreateDepthStencilState(&skyDepthDesc, &m_pSkyDepthStencilState)))
		{
			MessageBox(nullptr, "Direct3D Create Sky DepthStencil State failed.", 0, 0);
			return false;
		}

		return true;
	}

	void D3D11Renderer::Release()
	{
		if (m_pShadowSceneView)
		{
			delete m_pShadowSceneView;
			m_pShadowSceneView = nullptr;
		}

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

		ReleaseInternalShaders();

		ReleaseShadowResources();

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

		if (m_pSkyRasterizerState)
		{
			m_pSkyRasterizerState->Release();
			m_pSkyRasterizerState = nullptr;
		}

		if (m_pSkyDepthStencilState)
		{
			m_pSkyDepthStencilState->Release();
			m_pSkyDepthStencilState = nullptr;
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
		//输出详细的debug信息，如有需要去掉注释即可
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

	void D3D11Renderer::Update(IScene* pScene)
	{
		if (m_pSceneView->GetLightNum() > 0)
		{
			ILight* pMainLight = m_pSceneView->GetLight(0);
			DirectionalLightData dirLightData = {};
			dirLightData.direction = pMainLight->GetDirection();
			dirLightData.color = Vector4(pMainLight->GetColor(), 1.0f);
			dirLightData.luminance = pMainLight->GetLuminance();
			m_pLightingConstantBuffer->Update(&dirLightData, sizeof(dirLightData));

			if (pMainLight->GetType() == LIGHT_TYPE::DIRECTIONAL)
			{
				BoundingSphere sceneBoundingSphere = pScene->GetBoundingSphere();
				float lightFrustumSize = sceneBoundingSphere.radius;
				Vector3 dirLightPos = -(pMainLight->GetDirection() * lightFrustumSize);
				Matrix4x4 lightViewMatrix = GetLookAtMatrixLH(dirLightPos, Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f));
				Matrix4x4 lightOrthoMatrix = GetOrthographicMatrixLH(-lightFrustumSize, lightFrustumSize,
					-lightFrustumSize, lightFrustumSize,
					0.0f, 2.0f * lightFrustumSize);
				ShadowDepthConstantData shadowDepthData = {};
				shadowDepthData.viewMatrix = lightViewMatrix;
				shadowDepthData.orthoMatrix = lightOrthoMatrix;
				m_pShadowDepthConstantBuffer->Update(&shadowDepthData, sizeof(shadowDepthData));

				ShadowTestConstantData shadowTestData = {};
				shadowTestData.param0.x = g_ShadowDepthOffset;
				shadowTestData.param0.y = static_cast<float>(g_ShadowMapRTSize);
				m_pShadowTestConstantBuffer->Update(&shadowTestData, sizeof(shadowTestData));
			}
		}

		m_pShadowSceneView->ClearPrimitives();
		AddPrisToShadowView(pScene);

		m_pSceneView->ClearPrimitives();
		AddPrisAndLightsToSceneView(pScene);
	}


	void D3D11Renderer::Render(IScene* pScene)
	{
		ShadowDepthPass();

		float clearColor[4] = { 0.85f, 0.92f, 0.99f, 1.0f };
		ClearBackRenderTarget(clearColor);

		RenderPrimitives(pScene);
		
		RenderSky(pScene);
		
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

		float shadowMapSize = static_cast<float>(g_ShadowMapRTSize);
		m_ShadowViewport.TopLeftX = 0;
		m_ShadowViewport.TopLeftY = 0;
		m_ShadowViewport.Width = shadowMapSize;
		m_ShadowViewport.Height = shadowMapSize;
		m_ShadowViewport.MinDepth = 0;
		m_ShadowViewport.MaxDepth = 1;
		g_pD3D11ImmediateContext->RSSetViewports(1, &m_ShadowViewport);

		if (m_pSceneView && m_pSceneView->GetCamera())
		{
			m_pSceneView->GetCamera()->OnWindowResize(width, height);

		}

		return true;
	}

	//------------------------------------------------------------------------
	//   Private Function
	//------------------------------------------------------------------------
	bool D3D11Renderer::CreateInternalShaders()
	{
		ShaderCompileDesc vsDesc(g_StaticModelVertexShaderFilePath, nullptr, "Main", "vs_5_0", 0);
		m_pStaticModelVertexShader = g_pIShaderManager->GetStaticModelVertexShader(VERTEX_TYPE::STANDARD, vsDesc);
		ShaderCompileDesc psDesc(g_DirectionalLightingPixelShaderFilePath, nullptr, "Main", "ps_5_0", 0);
		m_pDirectionalLightingPixelShader = g_pIShaderManager->GetDirectionLightingPixelShader(psDesc);

		vsDesc = ShaderCompileDesc(g_SkyVertexShaderFilePath, nullptr, "Main", "vs_5_0", 0);
		m_pSkyVertexShader = g_pIShaderManager->GetStaticModelVertexShader(VERTEX_TYPE::STANDARD, vsDesc);
		psDesc = ShaderCompileDesc(g_SkyPixelShaderFilePath, nullptr, "Main", "ps_5_0", 0);
		m_pSkyPixelShader = g_pIShaderManager->GetSkyPixelShader(psDesc);

		vsDesc = ShaderCompileDesc("Shaders/ShadowDepthStaticModelVertexShader.hlsl", nullptr, "Main", "vs_5_0", 0);
		m_pShadowDepthStaticModelVS = g_pIShaderManager->GetStaticModelVertexShader(VERTEX_TYPE::STANDARD, vsDesc);
		vsDesc = ShaderCompileDesc("Shaders/ShadowDepthSkinModelVertexShader.hlsl", nullptr, "Main", "vs_5_0", 0);
		m_pShadowDepthSkinModelVS = g_pIShaderManager->GetVertexShader(VERTEX_TYPE::SKIN, vsDesc);
		psDesc = ShaderCompileDesc("Shaders/ShadowDepthPixelShader.hlsl", nullptr, "Main", "ps_5_0", 0);
		m_pShadowDepthPS = g_pIShaderManager->GetPixelShader(psDesc);

		return true;
	}

	void D3D11Renderer::ReleaseInternalShaders()
	{
		if (m_pStaticModelVertexShader)
		{
			m_pStaticModelVertexShader->Release();
			m_pStaticModelVertexShader = nullptr;
		}

		if (m_pDirectionalLightingPixelShader)
		{
			m_pDirectionalLightingPixelShader->Release();
			m_pDirectionalLightingPixelShader = nullptr;
		}

		if (m_pSkyVertexShader)
		{
			m_pSkyVertexShader->Release();
			m_pSkyVertexShader = nullptr;
		}

		if (m_pSkyPixelShader)
		{
			m_pSkyPixelShader->Release();
			m_pSkyPixelShader = nullptr;
		}

		if (m_pShadowDepthStaticModelVS)
		{
			m_pShadowDepthStaticModelVS->Release();
			m_pShadowDepthStaticModelVS = nullptr;
		}

		if (m_pShadowDepthSkinModelVS)
		{
			m_pShadowDepthSkinModelVS->Release();
			m_pShadowDepthSkinModelVS = nullptr;
		}

		if (m_pShadowDepthPS)
		{
			m_pShadowDepthPS->Release();
			m_pShadowDepthPS = nullptr;
		}
	}


	bool D3D11Renderer::CreateShadowResources(uint32_t width, uint32_t height)
	{
		BufferDesc cbDesc = {};
		cbDesc.name = "MainLightShadowDepthConstantBuffer";
		cbDesc.byteWidth = sizeof(ShadowDepthConstantData);
		cbDesc.pInitData = nullptr;
		cbDesc.isDynamic = true;
		m_pShadowDepthConstantBuffer = (IConstantBuffer*)g_pIBufferManager->GetConstantBuffer(cbDesc);

		cbDesc.name = "MainLightShadowTestConstantBuffer";
		cbDesc.byteWidth = sizeof(ShadowTestConstantData);
		cbDesc.pInitData = nullptr;
		cbDesc.isDynamic = false;
		m_pShadowTestConstantBuffer = (IConstantBuffer*)g_pIBufferManager->GetConstantBuffer(cbDesc);

		TextureDesc desc;
		desc.name = L"ShadowDepthTexture";
		desc.format = TEXTURE_FORMAT::R24G8_TYPELESS;
		desc.width = width;
		desc.height = height;
		desc.mipLevels = 1;
		desc.isDynamic = false;
		desc.isDepthTexture = true;

		m_pShadowDepthTexture = g_pITextureManager->GetTexture2D(desc);
		if (!m_pShadowDepthTexture)
		{
			return false;
		}

		SamplerDesc shadowMapSamplerDesc;
		shadowMapSamplerDesc.filterMode = SAMPLER_FILTER::COMPARISON_LINEAR;
		shadowMapSamplerDesc.addressMode = SAMPLER_ADDRESS::BORDER;
		shadowMapSamplerDesc.borderColor[0] = 1.0f;
		shadowMapSamplerDesc.borderColor[1] = 1.0f;
		shadowMapSamplerDesc.borderColor[2] = 1.0f;
		shadowMapSamplerDesc.borderColor[3] = 1.0f;
		m_pShadowDepthTextureSampler = g_pISamplerStateManager->CreateSamplerState(shadowMapSamplerDesc);
		if (!m_pShadowDepthTextureSampler)
		{
			return false;
		}

		return true;
	}

	void D3D11Renderer::ReleaseShadowResources()
	{
		if (m_pShadowDepthConstantBuffer)
		{
			m_pShadowDepthConstantBuffer->Release();
			m_pShadowDepthConstantBuffer = nullptr;
		}

		if (m_pShadowTestConstantBuffer)
		{
			m_pShadowTestConstantBuffer->Release();
			m_pShadowTestConstantBuffer = nullptr;
		}

		if (m_pShadowDepthTexture)
		{
			m_pShadowDepthTexture->Release();
			m_pShadowDepthTexture = nullptr;
		}

		if (m_pShadowDepthTextureSampler)
		{
			m_pShadowDepthTextureSampler->Release();
			m_pShadowDepthTextureSampler = nullptr;
		}
	}

	void D3D11Renderer::AddPrisToShadowView(IScene* pScene)
	{
		uint32_t priNum = pScene->GetPrimitivesNum();
		for (uint32_t i = 0; i < priNum; ++i)
		{
			IPrimitive* pPri = pScene->GetPrimitive(i);

			//FIXME!!! 这里后续应该添加视锥裁剪
			m_pShadowSceneView->AddPrimitive(pPri);
		}
	}

	void D3D11Renderer::ShadowDepthPass()
	{
		g_pD3D11ImmediateContext->RSSetViewports(1, &m_ShadowViewport);

		ID3D11DepthStencilView* pShadowDSV = (ID3D11DepthStencilView*)m_pShadowDepthTexture->GetDepthStencilView();
		g_pD3D11ImmediateContext->ClearDepthStencilView(pShadowDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
		g_pD3D11ImmediateContext->OMSetRenderTargets(0, nullptr, pShadowDSV);

		D3D11MeshShadowRenderer staticModelShadowRenderer(m_pSceneView, m_pShadowDepthStaticModelVS, m_pShadowDepthPS);
		D3D11MeshShadowRenderer skinModelShadowRenderer(m_pSceneView, m_pShadowDepthSkinModelVS, m_pShadowDepthPS);


		uint32_t opaquePriNum = m_pSceneView->GetOpaquePrisNum();
		for (uint32_t i = 0; i < opaquePriNum; ++i)
		{
			IPrimitive* pPri = m_pSceneView->GetOpaquePri(i);
			if (pPri->GetPriType() != PRIMITIVE_TYPE::SKIN_PRI)
			{
				pPri->Render(&staticModelShadowRenderer);
			}
			else
			{
				pPri->Render(&skinModelShadowRenderer);
			}
		}
	}

	void D3D11Renderer::ClearBackRenderTarget(float* clearColor)
	{
		g_pD3D11ImmediateContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

		g_pD3D11ImmediateContext->ClearRenderTargetView(m_pRenderTargetView, clearColor);
		g_pD3D11ImmediateContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	void D3D11Renderer::AddPrisAndLightsToSceneView(IScene* pScene)
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

	void D3D11Renderer::RenderPrimitives(IScene* pScene)
	{
		g_pD3D11ImmediateContext->RSSetViewports(1, &m_ScreenViewport);

		D3D11LineMeshRenderer lineRender(m_pSceneView);

		uint32_t simplePriNum = m_pSceneView->GetSimplePrisNum();
		for (uint32_t i = 0; i < simplePriNum; ++i)
		{
			IPrimitive* pPri = m_pSceneView->GetSimplePri(i);
			pPri->Render(&lineRender);
		}

		SkyBox* pSkyBox = pScene->GetSkyBox();
		ITexture2D* pEnvReflectionTexture = pSkyBox->GetCubeTexture();
		ISamplerState* pEnvReflectionTextureSampler = pSkyBox->GetCubeTextureSampler();

		MeshLightingGlobalData meshLightingData;
		meshLightingData.pSceneView = m_pSceneView;
		meshLightingData.pLightingCB = m_pLightingConstantBuffer;
		meshLightingData.pShadowDepthCB = m_pShadowDepthConstantBuffer;
		meshLightingData.pShadowTestCB = m_pShadowTestConstantBuffer;
		meshLightingData.pShadowDepthTexture = m_pShadowDepthTexture;
		meshLightingData.pShadowDepthTextureSampler = m_pShadowDepthTextureSampler;
		meshLightingData.pEnvReflectionTexture = pEnvReflectionTexture;
		meshLightingData.pEnvReflectionTextureSampler = pEnvReflectionTextureSampler;
		D3D11MeshLightingRenderer meshRender(meshLightingData);

		uint32_t opaquePriNum = m_pSceneView->GetOpaquePrisNum();
		for (uint32_t i = 0; i < opaquePriNum; ++i)
		{
			IPrimitive* pPri = m_pSceneView->GetOpaquePri(i);
			pPri->Render(&meshRender);
		}
	}

	void D3D11Renderer::RenderSky(IScene* pScene)
	{
		g_pD3D11ImmediateContext->RSSetState(m_pSkyRasterizerState);
		g_pD3D11ImmediateContext->OMSetDepthStencilState(m_pSkyDepthStencilState, 0);

		SkyBox* pSkyBox = pScene->GetSkyBox();

		IPrimitive* pPri = pSkyBox->GetPrimitive();

		D3D11SkyRenderer skyRender(m_pSceneView);
		if (pPri)
		{
			pPri->Render(&skyRender);
		}

		g_pD3D11ImmediateContext->RSSetState(nullptr);
		g_pD3D11ImmediateContext->OMSetDepthStencilState(nullptr, 0);
	}
#pragma endregion PipelineRenderer

}// namespace RenderDog